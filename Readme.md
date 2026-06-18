# Portable-display-driver

<p align="center">
  <img src="https://raw.githubusercontent.com/Paacmann/portable-display-driver/refs/heads/master/.docs/portable-display.jpeg" alt="Portable Display Driver" width="640">
</p>

## Intro

A lightweight, cross-platform display driver library for embedded devices, written in
portable C11. The driver targets I2C framebuffer displays (e.g. **SSD1306 / SH1106**,
128×64) and is hardware-agnostic - all platform I/O is supplied by the application through
function pointers, making it portable across any MCU or operating system.
The repository ships with a ready-made **STM32F1** port as a reference implementation,
as well as a native Linux simulation port built on top of **SDL2** for desktop
development, testing and debugging without physical hardware.

## Design

This library is designed in a layered manner:

```text
+-------------------+
|    application    | - Embedded application that draws to the display
+-------------------+
          ^
          |
+-------------------+
|      driver       | - Display driver: framebuffer + API, cross-platform (driver/)
+-------------------+
          ^
          |
+-------------------+
|       port        | - Platform port: user configurable, depends on hardware (port/)
+-------------------+
```

The **driver** layer provides a unified API that remains identical across all supported
platforms. It owns an in-RAM framebuffer (`128 * 64 / 8 = 1024 B`); all drawing operations
(`display_clear`, `display_draw_pixel`, `display_putc`, `display_puts`) modify only that
local buffer, and a single `display_update()` flushes it to the panel.

All hardware interaction is abstracted behind a small set of function pointers held in a
`struct display_device`:

| Callback | Description |
|---|---|
| `i2c_write` | Transmit a raw byte buffer over I2C to the display |
| `i2c_write_cmd` | Transmit a single command byte over I2C |
| `delay_ms` | Block for N milliseconds |
| `get_tick_ms` | Optional: return a millisecond tick counter |

The **port** layer connects a specific platform to the driver by implementing those
callbacks.

Reference ports are provided for:

- **STM32F1** (`port/stm32/f1/`) – built on top of the ST HAL and intended for
  deployment on real hardware.
- **Native Linux Simulator** (`port/native_sim/`) – built on top of SDL2 and
  intended for desktop development and testing.

The native simulator implements the same callback interface expected by the driver,
emulates SSD1306 page/column addressing in software and renders the display
framebuffer into an SDL2 window. This allows applications to be developed and
debugged on a desktop machine before running on embedded hardware.

## Driver API

```c
#include "display.h"

/* Lifecycle */
enum display_status display_init(display_t *disp);
enum display_status display_on(display_t *disp);
enum display_status display_off(display_t *disp);

/* Low-level I/O */
enum display_status display_send_command(display_t *disp, uint8_t cmd);

/* Framebuffer */
enum display_status display_clear(display_t *disp);   /* clears local buffer  */
enum display_status display_update(display_t *disp);  /* flushes to the panel */

/* Drawing */
void display_draw_pixel(display_t *disp, uint16_t x, uint16_t y);
void display_putc(display_t *disp, char c, uint16_t x, uint16_t y);
void display_puts(display_t *disp, const char *str, uint16_t x, uint16_t y);
```

Every operation returns an explicit status code:

| Status | Meaning |
|---|---|
| `DISPLAY_SUCCESS` | Operation completed successfully |
| `DISPLAY_ERROR` | Generic I/O error |
| `DISPLAY_BUSY` | Device is busy |
| `DISPLAY_TIMEOUT` | Operation timed out |
| `DISPLAY_ENODEV` | Device handle is `NULL` |
| `DISPLAY_EINVAL` | Invalid argument |

> **Note:** `display_clear` and the drawing functions only touch the in-RAM framebuffer.
> Nothing appears on the panel until you call `display_update()`.

## Porting the Library

To bring the driver up on a new platform, implement the callbacks of `struct display_device`
and populate a `display_t` instance.

**Header (`my_port.h`):**

```c
#include "display.h"

enum display_status i2c_write(void *ctx, uint8_t addr, uint8_t *data, uint16_t size);
enum display_status i2c_write_cmd(void *ctx, uint8_t addr, uint8_t cmd);
void     delay_ms(void *ctx, uint32_t ms);
uint32_t get_tick_ms(void *ctx);
```

**Wiring it up in the application:**

```c
display_t disp = {
    .address = 0x3C,            /* 7-bit SSD1306 address */
    .width   = DISPLAY_WIDTH,
    .height  = DISPLAY_HEIGHT,
    .dev = {
        .context       = &my_ctx,   /* passed back to every callback */
        .i2c_write     = i2c_write,
        .i2c_write_cmd = i2c_write_cmd,
        .delay_ms      = delay_ms,
        .get_tick_ms   = get_tick_ms,
    },
};

display_init(&disp);
display_clear(&disp);
display_puts(&disp, "Hello, world!", 0, 0);
display_update(&disp);
```

See `port/stm32/f1/stm32f1x_display_port.c` for a complete STM32F1 reference port built on
the ST HAL (`HAL_I2C_Master_Transmit`, `HAL_Delay`, `HAL_GetTick`).


## Native Linux Simulator

The native Linux simulator provides a desktop implementation of the port layer
using SDL2. It implements the same callback interface used by embedded targets,
allowing applications built against the driver API to run unmodified on Linux.

Internally, the simulator emulates the SSD1306 command and addressing model,
maintains a software framebuffer and renders its contents into an SDL2 window.

Features:

- SDL2-based display rendering
- SSD1306 page and column addressing emulation
- Display ON/OFF command support
- Normal and inverted display modes
- Portable timing implementation (`clock_gettime()`, `nanosleep()`)
- Useful for development, debugging and demonstrations without hardware

<p align="center">
  <img src="https://raw.githubusercontent.com/Paacmann/portable-display-driver/master/.docs/simulator-display.jpeg"
       alt="Native Linux Simulator"
       width="640">
</p>

The simulator uses the same driver API and callback model as embedded targets,
making it possible to develop and validate application code on a desktop machine
before deploying it to a microcontroller.


## Building

The core driver always builds as a standalone static library; platform ports are gated
behind CMake options.

```sh
cmake -S . -B build -G Ninja
cmake --build build
```
To also build the STM32F1 port (requires an ARM toolchain and the ST HAL on your include
path):

```sh
cmake -S . -B build -G Ninja -DDISPLAY_BUILD_STM32F1_PORT=ON
cmake --build build
```

To build the native Linux simulator (requires SDL2):

```sh
cmake -S . -B build -G Ninja -DDISPLAY_BUILD_NATIVE_SIM=ON
cmake --build build
```

| CMake target | Description |
|---|---|
| `portable_display_driver` | Core, platform-independent driver (always built) |
| `display_port_stm32f1` | STM32F1 reference port (`-DDISPLAY_BUILD_STM32F1_PORT=ON`) |
| `display_port_linux` | Native Linux SDL2 simulator (`-DDISPLAY_BUILD_NATIVE_SIM=ON`) |

## License

The bundled font tables (`driver/fonts.c`, `driver/fonts.h`) originate from the
Majerle/Lutsai LCD font library and are licensed under the **GNU GPLv3**.
The fonts have been slightly modified for integration into this project 
(format adjustments and portability changes), but remain GPLv3-licensed. 
