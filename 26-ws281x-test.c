#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "ws2811.h"

static uint8_t running = 1;

static ws2811_t led_string =
{
    .freq = WS2811_TARGET_FREQ,
    .dmanum = 10,
    .channel =
    {
        [0] =
        {
            .gpionum = 18,
            .invert = 0,
            .count = 144,
            .strip_type = WS2811_STRIP_GBR,
            .brightness = 255,
        },
        [1] =
        {
            .gpionum = 0,
            .invert = 0,
            .count = 0,
            .brightness = 0,
        },
    },
};

static void ctrl_c(int signum)
{
    (void)(signum);
    running = 0;
    signal(SIGINT, ctrl_c);
}

static ws2811_return_t leds_off()
{
    ws2811_return_t ret = WS2811_SUCCESS;
    memset(led_string.channel[0].leds, 0, led_string.channel[0].count * sizeof(ws2811_led_t));
    if((ret = ws2811_render(&led_string)) != WS2811_SUCCESS)
        fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));

    return ret;
}

static ws2811_return_t effect_00()
{
    ws2811_return_t ret = WS2811_SUCCESS;

    float c[3] = {0.99, 0.99, 0.99}, s[3] = {0};
    const float f[3] = {0.01, 0.011, 0.012};
    int si = 0;
    int ci = 0;

    while(running)
    {
        led_string.channel[0].leds[si] = 0x00;
        led_string.channel[0].leds[ci] = 0x00;
        c[0] += s[0] * f[0];
        s[0] -= c[0] * f[0];
        c[1] += s[1] * f[1];
        s[1] -= c[1] * f[1];
        c[2] += s[2] * f[2];
        s[2] -= c[2] * f[2];

        si = (int)((s[0] * s[0]
                    + s[1] * s[1]
                    + s[2] * s[2]) / 3 * led_string.channel[0].count);
        ci = (int)((c[0] * c[0]
                    + c[1] * c[1]
                    + c[2] * c[2]) / 3 * led_string.channel[0].count);

        led_string.channel[0].leds[si] |= 0xFF;
        led_string.channel[0].leds[ci] |= (0xFF << 16);

        if((ret = ws2811_render(&led_string)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
            break;
        }
        usleep(1000000 / 100);
    }
    return ret;
}

static void set_16(uint32_t arr[16], uint32_t val)
{
    for(int i = 0; i < 16; i ++)
        arr[i] = val;
}

static void push_16(uint32_t arr[16], uint32_t val)
{
    uint32_t* pp1 = &arr[15];
    uint32_t* pp2 = &arr[14];
    while(pp2 > arr)
        *pp1 -- = *pp2 --;
    *pp1 = *pp2;
    *pp2 = val;
}

static void erase_leds(const uint32_t arr[16])
{
    for(int i = 0; i < 16; i ++)
        led_string.channel[0].leds[arr[i]] = 0x00;
}

static void fill_leds_16(const uint32_t arr[16], const uint32_t color_arr[16])
{
    for(int i = 0; i < 16; i ++)
        led_string.channel[0].leds[arr[i]] |= color_arr[i];
}

static ws2811_return_t effect_01()
{
    ws2811_return_t ret = WS2811_SUCCESS;

    float c[3] = {0.99, 0.99, 0.99}, s[3] = {0};
    const float f[3] = {0.01, 0.011, 0.012};
    uint32_t si[16] = {0};
    uint32_t ci[16] = {0};

    const uint32_t color_tbl_s_16[] = 
    {
        0x00FF0000
            , 0x00B60000
            , 0x00820000
            , 0x005C0000
            , 0x00420000
            , 0x002F0000
            , 0x00210000
            , 0x00180000
            , 0x00110000
            , 0x000C0000
            , 0x00080000
            , 0x00060000
            , 0x00040000
            , 0x00030000
            , 0x00020000
            , 0x00010000
    };

    const uint32_t color_tbl_c_16[] =
    {
        0x000000FF
            , 0x000000B6
            , 0x00000082
            , 0x0000005C
            , 0x00000042
            , 0x0000002F
            , 0x00000021
            , 0x00000018
            , 0x00000011
            , 0x0000000C
            , 0x00000008
            , 0x00000006
            , 0x00000004
            , 0x00000003
            , 0x00000002
            , 0x00000001
    };

    set_16(si, (int)((s[0] * s[0]
                    + s[1] * s[1]
                    + s[2] * s[2]) / 3 * led_string.channel[0].count));
    set_16(ci, (int)((c[0] * c[0]
                    + c[1] * c[1]
                    + c[2] * c[2]) / 3 * led_string.channel[0].count));

    while(running)
    {
        erase_leds(si);
        erase_leds(ci);
        c[0] += s[0] * f[0];
        s[0] -= c[0] * f[0];
        c[1] += s[1] * f[1];
        s[1] -= c[1] * f[1];
        c[2] += s[2] * f[2];
        s[2] -= c[2] * f[2];

        push_16(si, (int)((s[0] * s[0]
                        + s[1] * s[1]
                        + s[2] * s[2]) / 3 * led_string.channel[0].count));
        push_16(ci, (int)((c[0] * c[0]
                        + c[1] * c[1]
                        + c[2] * c[2]) / 3 * led_string.channel[0].count));

        fill_leds_16(si, color_tbl_s_16);
        fill_leds_16(ci, color_tbl_c_16);

        if((ret = ws2811_render(&led_string)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
            break;
        }
        usleep(1000000 / 100);
    }
    return ret;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, ctrl_c);
    signal(SIGTERM, ctrl_c);
    ws2811_return_t ret = WS2811_SUCCESS;
    srand(time(NULL));

    if((ret = ws2811_init(&led_string)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    (void)((ret = effect_01())
            || (ret = leds_off()));

    ws2811_fini(&led_string);
    fprintf(stderr, "\n");
    return ret;
}
