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

struct {
	struct {
		uint32_t xres
			, yres
			, bits_per_pixel
			, xoffset
			, yoffset
			, line_length
			, size;
		int32_t fd;
		uint8_t* fbp;
	} screen;

	struct {
		uint32_t gs
			, left
			, top
			, width
			, height;

		struct {
			uint32_t rad
				, cx
				, cy;

			int32_t vx
				, vy;

		} ball;

		struct {
			int32_t x0, x1, x2, x3;
			int32_t l0, l1, l2, l3;
			uint32_t c0, c1, c2, c3;
		} wall;

		struct {
			uint32_t bground
				, border
				, ball;
		} color;
	} board;

	struct {
		uint32_t x
			, y;

		uint8_t* pp;
	} scan;

	volatile int32_t running;
}static g;

static void draw_borders()
{
	uint32_t x = g.scan.x
		, y = g.scan.y
		, r = g.board.width - 1
		, b = g.board.height - 1;

	if(0 == x || r == x || 0 == y || b == y) {
		*(uint32_t*)g.scan.pp = g.board.color.border;
	}
}

static void process_ball_collision(int32_t cx, int32_t cy)
{
	uint32_t rr = g.board.ball.rad;
	rr *= rr;
	rr /= 3;
	if(cy * cy > rr) {
		g.board.ball.vy = -g.board.ball.vy;
		return;
	}
	if(cx * cx > rr) {
		g.board.ball.vx = -g.board.ball.vx;
		return;
	}
}

static void draw_line(int32_t x, int32_t y, int32_t len, uint32_t color)
{
	if(y != g.scan.y) 
		return;
	if(g.scan.x < x || g.scan.x > x + len)
		return;

	*(uint32_t*)g.scan.pp = color;
}

static void draw_walls()
{
	int32_t sy = 4 * g.board.gs;
	int32_t y = sy;
	draw_line(g.board.wall.x0, y, g.board.wall.l0, 0x00FF0000);
	y += sy;
	draw_line(g.board.wall.x1, y, g.board.wall.l1, 0x0000FF00);
	y += sy;
	draw_line(g.board.wall.x2, y, g.board.wall.l2, 0x000000FF);
	y += sy;
	draw_line(g.board.wall.x3, y, g.board.wall.l3, 0x0000FFFF);
}

static void draw_ball()
{
	int32_t cx = (int32_t)g.scan.x - (int32_t)g.board.ball.cx
		, cy = (int32_t)g.scan.y - (int32_t)g.board.ball.cy
		, r = g.board.ball.rad;

	if(cx * cx + cy * cy < r * r) {
		if(*(uint32_t*)g.scan.pp != g.board.color.bground)
			process_ball_collision(cx, cy);

		*(uint32_t*)g.scan.pp = g.board.color.ball;
	}
}

static void update_ball_moving()
{
	g.board.ball.cx += g.board.ball.vx;
	g.board.ball.cy += g.board.ball.vy;
}

static void draw() {
	*(uint32_t*)g.scan.pp = g.board.color.bground;
	draw_borders();
	draw_walls();
	draw_ball();
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
	g.screen.xres = vinfo.xres;
	g.screen.yres = vinfo.yres;
	g.screen.bits_per_pixel = vinfo.bits_per_pixel;
	g.screen.line_length = finfo.line_length;

	printf("%dx%d, %dbpp\n", g.screen.xres, g.screen.yres, g.screen.bits_per_pixel);

	g.screen.size = g.screen.xres * g.screen.yres * g.screen.bits_per_pixel / 8;

	g.screen.fbp = (unsigned char *)mmap(0, g.screen.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


	if ((int)g.screen.fbp == -1) {
		close(fd);
		perror("Error: failed to map framebuffer device to memory");
		return 4;
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
	g.board.ball.cx = g.board.width / 2;
	g.board.ball.cy = g.board.height / 2;
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
	g.board.wall.x0 = rand() % (20 * g.board.gs);
	g.board.wall.x1 = rand() % (20 * g.board.gs);
	g.board.wall.x2 = rand() % (20 * g.board.gs);
	g.board.wall.x3 = rand() % (20 * g.board.gs);
	
	g.board.wall.l0 = rand() % (20 * g.board.gs) + 5 * g.board.gs;
	g.board.wall.l1 = rand() % (20 * g.board.gs) + 5 * g.board.gs;
	g.board.wall.l2 = rand() % (20 * g.board.gs) + 5 * g.board.gs;
	g.board.wall.l3 = rand() % (20 * g.board.gs) + 5 * g.board.gs;
}

static void init_board()
{
	g.board.gs		= 10;
	g.board.left	= 30 * g.board.gs;
	g.board.top		= 30 * g.board.gs;
	g.board.width	= 30 * g.board.gs;
	g.board.height	= 20 * g.board.gs;

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
	uint32_t x, y, offset
		, l = g.board.left
		, t = g.board.top
		, r = l + g.board.width
		, b = t + g.board.height;

	for(y = t; y < b; y++) {
		for(x = l; x < r; x++) {
			g.scan.x = x - g.board.left;
			g.scan.y = y - g.board.top;
			offset = (x + g.screen.xoffset) * (g.screen.bits_per_pixel / 8) + (y + g.screen.yoffset) * g.screen.line_length;
			g.scan.pp = g.screen.fbp + offset;
			draw();
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

