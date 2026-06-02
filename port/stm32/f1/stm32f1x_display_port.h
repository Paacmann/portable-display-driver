#ifndef STM32F1X_DISPLAY_PORT_H
#define STM32F1X_DISPLAY_PORT_H

#include "/home/petar/Desktop/portable-display-driver/driver/display.h"

typedef struct
{
    void *hi2c;     /* I2C_HandleTypeDef* */
} stm32f1x_display_context_t;

enum display_status i2c_write(
    void *context,
    uint8_t addr,
    uint8_t *data,
    uint16_t size
);

enum display_status i2c_write_cmd(
    void *context,
    uint8_t addr,
    uint8_t cmd
);

uint32_t get_tick_ms(void *context);

void delay_ms(
    void *context,
    uint32_t delay
);



#endif
