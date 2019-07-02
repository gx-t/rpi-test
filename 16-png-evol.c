#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <png.h>

#define PIXEL_SIZE 3
#define PIXEL_DEPTH 8
#define WIDTH 200
#define HEIGHT 200
#define DOT_COUNT 400

// time ./16-png-evol > /dev/null
//real    0m0.342s
//user    0m0.341s
//sys 0m0.000s


//x = -100 +100
//y = -100 +100

struct DOT
{
    int x, y, distance;
};

static int rgb_write_png(unsigned width, unsigned height, png_bytep rgb_data)
{
    unsigned y;
    png_bytep rows[height];
    png_structp png_ptr;
    png_infop info_ptr;
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr)
    {
        fprintf(stderr, "PNG structure creation failed.\n");
        return 1;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr)
    {
        fprintf(stderr, "PNG info structure creation failed.\n");
        png_destroy_write_struct(&png_ptr, NULL);
        return 2;
    }
    setjmp(png_jmpbuf(png_ptr));
    png_set_IHDR(png_ptr
            , info_ptr
            , width
            , height
            , PIXEL_DEPTH
            , PNG_COLOR_TYPE_RGB
            , PNG_INTERLACE_NONE
            , PNG_COMPRESSION_TYPE_DEFAULT
            , PNG_FILTER_TYPE_DEFAULT);

    for(y = 0; y < height; y ++)
        rows[y] = &rgb_data[y * PIXEL_SIZE * width];

    png_set_rows(png_ptr, info_ptr, rows);
    png_init_io(png_ptr, stdout);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return 0;
}

static void erase(unsigned width, unsigned height, png_bytep data, png_byte red, png_byte green, png_byte blue)
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
        , png_bytep data
        , unsigned count
        , struct DOT* dots
        , png_byte red
        , png_byte green
        , png_byte blue)
{
    while(count --)
    {
        png_bytep p = data;
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
    png_byte rgb_data[PIXEL_SIZE * width * height];
    srand(time(0));
    init_random_dots(width, height, DOT_COUNT, dots);
    for(file_num = 0; !result && file_num < 30; file_num ++)
    {
        erase(width, height, rgb_data, 64, 64, 64);
        draw_dots(width, height, rgb_data, DOT_COUNT, dots, 255, 255, 255);
        result = rgb_write_png(width, height, rgb_data);
        calc_dot_deltas(width, height, 75, DOT_COUNT, dots);
        qsort(dots, DOT_COUNT, sizeof(struct DOT), (int(*)(const void *, const void *))dot_sort_proc);
        evol_dots(DOT_COUNT, dots);
    }
    return result;
}

