
#include <stdint.h>

#include <esp_http_server.h>
#include <esp_log.h>
#include <lwip/sockets.h>

#include "led.h"

// static const char* TAG = "httpd";
static httpd_handle_t s_server = NULL;

static esp_err_t index_get_handler(httpd_req_t* req) {
    extern const uint8_t index_html_start[] asm("_binary_index_html_start");
    extern const uint8_t index_html_end[] asm("_binary_index_html_end");

    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char*)index_html_start, index_html_end - index_html_start);
}

static esp_err_t index_ws_handler(httpd_req_t* req) {
    if (req->method == HTTP_GET)
        return ESP_OK;

    httpd_ws_frame_t ws_pkt = {0};
    ws_pkt.type = HTTPD_WS_TYPE_BINARY;
    esp_err_t result = httpd_ws_recv_frame(req, &ws_pkt, 0);

    uint16_t values[3];
    ws_pkt.payload = (uint8_t*)&values;

    result = httpd_ws_recv_frame(req, &ws_pkt, 6);
    if (result == ESP_OK) {
        const struct LEDState ls = {
            .r = ntohs(values[0]), .g = ntohs(values[1]), .b = ntohs(values[2])};

        led_update_rgb(&ls);
        led_commit();
    }

    return result;
}

void start_web_panel(void) {
    const httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    const httpd_uri_t index_uri = {
        .uri = "/", .method = HTTP_GET, .handler = index_get_handler, .user_ctx = NULL};

    const httpd_uri_t ws_uri = {.uri = "/ws",
                                .method = HTTP_GET,
                                .handler = index_ws_handler,
                                .user_ctx = NULL,
                                .is_websocket = true};

    if (httpd_start(&s_server, &config) == ESP_OK) {
        httpd_register_uri_handler(s_server, &index_uri);
        httpd_register_uri_handler(s_server, &ws_uri);
    }
}