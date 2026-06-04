/*
 *  Cross-platform display driver - implementation 
 *
 * */

#include "display.h"
#include "fonts.h"

#include <stdint.h>
#include <stddef.h>



/* Forward declaration */

static enum display_status validate_device(display_t *disp);
static enum display_status send_init_seq(display_t *disp);

/* Public API */

enum display_status display_send_command(display_t *disp, uint8_t cmd) {

    enum display_status status;
    status = validate_device(disp);

    if (status != DISPLAY_SUCCESS) {
        return status;
    }
    
    if (disp->dev.i2c_write_cmd == NULL) {
        return DISPLAY_EINVAL;
    }

    status = disp->dev.i2c_write_cmd(disp->dev.context,disp->address,cmd);

    return status;

}


enum display_status display_send_data(display_t *disp, uint8_t *data, size_t size) {

    enum display_status status;
    status = validate_device(disp);

    if (status != DISPLAY_SUCCESS) {
        return status;
    }

    if (data == NULL || size == 0u) {
        return DISPLAY_EINVAL;
    }

    if (disp->dev.i2c_write == NULL) {
        return DISPLAY_EINVAL;
    }
    
    status = disp->dev.i2c_write(disp->dev.context, disp->address, data, (uint16_t)size);

    return status;
}


enum display_status display_on(display_t *disp) {

    return display_send_command(disp, DISPLAY_CMD_DISPLAY_ON);
}

enum display_status display_off(display_t *disp) {

    return display_send_command(disp , DISPLAY_CMD_DISPLAY_OFF); 
}

enum display_status display_init(display_t *disp) {
    
    enum display_status status;
    status = validate_device(disp);

    if (status != DISPLAY_SUCCESS) { 
        return status;
    }

    if (disp->height == 0) {
        disp->height = DISPLAY_HEIGHT;
    }

    if (disp->width == 0) {
        disp->width = DISPLAY_WIDTH;
    }

    status = send_init_seq(disp);
    if (status != DISPLAY_SUCCESS) { 
        return status;
    }
    

    return DISPLAY_SUCCESS;
}


/* Helper function ("private function") */

static enum display_status validate_device(display_t *disp) {
    
    if (disp == NULL) {
        return DISPLAY_ENODEV;
    }
    
    if (disp->dev.i2c_write == NULL || disp->dev.i2c_write_cmd == NULL) {
        return DISPLAY_EINVAL;
    }

    return DISPLAY_SUCCESS;
}

static enum display_status send_init_seq(display_t *disp) {

    enum display_status status;

    const uint8_t init_seq[] = {
        0xAE,              // display off

    	0x20, 0x00,        // horizontal addressing

    	0xA4,              // display follow RAM (FIX CRNE/STATIC TAČKE)

    	0xC8,              // COM scan dec
    	0xA1,              // segment remap

    	0x81, 0x7F,        // contrast

    	0xA6,              // normal display

    	0xA8, 0x3F,        // multiplex
    	0xD3, 0x00,        // offset
    	0xD5, 0x80,        // clock
    	0xD9, 0xF1,        // precharge
    	0xDA, 0x12,        // COM pins
    	0xDB, 0x40,        // VCOM

    	0x8D, 0x14,        // charge pump ON

    	0xAF               // display ON
    };

    for (size_t i = 0u; i < sizeof(init_seq) / sizeof(init_seq[0]); i++) {
        
        status = display_send_command(disp , init_seq[i]);

        if (status != DISPLAY_SUCCESS) {
            return status;
        }
    }

    if (disp->dev.delay_ms != NULL) {

        disp->dev.delay_ms(disp->dev.context, 100u);
    }

    return DISPLAY_SUCCESS;

}


enum display_status display_clear(display_t *disp) {
     
    if (!disp) {
        
         return DISPLAY_ENODEV;
    }


    for (uint16_t i = 0; i < sizeof(disp->buffer); i++) {
        
        disp->buffer[i] = 0x00;
    }

    return DISPLAY_SUCCESS;

}


enum display_status display_update(display_t *disp)
{
    if (!disp) {
        
        return DISPLAY_ENODEV;
    }

    for (uint8_t page = 0; page < 8; page++)
    {
        display_send_command(disp, (uint8_t)(0xB0 + page)); // set page
        display_send_command(disp, 0x00);        // low col
        display_send_command(disp, 0x10);        // high col

        for (uint8_t col = 0; col < 128; col++)
        {
            uint8_t tx[2u];
            tx[0] = 0x40; // DATA mode
            tx[1] = disp->buffer[page * 128 + col];

            disp->dev.i2c_write(
                disp->dev.context,
                disp->address,
                tx,
                2
            );
        }
    }

    return DISPLAY_SUCCESS;
}

void display_draw_pixel(display_t *disp, uint16_t x, uint16_t y)
{
    if (!disp) { 
        
        return ;
    }

    if (x >= disp->width || y >= disp->height) {
        
        return ;
    }

    uint16_t index = (uint16_t)(x + (y / 8u) * disp->width);
    disp->buffer[index] |= (uint8_t)(1u << (y % 8u));
}



void display_putc(display_t *disp, char c, uint16_t x, uint16_t y)
{
    if (!disp) { 
        
        return;
    }
    
    uint8_t ch = (uint8_t)c;

    if (ch < 32u || ch > 127u) {
        
        return;
    }

    uint16_t index = (uint16_t)((ch - 32u) * Font_7x10.FontHeight);

    for (uint8_t row = 0; row < Font_7x10.FontHeight; row++)
    {
        uint16_t bits = Font_7x10.data[index + row];

        for (uint8_t col = 0; col < Font_7x10.FontWidth; col++)
        {
            if (bits & (1u << (15u - col)))
            {
                display_draw_pixel(disp, (uint16_t)(x + col), (uint16_t)(y + row));
            }
        }
    }
}

void display_puts(display_t *disp,
                  const char *str,
                  uint16_t x,
                  uint16_t y)
{
    if (!disp || !str) {

        return;
    }

    while (*str)
    {
        display_putc(disp, *str++, x, y);
        x = (uint16_t)(x + (uint16_t)(Font_7x10.FontWidth + 1u));
    }
}

