#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define PIXEL_SIZE 3
#define PIXEL_DEPTH 8
#define WIDTH 200
#define HEIGHT 200
#define DOT_COUNT 400


//x = -100 +100
//y = -100 +100

struct DOT
{
    int x, y, distance;
};

static void erase(unsigned width, unsigned height, uint8_t* data, uint8_t red, uint8_t green, uint8_t blue)
{
    unsigned x, y;
    for(y = 0; y < height; y ++)
    {
        for(x = 0; x < width; x ++)
        {
            *data ++ = red;
            *data ++ = green;
            *data ++ = blue;
        }
    }
}

static void init_random_dots(unsigned width, unsigned height, unsigned count, struct DOT* dots)
{
    while(count --)
    {
        dots->x = (rand() % width) - (width / 2);
        dots->y = (rand() % height) - (height / 2);
        dots->distance = 0;
        dots ++;
    }
}

static void draw_dots(unsigned width
        , unsigned height
        , uint8_t* data
        , unsigned count
        , struct DOT* dots
        , uint8_t red
        , uint8_t green
        , uint8_t blue)
{
    while(count --)
    {
        uint8_t* p = data;
        p += (dots->x + width / 2 + (dots->y + height / 2) * width) * PIXEL_SIZE;
        *p ++ = red;
        *p ++ = green;
        *p ++ = blue;
        dots ++;
    }
}

static void calc_dot_deltas(unsigned width, unsigned height, unsigned radius, unsigned count, struct DOT* dots)
{
    while(count --)
    {
        int delta = (dots->x * dots->x + dots->y * dots->y) - (radius * radius);
        dots->distance = delta * delta;
        dots ++;
    }
}

static int dot_sort_proc(struct DOT* dot1, struct DOT* dot2)
{
    return dot1->distance - dot2->distance;
}

static void evol_dots(unsigned count, struct DOT* dots)
{
    struct DOT* p1 = dots;
    struct DOT* p2 = dots + count - 1;
    count /= 2;
    while(count --)
    {
        p2->x = p1->x + (rand() & 1 ? 1 : -1);
        p2->y = p1->y + (rand() & 1 ? 1 : -1);
        p2->distance = 0;
        p1 ++;
        p2 --;
    }
}

int main()
{
    int result = 0;
    struct DOT dots[DOT_COUNT]; 
    unsigned width = WIDTH, height = HEIGHT, file_num;
    uint8_t bitmap[PIXEL_SIZE * width * height];
    srand(time(0));
    init_random_dots(width, height, DOT_COUNT, dots);
    for(file_num = 0; !result && file_num < 30; file_num ++)
    {
        erase(width, height, bitmap, 64, 64, 64);
        draw_dots(width, height, bitmap, DOT_COUNT, dots, 255, 255, 255);
        fwrite(bitmap, sizeof(bitmap), 1, stdout);
        fflush(stdout);
        calc_dot_deltas(width, height, 75, DOT_COUNT, dots);
        qsort(dots, DOT_COUNT, sizeof(struct DOT), (int(*)(const void *, const void *))dot_sort_proc);
        evol_dots(DOT_COUNT, dots);
    }
    return result;
}

