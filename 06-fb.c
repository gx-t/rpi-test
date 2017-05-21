#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

struct {
	struct {
		uint32_t xres;
		uint32_t yres;
		uint32_t bits_per_pixel;
		uint32_t xoffset;
		uint32_t yoffset;
		uint32_t line_length;
		uint32_t size;
		uint8_t* fbp;
	} screen;
}static g;

static void draw_oval(unsigned char* rgba, int x, int y, int cx, int cy, int r, int ax, int ay) {
	x -= cx;
	y -= cy;
	x *= ax;
	y *= ay;
	int xx = x * x + y * y;
	int rr = r * r;
	if(xx > rr - 100 && xx < rr + 100) {
		rgba[0] = 0x00;
		rgba[1] = 0x00;
		rgba[2] = 0xFF;
		rgba[3] = 0;
	}
}

static void draw_filled_oval(unsigned char* rgba, int x, int y, int cx, int cy, int r, int ax, int ay, int color) {
	x -= cx;
	y -= cy;
	x *= ax;
	y *= ay;
	int xx = x * x + y * y;
	int rr = r * r;
	if(xx < rr) {
		rgba[0] = (color & 0x000000FF) >> 0;
		rgba[1] = (color & 0x0000FF00) >> 8;
		rgba[2] = (color & 0x00FF0000) >> 16;
		rgba[3] = (color & 0x00000000) >> 0;
	}
}

static void draw(unsigned char* rgba, int x, int y) {
	draw_filled_oval(rgba, x, y, 500, 500, 70, 1, 2, 0x000000FF);
	draw_filled_oval(rgba, x, y, 500, 500, 30, 1, 1, 0x0000FF00);
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

	printf("The framebuffer device was mapped to memory successfully.\n");
	return ret;
}

static void free_fb()
{
	munmap(g.screen.fbp, g.screen.size);
}

static void draw_fb()
{
	uint32_t x = 0, y = 0;
	uint32_t offset = 0;
	for (y = 0; y < g.screen.yres; y++) {
		for (x = 0; x < g.screen.xres; x++) {

			offset = (x + g.screen.xoffset) * (g.screen.bits_per_pixel / 8) + (y + g.screen.yoffset) * g.screen.line_length;
			draw(g.screen.fbp + offset, x, y);

		}
	}
}

int main()
{
	int ret = 0;

	if((ret = init_fb()))
		return ret;

	draw_fb();

	free_fb();
	return ret;
}

