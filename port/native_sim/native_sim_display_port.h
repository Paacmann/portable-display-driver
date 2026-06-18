#ifndef NATIVE_SIM_DISPLAY_PORT_H
#define NATIVE_SIM_DISPLAY_PORT_H

#include "display.h"

#include <SDL2/SDL.h>
#include <stddef.h>
#include <stdint.h>

/*
    *  SDL2 simulation context.
    *
    *  One instance represents a single simulated display.
 *
 * */

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    uint32_t width;
    uint32_t height;

    uint32_t scale;

    uint8_t framebuffer[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    int running;
    
    uint8_t display_on;
    uint8_t invert;
    uint8_t page;
    uint8_t column;


} native_sim_ctx_t;



int native_sim_init(native_sim_ctx_t *ctx, uint32_t width, uint32_t height , uint32_t scale);


void native_sim_deinit(native_sim_ctx_t *ctx);

void native_sim_poll_events(native_sim_ctx_t *ctx);

void native_sim_present(native_sim_ctx_t *ctx);


// ----------------------------------------------
//              Callbacks
// ----------------------------------------------

enum display_status native_sim_i2c_write(void *context ,uint8_t addr , uint8_t *data , uint16_t size);

enum display_status native_sim_i2c_write_cmd(void *context , uint8_t addr , uint8_t cmd);


uint32_t native_sim_get_tick_ms(void *context);

void native_sim_delay_ms(void *context, uint32_t delay_ms);



#endif
