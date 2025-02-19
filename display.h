#include "inc/ssd1306.h"
#include "inc/font.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

void init_display();
void atualizar_display(uint16_t x, uint16_t y);
