/**
 * 
 * Cross-platform embedded display driver — public API.
 *
 * Provides a hardware-agnostic interface for I2C-based display controllers
 * (e.g. SSD1306, SH1106).
 *
 * All platform I/O is supplied via function pointers in display_device,
 * making the driver portable across any MCU or OS.
 *
 * v1 supports I2C-only displays. SPI support is planned for a future revision.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stddef.h>

/** display_status Driver return codes
 *
 */
enum display_status {
    DISPLAY_SUCCESS = 0,   // Operation completed successfully.
    DISPLAY_ERROR   = 1,   // Generic I/O error
    DISPLAY_BUSY    = 2,   // Device is busy
    DISPLAY_TIMEOUT = 3,   // Operation timed out 
    DISPLAY_ENODEV  = 4,   // Device handle is NULL
    DISPLAY_EINVAL  = 5,   // Invalid argument
};

/* Display geometry constants
 *
 * Default geometry for SSD1306 128x64 display.
 * 
 */
#define DISPLAY_WIDTH   128u
#define DISPLAY_HEIGHT  64u
#define DISPLAY_PAGES   8u   // 64px / 8 = 8 pages


/*  display_cmd I2C command constants
 *
 * Basic SSD1306 command set (minimal subset).
 * 
 */
#define DISPLAY_CMD_DISPLAY_OFF        0xAE
#define DISPLAY_CMD_DISPLAY_ON         0xAF
#define DISPLAY_CMD_SET_CONTRAST       0x81
#define DISPLAY_CMD_SET_NORMAL         0xA6
#define DISPLAY_CMD_SET_INVERSE        0xA7

/*
 * HAL abstraction layer (I2C + timing).
 *
 * All low-level communication is delegated through function pointers.
 */
struct display_device {
    void *context;

    // Write raw I2C bytes to display
    enum display_status (*i2c_write)(void *context, uint8_t addr, uint8_t *data, uint16_t size);

    // Write command byte 
    enum display_status (*i2c_write_cmd)(void *context, uint8_t addr, uint8_t cmd);

    // Delay in milliseconds 
    void (*delay_ms)(void *context, uint32_t ms);

    // Optional: millisecond tick 
    uint32_t (*get_tick_ms)(void *context);
};


// Display driver instance.

typedef struct {
    struct display_device dev;
    uint8_t address;
    uint8_t width;
    uint8_t height;
    uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];
} display_t;


// Initialize display controller.

enum display_status display_init(display_t *disp);


// Turn display ON.

enum display_status display_on(display_t *disp);


// Turn display OFF.

enum display_status display_off(display_t *disp);


// Send raw command.

enum display_status display_send_command(display_t *disp, uint8_t cmd);


// clear display 

enum display_status display_clear(display_t *disp);


// display update

enum display_status display_update(display_t *disp);

// draw pixel
void display_draw_pixel(display_t *disp, uint16_t x, uint16_t y);

// text 

void display_putc(display_t *disp, char c, uint16_t x, uint16_t y);


void display_puts(display_t *disp, const char *str, uint16_t x, uint16_t y);

#endif
