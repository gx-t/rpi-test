//#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#define WIDTH 400
#define HEIGHT 400
#define DOT_COUNT 16000


struct DOT
{
    int x, y, distance;
};

static void erase(uint32_t bitmap[HEIGHT][WIDTH], unsigned width, unsigned height, uint32_t color)
{
    for(unsigned y = 0; y < HEIGHT; y ++)
        for(unsigned x = 0; x < WIDTH; x ++)
            bitmap[y][x] = color;
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
        , uint32_t bitmap[HEIGHT][WIDTH]
        , unsigned count
        , struct DOT* dots
        , uint32_t color)
{
    while(count --)
    {
        bitmap[dots->y + height / 2][dots->x + width / 2] = color;
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
        if(p1->x > -WIDTH / 2 + 1 && p1->x < WIDTH / 2 - 1)
            p2->x = p1->x + (rand() & 1 ? 1 : -1);
        if(p1->y > -HEIGHT / 2 + 1 && p1->y < HEIGHT / 2 - 1)
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
    unsigned width = WIDTH, height = HEIGHT;
    uint32_t bitmap[HEIGHT][WIDTH];
    srand(time(0));
    init_random_dots(width, height, DOT_COUNT, dots);
    for(int i = 0; i < 10; i ++)
    {
        erase(bitmap, WIDTH, HEIGHT, 0);
        draw_dots(width, height, bitmap, DOT_COUNT, dots, 0xFF00FF00);
        write(STDOUT_FILENO, bitmap, sizeof(bitmap));
        evol_dots(DOT_COUNT, dots);
        calc_dot_deltas(width, height, 100, DOT_COUNT, dots);
        qsort(dots, DOT_COUNT, sizeof(struct DOT), (int(*)(const void *, const void *))dot_sort_proc);
    }

//    for(unsigned y = 0; y < HEIGHT; y ++) {
//        for(unsigned x = 0; x < WIDTH; x ++) {
//            bitmap[y][x] |= 0xFF00FF00;
//            bitmap[HEIGHT - y - 1][WIDTH - x - 1] |= 0xFF0000FF;
//            write(STDOUT_FILENO, bitmap, sizeof(bitmap));
//        }
//    }

    return result;
}

