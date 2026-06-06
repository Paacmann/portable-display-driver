#include "display.h"
#include "native_sim_display_port.h"

int main() {

    native_sim_ctx_t sim;

    native_sim_init(&sim, 128, 64, 4);

    display_t disp = {0};

    disp.address = 0x3C;
    disp.width   = 128;
    disp.height  = 64;

    disp.dev.context       = &sim;
    disp.dev.i2c_write     = native_sim_i2c_write;
    disp.dev.i2c_write_cmd = native_sim_i2c_write_cmd;

    display_init(&disp);
    display_clear(&disp);

    display_puts(&disp, "DISPLAY DRIVER", 10, 20);
    display_update(&disp);

    while (sim.running) {

        native_sim_poll_events(&sim);
        native_sim_present(&sim);
        SDL_Delay(16);
    }

    native_sim_deinit(&sim);

    return 0;
}
