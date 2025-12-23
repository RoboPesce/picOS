#include "pico/multicore.h"
#include "core1.h"

#include "drivers/st7789_driver.h"

void start_core1()
{
    multicore_launch_core1(core1_main);
}

void core1_main()
{
    st7789_init();

#if FRAMERATE > 0
    uint32_t last_drew_fb = to_ms_since_boot(get_absolute_time());
#endif

    while (true)
    {
        while (/* TODO: Wait for packet from core 0 */) tight_loop_contents();

        // Draw framebuffer, restricted by framerate if configured
#if FRAMERATE > 0
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_drew_fb >= (1.0 / framerate) * 1000)
        {
            draw_framebuffer();
            last_drew_fb = now;
        }
#else
        draw_framebuffer();
#endif

        // TODO: Pop packet
    }
}