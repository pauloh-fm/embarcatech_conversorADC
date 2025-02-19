#include "joysticki2c.h"

void init_joystick() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

void ler_joystick(uint16_t *x, uint16_t *y) {
    adc_select_input(0);
    *x = adc_read();

    adc_select_input(1);
    *y = adc_read();
}
