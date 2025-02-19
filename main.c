#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"
#include "inc/font.h"

// Definições do I2C para o display OLED
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define OLED_ADDR 0x3C

// Definições do joystick
#define JOYSTICK_X_PIN 26  // GPIO para eixo X
#define JOYSTICK_Y_PIN 27  // GPIO para eixo Y
#define JOYSTICK_PB 22     // GPIO para botão do joystick

// Definições do LED RGB (PWM)
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12

// Botão A
#define BUTTON_A 5

// Parâmetros do PWM
#define WRAP_PERIOD 4095 // PWM de 12 bits (0-4095)
#define PWM_DIV 1.0

// Estrutura do display
ssd1306_t ssd;
volatile bool leds_enabled = true;  // Estado dos LEDs
volatile bool border_style = false; // Estilo da borda do display
volatile bool led_green_state = false; // Estado do LED verde

// Debounce - Tempo mínimo entre pressionamentos (200ms)
#define DEBOUNCE_TIME 200
uint32_t last_press_time_pb = 0;
uint32_t last_press_time_a = 0;

// Configuração do PWM para LEDs
void pwm_setup(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice, PWM_DIV);
    pwm_set_wrap(slice, WRAP_PERIOD);
    pwm_set_gpio_level(gpio, 0);
    pwm_set_enabled(slice, true);
}

// Configuração do ADC
void adc_setup() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

// Configuração do display OLED
void oled_setup() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, OLED_ADDR, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

// Implementação do debounce para botões
bool button_debounce(uint32_t *last_press_time) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - *last_press_time > DEBOUNCE_TIME) {
        *last_press_time = current_time;
        return true;
    }
    return false;
}

// Interrupção para o botão do joystick (alterna LED verde e altera borda)
void joystick_button_callback(uint gpio, uint32_t events) {
    if (button_debounce(&last_press_time_pb)) {
        led_green_state = !led_green_state;
        gpio_put(LED_GREEN, led_green_state);
        border_style = !border_style;
        printf("Botão do joystick pressionado! LED Verde: %d, Estilo da Borda: %d\n", led_green_state, border_style);
    }
}

// Interrupção para o botão A (ativa/desativa os LEDs)
void button_a_callback(uint gpio, uint32_t events) {
    if (button_debounce(&last_press_time_a)) {
        leds_enabled = !leds_enabled;
        printf("Botão A pressionado! LEDs: %s\n", leds_enabled ? "Ligados" : "Desligados");
    }
}

// Leitura do joystick e atualização do PWM dos LEDs RGB
void update_leds() {
    adc_select_input(0);
    uint16_t adc_x = adc_read();
    adc_select_input(1);
    uint16_t adc_y = adc_read();

    // Ajusta a intensidade dos LEDs baseando-se no deslocamento do centro (2048)
    int16_t deslocamento_x = adc_x - 2048;
    int16_t deslocamento_y = adc_y - 2048;

    uint16_t pwm_red = 0;
    uint16_t pwm_blue = 0;

    if (leds_enabled) {
        // LEDs só acendem se o joystick sair do centro
        pwm_red = abs(deslocamento_x) > 100 ? abs(deslocamento_x) : 0;  // LED Vermelho (X)
        pwm_blue = abs(deslocamento_y) > 100 ? abs(deslocamento_y) : 0; // LED Azul (Y)
    }

    pwm_set_gpio_level(LED_RED, pwm_red);
    pwm_set_gpio_level(LED_BLUE, pwm_blue);

    // Imprime valores do joystick
    printf("Joystick X: %d, Joystick Y: %d\n", adc_x, adc_y);
}

// Atualização do display OLED com o quadrado móvel
void update_display() {
    adc_select_input(0);
    uint16_t adc_x = adc_read();
    adc_select_input(1);
    uint16_t adc_y = adc_read();

    // Corrigindo a inversão dos eixos do joystick no display
    uint8_t pos_x = (adc_y * 120) / 4095; // Agora o eixo Y controla a vertical corretamente
    uint8_t pos_y = 56 - ((adc_x * 56) / 4095);  // Agora o eixo X controla a horizontal corretamente

    // Atualiza o display
    ssd1306_fill(&ssd, false);
    if (border_style) {
        ssd1306_rect(&ssd, 0, 0, 127, 63, true, false); // Estilo 1
    } else {
        ssd1306_rect(&ssd, 2, 2, 124, 60, true, false); // Estilo 2
    }
    ssd1306_rect(&ssd, pos_x, pos_y, 8, 8, true, true); // Quadrado móvel corrigido
    ssd1306_send_data(&ssd);
}

int main() {
    stdio_init_all();

    // Configuração de hardware
    adc_setup();
    oled_setup();
    pwm_setup(LED_RED);
    pwm_setup(LED_BLUE);
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    // Configuração de botões e interrupções
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);
    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &joystick_button_callback);

    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_a_callback);

    while (true) {
        update_leds();
        update_display();
        sleep_ms(50);
    }

    return 0;
}
