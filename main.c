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

// Definições do joystick e botões
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define JOYSTICK_PB 22  // Botão do joystick
#define BUTTON_A 5       // Botão A

// Definições do LED RGB (PWM)
#define LED_RED 13
#define LED_GREEN 11
#define LED_BLUE 12

// Parâmetros do PWM
#define WRAP_PERIOD 4095
#define PWM_DIV 1.0

// Estrutura do display
ssd1306_t ssd;
volatile bool leds_enabled = true;
volatile bool border_style = false;
volatile bool led_green_state = false;

// ZONA MORTA para evitar pequenas variações no centro do joystick
#define DEAD_ZONE 150

// Debounce - Tempo mínimo entre pressionamentos (200ms)
#define DEBOUNCE_TIME 200
uint32_t last_press_time = 0; // Agora há apenas um controle de tempo para os dois botões

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
bool button_debounce() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_press_time > DEBOUNCE_TIME) {
        last_press_time = current_time;
        return true;
    }
    return false;
}

// Função única de interrupção para ambos os botões
void button_callback(uint gpio, uint32_t events) {
    if (!button_debounce()) return;

    if (gpio == JOYSTICK_PB) {
        // Alterna estado do LED verde
        led_green_state = !led_green_state;
        gpio_put(LED_GREEN, led_green_state);

        // Desabilita apenas o LED vermelho (não afeta o azul)
        pwm_set_gpio_level(LED_RED, led_green_state ? 0 : WRAP_PERIOD / 2);

        // Altera estilo da borda
        border_style = !border_style;

        printf("Botão do joystick pressionado! LED Verde: %s, LED Vermelho: %s, Estilo da Borda: %s\n",
               led_green_state ? "Ligado" : "Desligado",
               led_green_state ? "Desligado" : "Ligado",
               border_style ? "Grossa" : "Normal");
    } 
    else if (gpio == BUTTON_A) {
        // Alterna estado geral dos LEDs
        leds_enabled = !leds_enabled;
        printf("Botão A pressionado! LEDs: %s\n", leds_enabled ? "Ligados" : "Desligados");
    }
}

// Leitura do joystick e atualização do PWM dos LEDs RGB
void update_leds() {
    adc_select_input(0);
    uint16_t adc_y = adc_read();
    adc_select_input(1);
    uint16_t adc_x = adc_read();

    int16_t deslocamento_y = adc_y - 2048;
    int16_t deslocamento_x = adc_x - 2048;

    uint16_t pwm_red = 0;
    uint16_t pwm_blue = 0;

    if (leds_enabled) {
        pwm_red = (abs(deslocamento_x) > DEAD_ZONE) ? abs(deslocamento_x) : 0;
        pwm_blue = (abs(deslocamento_y) > DEAD_ZONE) ? abs(deslocamento_y) : 0;
    }

    pwm_set_gpio_level(LED_RED, pwm_red);
    pwm_set_gpio_level(LED_BLUE, pwm_blue);
}

// Atualização do display OLED com o quadrado móvel
void update_display() {
    adc_select_input(0);
    uint16_t adc_x = adc_read();
    adc_select_input(1);
    uint16_t adc_y = adc_read();

    uint8_t pos_y = (adc_y * 120) / 4095; 
    uint8_t pos_x = (adc_x * 56) / 4095;

    if (abs(adc_y - 2048) < DEAD_ZONE) pos_y = 60;
    if (abs(adc_x - 2048) < DEAD_ZONE) pos_x = 28;

    printf("ADC X: %d, ADC Y: %d -> Pos X: %d, Pos Y: %d\n", adc_x, adc_y, pos_x, pos_y);

    ssd1306_fill(&ssd, false);

    if (border_style) {
        // **Borda pontilhada**
        for (int i = 0; i < 128; i += 4) { // Pontos na borda superior e inferior
            ssd1306_pixel(&ssd, i, 0, true);
            ssd1306_pixel(&ssd, i, 63, true);
        }
        for (int i = 0; i < 64; i += 4) { // Pontos nas laterais esquerda e direita
            ssd1306_pixel(&ssd, 0, i, true);
            ssd1306_pixel(&ssd, 127, i, true);
        }
    } else {
        // **Borda de linha contínua**
        ssd1306_rect(&ssd, 0, 0, 127, 63, true, false);
    }

    ssd1306_rect(&ssd, pos_x, pos_y, 8, 8, true, true);
    ssd1306_send_data(&ssd);
}


int main() {
    stdio_init_all();

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
    
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    // Registra interrupção para ambos os botões
    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &button_callback);
    gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_FALL, true);

    while (true) {
        update_leds();
        update_display();
        sleep_ms(50);
    }

    return 0;
}
