idf_component_register(
    SRCS
        "main.c"
        "httpd.c"
        "led.c"
        "server.c"
        "wifi.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        esp_driver_ledc
        esp_http_server
        esp_wifi
        nvs_flash

    EMBED_TXTFILES
        "index.html"
)
