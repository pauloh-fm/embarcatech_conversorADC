#include "pico/stdlib.h"

#define JOYSTICK_BUTTON 22
#define BOTAO_A 5

void init_botoes();
void gpio_callback(uint gpio, uint32_t events);
bool button_debounce(uint botao_pin);