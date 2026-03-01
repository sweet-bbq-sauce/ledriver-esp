#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <lwip/sockets.h>

#include "led.h"

static const char* TAG = "LED server";

#define BUFFER_SIZE 64

typedef int socket_t;
#define INVALID_SOCKET ((socket_t) - 1)

#define MAGIC_VALUE 0x4C454452
#define PROTOCOL_VERSION 0x00
enum Action { NONE = 0x00, PING = 0x01, UPDATE = 0x02, POWER = 0x03, STATUS = 0x04 };

struct RootHeader {
    uint32_t magic;
    uint8_t version;
    uint8_t action;
    uint16_t flags;
};

struct ColorState {
    uint16_t r_ne, g_ne, b_ne;
};

void led_server_task(void* arg) {
    (void)arg;

    struct sockaddr_storage listen_address;
    bzero(&listen_address, sizeof(listen_address));

    struct sockaddr_in* in = (struct sockaddr_in*)&listen_address;
    in->sin_family = AF_INET;
    in->sin_addr.s_addr = 0;
    in->sin_port = htons(6666);

    const socket_t listen_fd = socket(listen_address.ss_family, SOCK_DGRAM, IPPROTO_UDP);
    if (listen_fd == INVALID_SOCKET) {
        ESP_LOGE(TAG, "can't create listen socket: %s", strerror(errno));
        vTaskDelete(NULL);
        return;
    }

    if (bind(listen_fd, (const struct sockaddr*)&listen_address, sizeof(listen_address)) != 0) {
        close(listen_fd);
        ESP_LOGE(TAG, "can't bind listen socket: %s", strerror(errno));
        vTaskDelete(NULL);
        return;
    }

    uint8_t buffer[BUFFER_SIZE];

#define ERROR_THRESHOLD 5
    int error_count = 0;

    struct ColorState color_state;
    bzero(&color_state, sizeof(color_state));
    uint8_t power = false;

    for (;;) {
        struct sockaddr_storage client_address;
        socklen_t client_address_length = sizeof(client_address);

        const ssize_t n = recvfrom(listen_fd, buffer, sizeof(buffer), 0,
                                   (struct sockaddr*)&client_address, &client_address_length);
        if (n < 0) {
            error_count++;
            ESP_LOGE(TAG, "recvfrom() error: %s", strerror(errno));
            if (error_count >= ERROR_THRESHOLD) {
                close(listen_fd);
                vTaskDelete(NULL);
            }
            continue;
        }
        error_count = 0;

        if (n < sizeof(struct RootHeader)) {
            ESP_LOGI(TAG, "Too small packet");
            continue;
        }

        const struct RootHeader* header = (const struct RootHeader*)buffer;
        if (ntohl(header->magic) != MAGIC_VALUE) {
            ESP_LOGI(TAG, "Invalid magic value");
            continue;
        }
        if (header->version != PROTOCOL_VERSION) {
            ESP_LOGI(TAG, "Invalid protocol version");
            continue;
        }

        const uint8_t* payload_ptr = buffer + sizeof(*header);
        const size_t payload_size = n - sizeof(*header);

        switch (header->action) {
        case NONE: {
            // Do nothing.
        } break;

        case PING: {
            // Resend header.
            sendto(listen_fd, header, sizeof(*header), 0, (const struct sockaddr*)&client_address,
                   client_address_length);
        } break;

        case UPDATE: {
            if (payload_size < 6) {
                ESP_LOGI(TAG, "UPDATE: Too small payload");
                break;
            }

            memcpy(&color_state, payload_ptr, sizeof(color_state));

            if (power)
                update_led(ntohs(color_state.r_ne), ntohs(color_state.g_ne),
                           ntohs(color_state.b_ne));
        } break;

        case POWER: {
            if (payload_size < 1) {
                ESP_LOGI(TAG, "POWER: Too small payload");
                break;
            }

            power = payload_ptr[0];
            if (power)
                update_led(ntohs(color_state.r_ne), ntohs(color_state.g_ne),
                           ntohs(color_state.b_ne));
            else
                update_led(0, 0, 0);
        } break;

        case STATUS: {
            uint8_t response[15]; // header (8) + color state (6) + power (1)
            memcpy(response, header, 8);
            memcpy(response + 8, &color_state, 6);
            memcpy(response + 14, &power, 1);

            sendto(listen_fd, response, sizeof(response), 0,
                   (const struct sockaddr*)&client_address, client_address_length);
        }
        }
    }

    vTaskDelete(NULL);
    return;
}