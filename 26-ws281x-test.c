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
            .count = 288,
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

static void set_pos_16(uint32_t pos_arr[16], uint32_t pos)
{
    for(int i = 0; i < 16; i ++)
        pos_arr[i] = pos;
}

static void push_pos_16(uint32_t pos_arr[16], uint32_t new_pos)
{
    uint32_t* pp1 = &pos_arr[15];
    uint32_t* pp2 = &pos_arr[14];
    while(pp2 > pos_arr)
        *pp1 -- = *pp2 --;
    *pp1 = *pp2;
    *pp2 = new_pos;
}

static void erase_leds(const uint32_t arr[16])
{
    for(int i = 0; i < 16; i ++)
        led_string.channel[0].leds[arr[i]] = 0x00;
}

static void fill_colors_16(const uint32_t arr[16], const float head_clr[3])
{
    float r = head_clr[0];
    float g = head_clr[1];
    float b = head_clr[2];
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

static void rand_color(float clr[3])
{
    float red = (float)rand() / RAND_MAX;
    float green = (float)rand() / RAND_MAX;
    float blue = (float)rand() / RAND_MAX;
    float coef = 1.0 / (red + green + blue);
    clr[0] = red * coef;
    clr[1] = green * coef;
    clr[2] = blue * coef;
}

static ws2811_return_t effect_01()
{
    ws2811_return_t ret = WS2811_SUCCESS;

    float c[8] = {0.999, 0.99, 0.99, 0.99}, s[8] = {0};
    float fp = 0.01;
    float y = 0.0, v = 0.01342;
    uint32_t si[16] = {0};
    uint32_t ci[16] = {0};
    uint32_t pi[16] = {0};
    float clr[3];
    rand_color(clr);

    led_string.channel[0].brightness = 0xFF;

    set_pos_16(si, 144 + (uint32_t)(s[0] * s[0] * 144));
    set_pos_16(ci, 144 + (uint32_t)(c[0] * c[0] * 144));
    set_pos_16(pi, 0 + (uint32_t)(y * 144));

    while(running)
    {
        erase_leds(si);
        erase_leds(ci);
        erase_leds(pi);

        c[0] -= s[0] * fp;
        s[0] += c[0] * fp;
        c[1] -= s[1] * 0.001;
        s[1] += c[1] * 0.001;
        c[2] -= s[2] * 0.0011;
        s[2] += c[2] * 0.0011;
        c[3] -= s[3] * 0.0012;
        s[3] += c[3] * 0.0012;

        fp += c[2] * 0.00001;

        v += (-0.00009);
        y += v;
        if(y <= 0.0)
        {
            y = 0.0;
            v = (-v);
            v *= (1.0/1.25);
            if(v < 0.0015)
            {
                v = 0.01342;
                rand_color(clr);
            }
        }

        push_pos_16(si, 144 + (uint32_t)(s[0] * s[0] * 144));
        push_pos_16(ci, 144 + (uint32_t)(c[0] * c[0] * 144));
        push_pos_16(pi, 0 + (uint32_t)(y * 144));
        fill_colors_16(si, (float[]){s[1] * s[1] / 2, s[2] * s[2] / 2, s[3] * s[3] / 2});
        fill_colors_16(ci, (float[]){c[1] * c[1] / 2, c[2] * c[2] / 2, c[3] * c[3] / 2});
        fill_colors_16(pi, clr);

        if((ret = ws2811_render(&led_string)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
            break;
        }
        usleep(1000000/100);
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
