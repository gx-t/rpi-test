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
	int8_t event[3];

	fd = open("/dev/input/mouse0", O_RDONLY);
	if(fd < 0) {
		perror("mouse");
		return 1;
	}

	signal(SIGINT, ctrl_c);

	while(sizeof(event) == read(fd, event, sizeof(event))) {
		printf("MOUSE: dx=%d, dy=%d, l=%d, m=%d, r=%d\n",
				event[1], event[2], !!(event[0] & 1), !!(event[0] & 4), !!(event[0] & 2));
	}

	close(fd);

	fprintf(stderr, "exitting...\n");
	return 0;
}

