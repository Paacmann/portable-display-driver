#include "stm32f1x_display_port.h"



#include "stm32f1xx_hal.h"

#define I2C_TIMEOUT_MS 100u

enum display_status i2c_write(void *context,
                              uint8_t addr,
                              uint8_t *data,
                              uint16_t size)
{
    if (!context || !data || !size) {
        
        return DISPLAY_EINVAL;
    }

    stm32f1x_display_context_t *port_context =
        (stm32f1x_display_context_t *)context;

    HAL_StatusTypeDef h_status =
        HAL_I2C_Master_Transmit(
            (I2C_HandleTypeDef *)port_context->hi2c,
            (uint16_t)(addr << 1),
            data,
            size,
            I2C_TIMEOUT_MS
        );

    return (h_status == HAL_OK) ? DISPLAY_SUCCESS : DISPLAY_ERROR;
}

enum display_status i2c_write_cmd(void *context, uint8_t addr, uint8_t cmd) {

    if (context == NULL) {
        
        return DISPLAY_EINVAL;
    }

    stm32f1x_display_context_t *port_context =
        (stm32f1x_display_context_t *)context;

    uint8_t tx_buf[2];

    /*
     * SSD1306:
     * 0x00 = command stream
     * tx_buf[1] = actual command
     */

    tx_buf[0] = 0x00u;
    tx_buf[1] = cmd;

    HAL_StatusTypeDef h_status =
        HAL_I2C_Master_Transmit(
            (I2C_HandleTypeDef *)port_context->hi2c ,
            (uint16_t)(addr << 1) ,
            tx_buf ,
            sizeof(tx_buf) ,
            I2C_TIMEOUT_MS
        );

    if (h_status != HAL_OK) {
        
        return DISPLAY_ERROR;
    }

    return DISPLAY_SUCCESS;
}



uint32_t get_tick_ms(void *context)
{
    (void)context;

    return HAL_GetTick();
}

void delay_ms(void *context, uint32_t delay)
{
    (void)context;

    HAL_Delay(delay);
}

