#pragma once

#include <stdint.h>

struct LEDState {
    uint16_t r, g, b;
};

void led_init(void);

void led_update_r(uint16_t r);
void led_update_g(uint16_t g);
void led_update_b(uint16_t b);
void led_update_rgb(const struct LEDState* state);

void led_commit(void);
struct LEDState led_state(void);