#include "output.h"
#include "platform.h"

#ifdef PLATFORM_DOS

#include "output_sb16.h"

#define OUTPUT_DRIVERS_COUNT 1
static const output_generic_t output_drivers[] = {
    { output_driver_sb16, output_sb16_init, output_sb16_cleanup, output_sb16_start, output_sb16_stop },
};
#else       // DOS

#include "output_portaudio.h"
#include "output_raw.h"
#include "output_benchmark.h"

#define OUTPUT_DRIVERS_COUNT 3
static const output_generic_t output_drivers[] = {
    { output_driver_portaudio, output_portaudio_init, output_portaudio_cleanup, output_portaudio_start, output_portaudio_stop },
    { output_driver_raw, output_raw_init, output_raw_cleanup, output_raw_start, output_raw_stop },
    { output_driver_benchmark, output_benchmark_init, output_benchmark_cleanup, output_benchmark_start, output_benchmark_stop }
};
#endif      // DOS

output_drivers_t output_selected_driver_index = -1;

int output_init(output_opts_t * output_opts) 
{
    int i;
    for (i = 0; i < OUTPUT_DRIVERS_COUNT; i++) {
        if (output_drivers[i].driver == output_opts->driver) {
            output_selected_driver_index = i;
            return output_drivers[i].init(output_opts);
        }
    }
    return -1;
}

int output_cleanup() {
    int ret;
    if (output_selected_driver_index >= 0) {
        ret = output_drivers[output_selected_driver_index].cleanup();
        output_selected_driver_index = -1;
        return ret;
    }
    return -1;
}

int output_start(player_t * player) {
    if (output_selected_driver_index >= 0) {
        return output_drivers[output_selected_driver_index].start(player);
    }
    return -1;
}

int output_stop() {
    if (output_selected_driver_index >= 0) {
        return output_drivers[output_selected_driver_index].stop();
    }
    return -1;
}