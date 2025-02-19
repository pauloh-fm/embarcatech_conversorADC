#include "leds.h"
#include "hardware/pwm.h"

static bool estado_led_verde = false;

void init_leds() {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
}

void ajustar_brilho_led(int eixoX, int eixoY) {
    pwm_set_gpio_level(LED_RED_PIN, eixoX);
    pwm_set_gpio_level(LED_BLUE_PIN, eixoY);
}

void alternar_led_verde() {
    estado_led_verde = !estado_led_verde;
    gpio_put(LED_GREEN_PIN, estado_led_verde);
}
