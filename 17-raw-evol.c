#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#define FRAME_WIDTH 400
#define FRAME_HEIGHT 400
#define FRAME_COUNT 300
#define RAD 100

static uint32_t in_buff[FRAME_HEIGHT][FRAME_WIDTH] = {0};
static uint32_t out_buff[FRAME_HEIGHT][FRAME_WIDTH] = {0};

static uint32_t make_color(uint8_t r, uint8_t g, uint8_t b)
{
    return 0xFF000000 | (b << 16) | (g << 8) | r; 
}

static void fill_random_dots()
{
    for(int y = 0; y < FRAME_HEIGHT; y ++) {
        for(int x = 0; x < FRAME_WIDTH; x ++) {
            out_buff[y][x] = make_color(rand() % 0xFF, rand() % 0xFF, rand() % 0xFF);
        }
    }
}

static void evol()
{
    for(int i = 0; i < FRAME_COUNT; i ++) {
        int cnt = 100;
        int x = 0;
        int y = 0;
        do {
            x = rand() % FRAME_WIDTH;
            y = rand() % FRAME_HEIGHT;
            cnt --;
        } while(cnt && out_buff[y][x] == in_buff[y][x]);
        out_buff[y][x] = in_buff[y][x];
    }
}

int main()
{
    int result = 0;
    srand(time(0));
    fread(in_buff, sizeof(in_buff), 1, stdin);
    fill_random_dots();
    for(int i = 0; i < 600; i ++)
    {
        fwrite(out_buff, sizeof(out_buff), 1, stdout);
        evol(0);
    }
    return result;
}

