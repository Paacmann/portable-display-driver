#define _GNU_SOURCE

#include "native_sim_display_port.h"


#include <time.h>
#include <unistd.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <string.h>
#include <time.h>
#include <stdint.h>


/* Helper function */

static void render_framebuffer(native_sim_ctx_t *ctx) {
    
    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);

    if (!ctx->display_on) {
        SDL_RenderPresent(ctx->renderer);
        return;
    }

    for (uint32_t y = 0; y < ctx->height; y++) {
        for (uint32_t x = 0; x < ctx->width; x++) {

            uint32_t byte_index = x + (y / 8u) * ctx->width;
            uint8_t bit = y % 8u;

            uint8_t byte = ctx->framebuffer[byte_index];
            uint8_t pixel_on = (byte >> bit) & 1u;

            if (ctx->invert) {
                pixel_on = !pixel_on;
            }

            SDL_SetRenderDrawColor(
                ctx->renderer,
                pixel_on ? 255 : 0,
                pixel_on ? 255 : 0,
                pixel_on ? 255 : 0,
                255
            );

            SDL_Rect r = {
                (int)(x * ctx->scale),
                (int)(y * ctx->scale),
                (int)ctx->scale,
                (int)ctx->scale
            };

            SDL_RenderFillRect(ctx->renderer, &r);
        }
    }

    SDL_RenderPresent(ctx->renderer);
}

/* Init function */

int native_sim_init(native_sim_ctx_t *ctx, uint32_t width, uint32_t height, uint32_t scale) {
    
    if (!ctx) {
        
        return -1;
    }

    memset(ctx, 0, sizeof(*ctx));
    
    ctx->display_on = 1;
    ctx->invert = 0;
    ctx->page = 0;
    ctx->column = 0;
    ctx->width  = width;
    ctx->height = height;
    ctx->scale  = scale;
    ctx->running = 1;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        
        return -1;
    }

    ctx->window = SDL_CreateWindow(
        "Display Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        (int)(width * scale),
        (int)(height * scale),
        0
    );

    if (!ctx->window) {
        
        return -1;
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED);

    if (!ctx->renderer) {
        
        return -1;
    }

    /* texture optional (not strictly needed here) */
    ctx->texture = NULL;

    return 0;
}

/* Deinit */

void native_sim_deinit(native_sim_ctx_t *ctx) {
    
    if (!ctx) {
        
        return;
    }

    if (ctx->texture) {
        
        SDL_DestroyTexture(ctx->texture);
    }

    if (ctx->renderer) {
        
        SDL_DestroyRenderer(ctx->renderer);
    }

    if (ctx->window) {
        
        SDL_DestroyWindow(ctx->window);
    }

    SDL_Quit();
}

/* Event handling */

void native_sim_poll_events(native_sim_ctx_t *ctx) {
    
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            ctx->running = 0;
        }
    }
}

/* Present framebuffer */

void native_sim_present(native_sim_ctx_t *ctx) {
    
    if (!ctx) {
        
        return;
    }

    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);
    SDL_RenderClear(ctx->renderer);

    render_framebuffer(ctx);
}

enum display_status native_sim_i2c_write(void *context, uint8_t addr, uint8_t *data,uint16_t size) {
    
    (void)addr;

    native_sim_ctx_t *ctx = context;

    if (!ctx || size < 2) {
        
        return DISPLAY_EINVAL;
    }

    /* SSD1306: data[0] = control byte (0x40), data[1..] payload */
    for (uint16_t i = 1; i < size; i++) {
        
        uint16_t index = ctx->page * ctx->width + ctx->column;

        if (index >= (ctx->width * 8)) {
            
            return DISPLAY_EINVAL;
        }

        ctx->framebuffer[index] = data[i];

        ctx->column++;

        if (ctx->column >= ctx->width) {
            
            ctx->column = 0;
            ctx->page++;

            if (ctx->page >= 8) {
                ctx->page = 0;
            }
        }
    }

    return DISPLAY_SUCCESS;
}

/* Display driver callbacks , I2C emulator */
enum display_status native_sim_i2c_write_cmd(void *context, uint8_t addr, uint8_t cmd) {
    
    (void)addr;

    native_sim_ctx_t *ctx = (native_sim_ctx_t *)context;

    if (!ctx) {
        
        return DISPLAY_ENODEV;
    }

    switch (cmd) {
        /* ---------------------------
         * Display ON / OFF
         * ------------------------- */
        case 0xAE:  /* display off */
            ctx->display_on = 0;
            break;

        case 0xAF:  /* display on */
            ctx->display_on = 1;
            break;

        /* ---------------------------
         * Invert display
         * ------------------------- */
        case 0xA6:  /* normal */
            ctx->invert = 0;
            break;

        case 0xA7:  /* inverted */
            ctx->invert = 1;
            break;

        /* ---------------------------
         * Reset addressing
         * ------------------------- */
        case 0xB0 ... 0xB7:  /* set page */
            ctx->page = cmd & 0x07;
            break;

        case 0x00 ... 0x0F:   /* lower column */
            ctx->column = (uint8_t)((ctx->column & 0xF0) | (cmd & 0x0F));
            break;

        case 0x10 ... 0x1F:   /* upper column */
            ctx->column = (uint8_t)((ctx->column & 0x0F) | ((cmd & 0x0F) << 4));
            break;

        default:
            /* ignore others commands */
            break;
    }

    return DISPLAY_SUCCESS;
}

/* Timing */

uint32_t native_sim_get_tick_ms(void *context) {
    
    (void)context;

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint32_t)((ts.tv_sec * 1000u) + (ts.tv_nsec / 1000000u));
}


void native_sim_delay_ms(void *context, uint32_t delay_ms) {
    
    (void)context;

    struct timespec ts = {
        .tv_sec  = delay_ms / 1000u,
        .tv_nsec = (delay_ms % 1000u) * 1000000u
    };

    nanosleep(&ts, NULL);
}






