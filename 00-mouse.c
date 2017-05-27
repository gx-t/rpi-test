#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

static int fd = -1;

static void ctrl_c(int sig) {
	close(fd);
}

int main(int argc, char* argv[]) {
	struct {
		uint8_t btn;
		int8_t dx;
		int8_t dy;
	} evt;

	fd = open("/dev/input/mouse0", O_RDONLY);
	if(fd < 0) {
		perror("mouse");
		return 1;
	}

	signal(SIGINT, ctrl_c);

	while(sizeof(evt) == read(fd, &evt, sizeof(evt))) {
		printf("MOUSE: dx=%d, dy=%d, l=%d, m=%d, r=%d\n",
				evt.dx, evt.dy, !!(evt.btn & 1), !!(evt.btn & 4), !!(evt.btn & 2));
	}

	close(fd);

	fprintf(stderr, "exitting...\n");
	return 0;
}

