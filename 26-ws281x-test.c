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

struct FALLING_OBJ
{
    float y, v;
    uint32_t ttl;
    uint32_t tail_coord[16];
    float clr[3];
};

static void init_falling_obj(struct FALLING_OBJ* self)
{
    self->y = 0.0;
    self->v = 0.01342;
    self->ttl = 500 + rand() % 900;
    set_pos_16(self->tail_coord, 0 + (uint32_t)(self->y * 144));
    rand_color(self->clr);
}

static void step_falling_obj(struct FALLING_OBJ* self)
{
    erase_leds(self->tail_coord);
    self->v += (-0.00009);
    self->y += self->v;
    self->ttl --;
    if(self->y <= 0.0 || !self->ttl)
    {
        self->y = 0.0;
        self->v = (-self->v);
        self->v *= (1.0/1.25);
        if(!self->ttl)
            init_falling_obj(self);
    }
    fill_colors_16(self->tail_coord, self->clr);
}

static ws2811_return_t effect_01()
{
    ws2811_return_t ret = WS2811_SUCCESS;

    float pos_c[4] = {0.999, 0.999, 0.999, 0.999}, pos_s[4] = {0};
    float clr_c[3] = {0.99, 0.99, 0.99}, clr_s[3] = {0};
    uint32_t si[16] = {0};
    uint32_t ci[16] = {0};
    struct FALLING_OBJ p[2];
    init_falling_obj(&p[0]);
    init_falling_obj(&p[1]);

    led_string.channel[0].brightness = 0xFF;

    set_pos_16(si, 144 + (uint32_t)((pos_s[0] * pos_s[0]
                    + pos_s[1] * pos_s[1]
                    + pos_s[2] * pos_s[2]
                    + pos_s[3] * pos_s[3])/ 4.0 * 144));

    set_pos_16(ci, 144 + (uint32_t)((pos_c[0] * pos_c[0]
                    + pos_c[1] * pos_c[1]
                    + pos_c[2] * pos_c[2]
                    + pos_c[3] * pos_c[3]) / 4.0 * 144));


    while(running)
    {
        erase_leds(si);
        erase_leds(ci);

        pos_c[0] -= pos_s[0] * 0.01;
        pos_s[0] += pos_c[0] * 0.01;
        pos_c[1] -= pos_s[1] * 0.015;
        pos_s[1] += pos_c[1] * 0.015;
        pos_c[2] -= pos_s[2] * 0.0225;
        pos_s[2] += pos_c[2] * 0.0225;
        pos_c[3] -= pos_s[3] * 0.03375;
        pos_s[3] += pos_c[3] * 0.03375;

        clr_c[0] -= clr_s[0] * 0.001;
        clr_s[0] += clr_c[0] * 0.001;
        clr_c[1] -= clr_s[1] * 0.0011;
        clr_s[1] += clr_c[1] * 0.0011;
        clr_c[2] -= clr_s[2] * 0.0012;
        clr_s[2] += clr_c[2] * 0.0012;

        step_falling_obj(&p[0]);
        step_falling_obj(&p[1]);

        push_pos_16(si, 144 + (uint32_t)((pos_s[0] * pos_s[0]
                        + pos_s[1] * pos_s[1]
                        + pos_s[2] * pos_s[2]
                        + pos_s[3] * pos_s[3])/ 4.0 * 144));
        push_pos_16(ci, 144 + (uint32_t)((pos_c[0] * pos_c[0]
                        + pos_c[1] * pos_c[1]
                        + pos_c[2] * pos_c[2]
                        + pos_c[3] * pos_c[3]) / 4.0 * 144));
        push_pos_16(p[0].tail_coord, 0 + (uint32_t)(p[0].y * 144));
        push_pos_16(p[1].tail_coord, 0 + (uint32_t)(p[1].y * 144));

        fill_colors_16(si, (float[]){
                clr_s[0] * clr_s[0] / 2
                , clr_s[1] * clr_s[1] / 2
                , clr_s[2] * clr_s[2] / 2
                });
        fill_colors_16(ci, (float[]){
                clr_c[0] * clr_c[0] / 2
                , clr_c[1] * clr_c[1] / 2
                , clr_c[2] * clr_c[2] / 2
                });

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
