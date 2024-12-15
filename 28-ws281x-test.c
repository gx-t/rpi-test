#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "ws2811.h"

static uint8_t running = 1;
#define LED_COUNT       (50 + 144 + 144)

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
            .count = LED_COUNT,
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
    int ttl;
    uint32_t tail_coord[16];
    float clr[3];
};

static void init_falling_obj(struct FALLING_OBJ* self)
{
    self->y = 0.0;
    self->v = 0.01345 * (3.0/4.0 + (float)rand() / RAND_MAX / 4.0);
    self->ttl = 500 + rand() % 900;
    set_pos_16(self->tail_coord, 50 + (uint32_t)(self->y * 288));
    rand_color(self->clr);
}

static void step_falling_obj(struct FALLING_OBJ* self)
{
    self->v += (-0.00009);
    self->y += self->v;
    self->ttl --;
    if(self->y <= 0.0)
    {
        self->y = 0.0;
        self->v = (-self->v);
        self->v *= (1.0/1.25);
        if(self->ttl <= 0)
            init_falling_obj(self);
    }
    push_pos_16(self->tail_coord, 50 + (uint32_t)(self->y * 288));
}

struct FADING_DOT
{
    int idx;
    float clr[3];
    float ttl;
};

static void init_fading_dot(struct FADING_DOT* self, int idx)
{
    self->idx = idx;
    self->clr[0] = self->clr[1] = self->clr[2] = 0.0;
    self->ttl = rand() % 200;
}

static void step_fading_dot(struct FADING_DOT* self)
{
    uint32_t clr_i = (int)(self->clr[0] * 255)
        | ((int)(self->clr[1] * 255) << 8)
        | ((int)(self->clr[2] * 255) << 16);
    led_string.channel[0].leds[self->idx] = clr_i;
    self->clr[0] *= 0.97;
    self->clr[1] *= 0.97;
    self->clr[2] *= 0.97;
    self->ttl --;
    if(self->ttl <= 0)
    {
        rand_color(self->clr);
        self->ttl = 100 + rand() % 500;
    }
}

static ws2811_return_t effect_01()
{
    ws2811_return_t ret = WS2811_SUCCESS;

    struct FALLING_OBJ p[2];
    init_falling_obj(&p[0]);
    init_falling_obj(&p[1]);

    struct FADING_DOT d[50];
    for(int i = 0; i < sizeof(d) / sizeof(d[0]); i ++)
        init_fading_dot(&d[i], i);

    led_string.channel[0].brightness = 0xFF;

    while(running)
    {
        erase_leds(p[0].tail_coord);
        erase_leds(p[1].tail_coord);

        step_falling_obj(&p[0]);
        step_falling_obj(&p[1]);
        for(int i = 0; i < sizeof(d) / sizeof(d[0]); i ++)
            step_fading_dot(&d[i]);

        fill_colors_16(p[0].tail_coord, p[0].clr);
        fill_colors_16(p[1].tail_coord, p[1].clr);

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
