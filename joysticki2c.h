#include "pico/stdlib.h"
#include "hardware/adc.h"

#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27

void init_joystick();
void ler_joystick(uint16_t *x, uint16_t *y);