#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <signal.h>

#define BYTES_PER_PIXEL     4

struct ALL {
    struct {
        uint32_t xoffset
            , yoffset
            , line_length
            , size;
        int32_t fd;
        uint8_t* fbp;
    } screen;

    struct {
        uint32_t gs;
        struct {
            uint32_t rad
                , cx
                , cy;
            int32_t vx
                , vy;
        } ball;

        struct {
            int32_t x0
                , x1
                , x2
                , x3;
            int32_t length;
            uint32_t c0
                , c1
                , c2
                , c3;
        } wall;

        struct {
            uint32_t bground
                , border
                , ball;
        } color;
    } board;

};

volatile int32_t running;

#define BOARD_LEFT      300
#define BOARD_TOP       300
#define BOARD_WIDTH     300
#define BOARD_HEIGHT    200

static void draw_borders(struct ALL* all, uint32_t* pp, int x, int y)
{
    uint32_t r = BOARD_WIDTH - 1
        , b = BOARD_HEIGHT - 1;

    if(0 == x || r == x || 0 == y || b == y) {
        *pp = all->board.color.border;
    }
}

static void process_ball_collision(struct ALL* all, int32_t cx, int32_t cy)
{
    all->board.ball.vx = -all->board.ball.vx;
    all->board.ball.vy = -all->board.ball.vy;
    if(0 == cx) {
        all->board.ball.vx = -all->board.ball.vx;
    } else if(0 == cy) {
        all->board.ball.vy = -all->board.ball.vy;
    }
}

static void draw_line(uint32_t* pp, int x, int y, int32_t cx, int32_t cy, int32_t len, uint32_t color)
{
    if(cy != y) 
        return;
    if(x < cx || x > cx + len)
        return;

    *pp = color;
}

static void draw_walls(struct ALL* all, uint32_t* pp, int x, int y)
{
    int32_t sy = 4 * all->board.gs;
    int32_t cy = sy;
    draw_line(pp, x, y, all->board.wall.x0, cy, all->board.wall.length, 0x00FF0000);
    cy += sy;
    draw_line(pp, x, y, all->board.wall.x1, cy, all->board.wall.length, 0x0000FF00);
    cy += sy;
    draw_line(pp, x, y, all->board.wall.x2, cy, all->board.wall.length, 0x000000FF);
    cy += sy;
    draw_line(pp, x, y, all->board.wall.x3, cy, all->board.wall.length, 0x0000FFFF);
}

static void draw_ball(struct ALL* all, uint32_t* pp, int x, int y)
{
    x -= (int)all->board.ball.cx;
    y -= (int)all->board.ball.cy;
    int r = (int)all->board.ball.rad;

    if(x * x + y * y < r * r) {
        if(*pp != all->board.color.bground)
            process_ball_collision(all, x, y);

        *pp = all->board.color.ball;
    }
}

static void update_ball_moving(struct ALL* all)
{
    all->board.ball.cx += all->board.ball.vx;
    all->board.ball.cy += all->board.ball.vy;
}

//-----------------------------------------------------------------------------

static int init_fb(struct ALL* all)
{
    int ret = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;

    all->screen.fd = -1;

    int fd = open("/dev/fb0", O_RDWR);
    if (fd == -1) {
        perror("Error: cannot open framebuffer device");
        return 1;
    }

    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
        close(fd);
        perror("Error reading fixed information");
        return 2;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        close(fd);
        perror("Error reading variable information");
        return 3;
    }
    if(BYTES_PER_PIXEL * 8 != vinfo.bits_per_pixel) {
        close(fd);
        fprintf(stderr, "Unsupported bits per pixel: %u. Only %d bpp is supported\n", vinfo.bits_per_pixel, BYTES_PER_PIXEL * 8);
        return 4;
    }
    all->screen.xoffset = vinfo.xoffset;
    all->screen.yoffset = vinfo.yoffset;
    all->screen.line_length = finfo.line_length;

    printf("%dx%d\n", vinfo.xres, vinfo.yres);

    all->screen.size = vinfo.xres * vinfo.yres * BYTES_PER_PIXEL;

    all->screen.fbp = (unsigned char *)mmap(0, all->screen.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


    if ((int)all->screen.fbp == -1) {
        close(fd);
        perror("Error: failed to map framebuffer device to memory");
        return 5;
    }

    all->screen.fd = fd;
    return ret;
}

static void free_fb(struct ALL* all)
{
    close(all->screen.fd);
    munmap(all->screen.fbp, all->screen.size);
}

//-----------------------------------------------------------------------------

static void init_ball(struct ALL* all)
{
    all->board.ball.rad = all->board.gs;
    all->board.ball.cx = BOARD_WIDTH / 2;
    all->board.ball.cy = BOARD_HEIGHT / 2;
    all->board.ball.vx = -2;
    all->board.ball.vy = -3;
}

static void init_color(struct ALL* all)
{
    all->board.color.bground	= 0x00000000;
    all->board.color.border	= 0x00FF8888;
    all->board.color.ball		= 0x00FF0000;
}

static void init_wall(struct ALL* all)
{
    srand(time(0));
    all->board.wall.x0 = 1 + rand() % (20 * all->board.gs - 2);
    all->board.wall.x1 = 1 + rand() % (20 * all->board.gs - 2);
    all->board.wall.x2 = 1 + rand() % (20 * all->board.gs - 2);
    all->board.wall.x3 = 1 + rand() % (20 * all->board.gs - 2);

    all->board.wall.length = 10 * all->board.gs;
}

static void init_board(struct ALL* all)
{
    all->board.gs		= 10;

    init_ball(all);
    init_color(all);
    init_wall(all);
}

static void update_board(struct ALL* all)
{
    update_ball_moving(all);
}

static void scan_write_fb(struct ALL* all)
{
    int x, y;
    uint32_t* pp = (uint32_t*)all->screen.fbp;
    pp += (BOARD_LEFT + all->screen.xoffset);
    pp += (BOARD_TOP + all->screen.yoffset) * all->screen.line_length / BYTES_PER_PIXEL;

    for(y = 0; y < BOARD_HEIGHT; y++, pp += (all->screen.line_length / BYTES_PER_PIXEL - BOARD_WIDTH)) {
        for(x = 0; x < BOARD_WIDTH; x++, pp ++) {
            *pp = all->board.color.bground;
            draw_borders(all, pp, x, y);
            draw_walls(all, pp, x, y);
            draw_ball(all, pp, x, y);
        }
    }
}

static void ctrl_c(int sig) {
    signal(SIGINT, ctrl_c);
    running = 0;
    printf("CTRL+C %d\n", running);
}

int main()
{
    struct ALL all;
    int ret = 0;

    running = 1;
    signal(SIGINT, ctrl_c);

    if((ret = init_fb(&all)))
        return ret;

    init_board(&all);

    while(running) {
        uint32_t count = 300;
        while(running && count --) {
            update_board(&all);
            ioctl(all.screen.fd, FBIO_WAITFORVSYNC, 0);
            scan_write_fb(&all);
        }
        init_wall(&all);
    }

    free_fb(&all);
    return ret;
}

