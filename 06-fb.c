#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>


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


int main()
{
	int ret = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    unsigned char *fbp = 0;
    int x = 0, y = 0;
    long int location = 0;

    int fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        return 1;
    }

	do {
		if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
			perror("Error reading fixed information");
			ret = 2;
			break;
		}

		if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
			perror("Error reading variable information");
			ret = 3;
			break;
		}

		printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

		screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

		fbp = (unsigned char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	} while(0);

    close(fbfd);
	
    if ((int)fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        return 4;
    }

    printf("The framebuffer device was mapped to memory successfully.\n");

	// Figure out where in memory to put the pixel
	for (y = 0; y < 1080; y++)
		for (x = 0; x < 1920; x++) {

			location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
				(y+vinfo.yoffset) * finfo.line_length;
			draw(fbp + location, x, y);

		}
	munmap(fbp, screensize);
	return ret;
}
