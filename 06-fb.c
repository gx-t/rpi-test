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

		uint8_t* fbp;
	} screen;

	struct {
		uint32_t left
			, top
			, width
			, height;

		struct {
			uint32_t width
				, height;
			int32_t x;
		} pad;
	
		struct {
			uint32_t rad
				, cx
				, cy;

			int32_t vx
				, vy;

			void (*update_pos)();
		} ball;
	
		struct {
			uint32_t bground
				, border
				, pad
				, ball;
		} color;
	} board;

	struct {
		uint32_t x
			, y;

		uint8_t* pp;
	} scan;

	struct {
		int fd;
		int8_t dx;
	}mouse;

	int32_t running;
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

static void draw_pad()
{
	int32_t x = (int32_t)g.scan.x
		, y = g.scan.y;

	if(y < g.board.height - g.board.pad.height)
		return;
	if(y > g.board.height - 2)
		return;
	
	x -= g.board.pad.x;

	if(0 > x)
		return;
	if(g.board.pad.width < x)
		return;

	*(uint32_t*)g.scan.pp = g.board.color.pad;
}

static void draw_ball()
{
	int32_t cx = (int32_t)g.scan.x - (int32_t)g.board.ball.cx
		, cy = (int32_t)g.scan.y - (int32_t)g.board.ball.cy
		, r = g.board.ball.rad;

//	if(*(uint32_t*)g.scan.pp != g.color.bground) {
//	}
	if(cx * cx + cy * cy < r * r) {
		*(uint32_t*)g.scan.pp = g.board.color.ball;
	}
}

static void update_ball_kept()
{
	uint32_t cx = g.board.pad.x + g.board.pad.width / 2
		, cy = g.board.height - g.board.pad.height - g.board.ball.rad;
	
	g.board.ball.cx = cx;
	g.board.ball.cy = cy;
}

static void update_ball_moving()
{
//	int32_t x = g.board.ball.cx;
//	int32_t y = g.board.ball.cy;
//	uint32_t r = g.board.ball.rad;

//	x += g.board.ball.vx;
//	y += g.board.ball.vy;

//	if(x - r < 2 && x + r > g.board.width - 2)
//		g.board.ball.vx = -g.board.ball.vx;
//
//	if(y - r < 2 && y + r > g.board.height - 2)
//		g.board.ball.vy = -g.board.ball.vy;

//	if(x - (int32_t)r < 1)
//		g.board.ball.vx = -g.board.ball.vx;

	g.board.ball.cx += g.board.ball.vx;
	g.board.ball.cy += g.board.ball.vy;
}

static void draw() {
	*(uint32_t*)g.scan.pp = g.board.color.bground;
	draw_borders();
	draw_pad();
	draw_ball();
}

//-----------------------------------------------------------------------------

static int init_fb()
{
	int ret = 0;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;

	int fd = open("/dev/fb0", O_RDWR);
	if (fd == -1) {
		perror("Error: cannot open framebuffer device");
		return 1;
	}

	do {
		if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
			perror("Error reading fixed information");
			ret = 2;
			break;
		}

		if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
			perror("Error reading variable information");
			ret = 3;
			break;
		}
		g.screen.xres = vinfo.xres;
		g.screen.yres = vinfo.yres;
		g.screen.bits_per_pixel = vinfo.bits_per_pixel;
		g.screen.line_length = finfo.line_length;

		printf("%dx%d, %dbpp\n", g.screen.xres, g.screen.yres, g.screen.bits_per_pixel);

		g.screen.size = g.screen.xres * g.screen.yres * g.screen.bits_per_pixel / 8;

		g.screen.fbp = (unsigned char *)mmap(0, g.screen.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	} while(0);

	close(fd);

	if ((int)g.screen.fbp == -1) {
		perror("Error: failed to map framebuffer device to memory");
		return 4;
	}
	
	return ret;
}

static void free_fb()
{
	munmap(g.screen.fbp, g.screen.size);
}

//-----------------------------------------------------------------------------

static void init_board()
{
	g.board.left = 300;
	g.board.top = 300;
	g.board.width = 300;
	g.board.height = 200;

	g.board.pad.width = g.board.width / 3;
	g.board.pad.height = g.board.height / 10;
	g.board.pad.x = g.board.width / 2 - g.board.pad.width;

	g.board.ball.rad = 10;
	g.board.ball.cx = g.board.width / 2;
	g.board.ball.cy = g.board.height / 2;
	g.board.ball.vx = 0;
	g.board.ball.vy = 0;
	g.board.ball.update_pos = update_ball_kept;

	g.board.color.bground = 0x00000000;
	g.board.color.border = 0x00FF8888;
	g.board.color.pad = 0x00FFFF00;
	g.board.color.ball = 0x00FF0000;

	srand(time(0));
	g.board.pad.x = 0;// 2 + rand() % (g.board.width - g.board.pad.width - 4);
}

static void update_board()
{
	g.board.pad.x += g.mouse.dx;
	g.mouse.dx = 0;

	g.board.ball.update_pos();
}

static int init_mouse()
{
	g.mouse.dx = 0;
	g.mouse.fd = open("/dev/input/mouse0", O_RDONLY | O_NONBLOCK);
	if(g.mouse.fd < 0) {
		perror("mouse");
		return 5;
	}
	return 0;
}

static void free_mouse()
{
	close(g.mouse.fd);
}

static void read_mouse()
{
	struct {
		uint8_t btn;
		int8_t dx;
		int8_t dy;
	} evt;
	if(sizeof(evt) != read(g.mouse.fd, &evt, sizeof(evt)))
		return;
	g.mouse.dx = evt.dx;
	if(evt.btn & 1) {
		g.board.ball.vx = -2;
		g.board.ball.vy = -3;
		g.board.ball.update_pos = update_ball_moving;
	}
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
}

int main()
{
	int ret = 0;

	g.running = 1;
	signal(SIGINT, ctrl_c);

	if((ret = init_fb()))
		return ret;
	if((ret = init_mouse())) {
		free_fb();
		return ret;
	}
	
	init_board();

	while(g.running) {
		read_mouse();
		update_board();
		scan_write_fb();
	}

	free_mouse();
	free_fb();
	return ret;
}

