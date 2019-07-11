#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#define WIDTH 200
#define HEIGHT 200
#define DOT_COUNT 400

//time ./17-raw-evol > /dev/null
//uint8_t and uint32_t:
//real    0m0.019s
//user    0m0.018s
//sys 0m0.002s


struct DOT
{
    int x, y, distance;
};

static void erase(unsigned width, unsigned height, uint32_t* bitmap, uint32_t color)
{
    unsigned x, y;
    for(y = 0; y < height; y ++)
        for(x = 0; x < width; x ++)
            *bitmap ++ = color;
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

static void draw_dots(unsigned width, unsigned height, uint32_t* bitmap, unsigned count, struct DOT* dots, uint32_t color)
{
    while(count --)
    {
        uint32_t* p = bitmap;
        p += (dots->x + width / 2 + (dots->y + height / 2) * width);
        *p ++ = color;
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

static int run_ffmpeg()
{
    char* const argv[] = {
        "ffmpeg"
            , "-y"
            , "-v"
            , "error"
            , "-s"
            , "200x200"
            , "-pix_fmt"
            , "rgba"
            , "-f"
            , "rawvideo"
            , "-r"
            , "2"
            , "-i"
            , "-"
            , "-c:v"
            , "libx264"
            , "-vf"
            , "fps=25,format=yuv420p"
            , "18-out.mp4"
            , NULL
    };
    int pp[2];
    signal(SIGCHLD, SIG_IGN);
    if(0 > pipe(pp)) {
        perror("pipe");
        return 1;
    }
    pid_t pid = fork();
    if(!pid) {
        dup2(pp[0], STDIN_FILENO);
        close(pp[1]);
        execv("/usr/bin/ffmpeg", argv);
        perror("execv");
        return 2;
    }
    close(pp[0]);
    if(0 > pid) {
        perror("fork");
        close(pp[1]);
        return 3;
    }
    dup2(pp[1], STDOUT_FILENO);
    return 0;
}

int main()
{
    struct DOT dots[DOT_COUNT]; 
    unsigned width = WIDTH, height = HEIGHT, file_num;
    uint32_t bitmap[width * height];
    srand(time(0));
    int result = run_ffmpeg();
    if(result)
        return result;
    init_random_dots(width, height, DOT_COUNT, dots);
    for(file_num = 0; !result && file_num < 30; file_num ++)
    {
        erase(width, height, bitmap, 0x00000000);
        draw_dots(width, height, bitmap, DOT_COUNT, dots, 0xFFFFFFFF);
        fwrite(bitmap, sizeof(bitmap), 1, stdout);
        calc_dot_deltas(width, height, 75, DOT_COUNT, dots);
        qsort(dots, DOT_COUNT, sizeof(struct DOT), (int(*)(const void *, const void *))dot_sort_proc);
        evol_dots(DOT_COUNT, dots);
    }
    fflush(stdout);
    return result;
}

