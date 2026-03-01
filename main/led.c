#include <stdint.h>
#include <strings.h>

#include <freertos/portmacro.h>

#include <driver/ledc.h>
#include <esp_err.h>
#include <esp_log.h>

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

static struct LEDState current_state;
static portMUX_TYPE cs_mux = portMUX_INITIALIZER_UNLOCKED;

void led_init(void) {
    const ledc_timer_config_t t = {.speed_mode = MODE,
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

    portENTER_CRITICAL(&cs_mux);
    bzero(&current_state, sizeof(current_state));
    portEXIT_CRITICAL(&cs_mux);
    led_commit();
}

void led_update_r(uint16_t r) {
    portENTER_CRITICAL(&cs_mux);
    current_state.r = r;
    portEXIT_CRITICAL(&cs_mux);
}

void led_update_g(uint16_t g) {
    portENTER_CRITICAL(&cs_mux);
    current_state.g = g;
    portEXIT_CRITICAL(&cs_mux);
}

void led_update_b(uint16_t b) {
    portENTER_CRITICAL(&cs_mux);
    current_state.b = b;
    portEXIT_CRITICAL(&cs_mux);
}

void led_update_rgb(const struct LEDState* state) {
    portENTER_CRITICAL(&cs_mux);
    current_state = *state;
    portEXIT_CRITICAL(&cs_mux);
}

void led_commit(void) {
    portENTER_CRITICAL(&cs_mux);
    ESP_ERROR_CHECK(ledc_set_duty(MODE, CH_R, current_state.r));
    ESP_ERROR_CHECK(ledc_update_duty(MODE, CH_R));

    ESP_ERROR_CHECK(ledc_set_duty(MODE, CH_G, current_state.g));
    ESP_ERROR_CHECK(ledc_update_duty(MODE, CH_G));

    ESP_ERROR_CHECK(ledc_set_duty(MODE, CH_B, current_state.b));
    ESP_ERROR_CHECK(ledc_update_duty(MODE, CH_B));
    portEXIT_CRITICAL(&cs_mux);
}

struct LEDState led_state(void) {
    portENTER_CRITICAL(&cs_mux);
    const struct LEDState cs_copy = current_state;
    portEXIT_CRITICAL(&cs_mux);
    return cs_copy;
}