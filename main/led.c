#include <stdint.h>

#include <driver/ledc.h>
#include <esp_err.h>

#include "led.h"

#define GPIO_RED 25
#define GPIO_GREEN 26
#define GPIO_BLUE 27

#define MODE LEDC_LOW_SPEED_MODE
#define TIMER LEDC_TIMER_0
#define FREQ_HZ 1000
#define RES LEDC_TIMER_16_BIT

#define CH_R LEDC_CHANNEL_0
#define CH_G LEDC_CHANNEL_1
#define CH_B LEDC_CHANNEL_2

void init_led(void) {
    ledc_timer_config_t t = {.speed_mode = MODE,
                             .timer_num = TIMER,
                             .duty_resolution = RES,
                             .freq_hz = FREQ_HZ,
                             .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&t));

    ledc_channel_config_t c = {.speed_mode = MODE,
                               .intr_type = LEDC_INTR_DISABLE,
                               .timer_sel = TIMER,
                               .duty = 0,
                               .hpoint = 0};

    c.channel = CH_R;
    c.gpio_num = GPIO_RED;
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    c.channel = CH_G;
    c.gpio_num = GPIO_GREEN;
    ESP_ERROR_CHECK(ledc_channel_config(&c));
    c.channel = CH_B;
    c.gpio_num = GPIO_BLUE;
    ESP_ERROR_CHECK(ledc_channel_config(&c));

    update_led(0, 0, 0);
}

void update_led(uint16_t r, uint16_t g, uint16_t b) {
    ESP_ERROR_CHECK(ledc_set_duty(MODE, CH_R, r));
    ESP_ERROR_CHECK(ledc_update_duty(MODE, CH_R));

    ESP_ERROR_CHECK(ledc_set_duty(MODE, CH_G, g));
    ESP_ERROR_CHECK(ledc_update_duty(MODE, CH_G));

    ESP_ERROR_CHECK(ledc_set_duty(MODE, CH_B, b));
    ESP_ERROR_CHECK(ledc_update_duty(MODE, CH_B));
}