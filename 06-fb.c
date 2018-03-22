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

struct {
	struct {
		uint32_t xres
			, yres
			, xoffset
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

	volatile int32_t running;
}static g;

#define BOARD_LEFT      300
#define BOARD_TOP       300
#define BOARD_WIDTH     300
#define BOARD_HEIGHT    200

static void draw_borders(uint32_t* pp, int x, int y)
{
	uint32_t r = BOARD_WIDTH - 1
		, b = BOARD_HEIGHT - 1;

	if(0 == x || r == x || 0 == y || b == y) {
		*pp = g.board.color.border;
	}
}

static void process_ball_collision(int32_t cx, int32_t cy)
{
    g.board.ball.vx = -g.board.ball.vx;
    g.board.ball.vy = -g.board.ball.vy;
	if(0 == cx) {
		g.board.ball.vx = -g.board.ball.vx;
	} else if(0 == cy) {
		g.board.ball.vy = -g.board.ball.vy;
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

static void draw_walls(uint32_t* pp, int x, int y)
{
	int32_t sy = 4 * g.board.gs;
	int32_t cy = sy;
	draw_line(pp, x, y, g.board.wall.x0, cy, g.board.wall.length, 0x00FF0000);
	cy += sy;
	draw_line(pp, x, y, g.board.wall.x1, cy, g.board.wall.length, 0x0000FF00);
	cy += sy;
	draw_line(pp, x, y, g.board.wall.x2, cy, g.board.wall.length, 0x000000FF);
	cy += sy;
	draw_line(pp, x, y, g.board.wall.x3, cy, g.board.wall.length, 0x0000FFFF);
}

static void draw_ball(uint32_t* pp, int x, int y)
{
    x -= (int)g.board.ball.cx;
    y -= (int)g.board.ball.cy;
    int r = (int)g.board.ball.rad;

	if(x * x + y * y < r * r) {
		if(*pp != g.board.color.bground)
			process_ball_collision(x, y);

		*pp = g.board.color.ball;
	}
}

static void update_ball_moving()
{
	g.board.ball.cx += g.board.ball.vx;
	g.board.ball.cy += g.board.ball.vy;
}

//-----------------------------------------------------------------------------

static int init_fb()
{
	int ret = 0;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;

	g.screen.fd = -1;

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
	g.screen.xres = vinfo.xres;
	g.screen.yres = vinfo.yres;
    g.screen.xoffset = vinfo.xoffset;
    g.screen.yoffset = vinfo.yoffset;
	g.screen.line_length = finfo.line_length;

	printf("%dx%d\n", g.screen.xres, g.screen.yres);

	g.screen.size = g.screen.xres * g.screen.yres * BYTES_PER_PIXEL;

	g.screen.fbp = (unsigned char *)mmap(0, g.screen.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


	if ((int)g.screen.fbp == -1) {
		close(fd);
		perror("Error: failed to map framebuffer device to memory");
		return 5;
	}

	g.screen.fd = fd;
	return ret;
}

static void free_fb()
{
	close(g.screen.fd);
	munmap(g.screen.fbp, g.screen.size);
}

//-----------------------------------------------------------------------------

static void init_ball()
{
	g.board.ball.rad = g.board.gs;
	g.board.ball.cx = BOARD_WIDTH / 2;
	g.board.ball.cy = BOARD_HEIGHT / 2;
	g.board.ball.vx = -2;
	g.board.ball.vy = -3;
}

static void init_color()
{
	g.board.color.bground	= 0x00000000;
	g.board.color.border	= 0x00FF8888;
	g.board.color.ball		= 0x00FF0000;
}

static void init_wall()
{
	srand(time(0));
	g.board.wall.x0 = 1 + rand() % (20 * g.board.gs - 2);
	g.board.wall.x1 = 1 + rand() % (20 * g.board.gs - 2);
	g.board.wall.x2 = 1 + rand() % (20 * g.board.gs - 2);
	g.board.wall.x3 = 1 + rand() % (20 * g.board.gs - 2);
	
	g.board.wall.length = 10 * g.board.gs;
}

static void init_board()
{
	g.board.gs		= 10;

	init_ball();
	init_color();
	init_wall();
}

static void update_board()
{
	update_ball_moving();
}

static void scan_write_fb()
{
    int x, y;
    uint32_t* pp = (uint32_t*)g.screen.fbp;
    pp += (BOARD_LEFT + g.screen.xoffset);
    pp += (BOARD_TOP + g.screen.yoffset) * g.screen.line_length / BYTES_PER_PIXEL;

	for(y = 0; y < BOARD_HEIGHT; y++, pp += (g.screen.line_length / BYTES_PER_PIXEL - BOARD_WIDTH)) {
		for(x = 0; x < BOARD_WIDTH; x++, pp ++) {
            *pp = g.board.color.bground;
            draw_borders(pp, x, y);
            draw_walls(pp, x, y);
            draw_ball(pp, x, y);
		}
	}
}

static void ctrl_c(int sig) {
	signal(SIGINT, ctrl_c);
	g.running = 0;
	printf("CTRL+C %d\n", g.running);
}

int main()
{
	int ret = 0;

	g.running = 1;
	signal(SIGINT, ctrl_c);

	if((ret = init_fb()))
		return ret;

	init_board();

	while(g.running) {
		uint32_t count = 300;
		while(g.running && count --) {
			update_board();
			scan_write_fb();
			ioctl(g.screen.fd, FBIO_WAITFORVSYNC, 0);
		}
		init_wall();
	}

	free_fb();
	return ret;
}

