#include "botao.h"
#include <stdio.h>

#define DEBOUNCE_TIME 50  // Debounce de 50ms

volatile bool estado_leds = true;  // Controla se os LEDs PWM estão ligados

// Implementação do debouncing por software
bool button_debounce(uint botao_pin) {
    static uint32_t last_press_time[30] = {0};  // Armazena o tempo da última ativação para múltiplos botões
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio_get(botao_pin) == 0) { // Botão pressionado (nível baixo)
        if (current_time - last_press_time[botao_pin] > DEBOUNCE_TIME) {
            last_press_time[botao_pin] = current_time;
            return true;
        }
    }
    return false;
}

// Função de interrupção para os botões
void gpio_callback(uint gpio, uint32_t events) {
    if (button_debounce(gpio)) {
        if (gpio == JOYSTICK_BUTTON) {
            printf("Botão do joystick pressionado\n");
        } 
        else if (gpio == BOTAO_A) {
            estado_leds = !estado_leds;  // Alterna o estado dos LEDs PWM
            printf("Botão A pressionado, LEDs %s\n", estado_leds ? "LIGADOS" : "DESLIGADOS");
        }
    }
}

// Inicializa os botões com interrupção
void init_botoes() {
    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    // Configura as interrupções
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, gpio_callback);
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, gpio_callback);
}
