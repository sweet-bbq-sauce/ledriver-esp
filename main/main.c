#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <strings.h>

#include <esp_log.h>
#include <lwip/sockets.h>
#include <nvs_flash.h>

#include "led.h"
#include "wifi.h"

extern void led_server_task(void* arg);
extern void start_web_panel(void);

void app_main(void) {
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);
    led_init();

    connect_to_wifi("Mayorka", "Symcia160");

    xTaskCreate(led_server_task, "LED controller", 4096, NULL, 5, NULL);
    start_web_panel();
}