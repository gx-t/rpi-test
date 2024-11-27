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

    led_string.channel[0].brightness = 0xFF;
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

static void fill_leds_16(const uint32_t arr[16], const float clr[3])
{
    float r = clr[0];
    float g = clr[1];
    float b = clr[2];
    for(int i = 0; i < 16; i ++)
    {
        uint32_t clr_i = (int)(r * 255)
            | ((int)(g * 255) << 8)
            | ((int)(b * 255) << 16);

        led_string.channel[0].leds[arr[i]] |= clr_i;

        r /= 1.41;
        g /= 1.41;
        b /= 1.41;
    }
}

static void step_8(float c[8], float s[8], const float f[8])
{
    for(int i = 0; i < 8; i ++)
    {
        c[i] -= s[i] * f[i];
        s[i] += c[i] * f[i];
    }
}

static ws2811_return_t effect_01()
{
    ws2811_return_t ret = WS2811_SUCCESS;

    float c[8] = {0.999, 0.00, 0.00, 0.00, 0.00, 0.99, 0.99, 0.99}, s[8] = {0};
    float f[8] = {0.01, 0.01, 0.012, 0.013, 0.014, 0.001, 0.0011, 0.0012};
    uint32_t si[16] = {0};
    uint32_t ci[16] = {0};

    led_string.channel[0].brightness = 0xFF;

    set_16(si, (int)((s[0] * s[0]
                    + s[1] * s[1]
                    + s[2] * s[2]
                    + s[3] * s[3]
                    + s[4] * s[4]) / 5 * led_string.channel[0].count));
    set_16(ci, (int)((c[0] * c[0]
                    + c[1] * c[1]
                    + c[2] * c[2]
                    + c[3] * c[3]
                    + c[4] * c[4]) / 5 * led_string.channel[0].count));

    while(running)
    {
        erase_leds(si);
        erase_leds(ci);
        step_8(c, s, f);
        f[0] += c[6] * 0.00001;
//        f[1] += c[5] * 0.00001;

        push_16(si, (int)((s[0] * s[0]
                        + s[1] * s[1]
                        + s[2] * s[2]
                        + s[3] * s[3]
                        + s[4] * s[4]) / 1 * led_string.channel[0].count));
        push_16(ci, (int)((c[0] * c[0]
                        + c[1] * c[1]
                        + c[2] * c[2]
                        + c[3] * c[3]
                        + c[4] * c[4]) / 1 * led_string.channel[0].count));

        fill_leds_16(si, (float[]){s[5] * s[5] / 2, s[6] * s[6] / 2, s[7] * s[7] / 2});
        fill_leds_16(ci, (float[]){c[5] * c[5] / 2, c[6] * c[6] / 2, c[7] * c[7] / 2});

        if((ret = ws2811_render(&led_string)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
            break;
        }
        usleep(1000000 / 100);
    }
    return ret;
}

static void fill_random_color(uint32_t* buff)
{
    const uint32_t clr_tbl[] = {
        0x00FF0000
        , 0x0000FF00
        , 0x000000FF
        , 0x00888800
        , 0x00880088
        , 0x00008888
        , 0x00888888
        , 0x00000000
    };
    for(int i = 0; i < led_string.channel[0].count; i ++)
        buff[i] = clr_tbl[rand() % sizeof(clr_tbl) / sizeof(clr_tbl[0])];
}

static void fill_led_all(const uint32_t* buff)
{
    memcpy(led_string.channel[0].leds
            , buff
            , led_string.channel[0].count * sizeof(led_string.channel[0].leds[0]));
}

static void rand_swap(uint32_t* buff)
{
    for(int i = 0; i < led_string.channel[0].count; i ++)
    {
        int dir = rand() & 2 ? -1 : 1;
        int new_pos = i + dir;
        if(0 > new_pos || new_pos >= led_string.channel[0].count)
            continue;
        uint32_t vv = buff[i];
        buff[i] = buff[new_pos];
        buff[new_pos] = vv;
    }
}

static ws2811_return_t effect_02()
{
    ws2811_return_t ret = WS2811_SUCCESS;
//    uint32_t buff[led_string.channel[0].count];
    led_string.channel[0].brightness = 64;
//    fill_random_color(buff);
   float cc[] = {0.99, 0.99, 0.99};
   float cs[] = {0.0, 0.0, 0.0};
   const float cf[] = {0.011, 0.012, 0.013};

   while(running)
    {
        cc[0] -= cs[0] * cf[0];
        cs[0] += cc[0] * cf[0];
        cc[1] -= cs[1] * cf[1];
        cs[1] += cc[1] * cf[1];
        cc[2] -= cs[2] * cf[2];
        cs[2] += cc[2] * cf[2];
//        fill_led_all(buff);
//        rand_swap(buff);
        uint32_t clr_i = (uint32_t)(cc[0] * cc[0] * 255)
            | ((uint32_t)(cc[1] * cc[1] * 255) << 8)
            | ((uint32_t)(cc[2] * cc[2] * 255) << 16);
        led_string.channel[0].leds[0] = clr_i;
        if((ret = ws2811_render(&led_string)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
            break;
        }
        usleep(1000000 / 10);
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
