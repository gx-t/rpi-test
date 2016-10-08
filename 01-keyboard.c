#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>

//see evtest /dev/input/event0

static int fd = -1;

static void ctrl_c(int sig) {
	close(fd);
}

int main(int argc, char* argv[]) {
	struct input_event event;

	fd = open("/dev/input/event0", O_RDONLY);
	if(fd < 0) {
		perror("event0");
		return 1;
	}

	signal(SIGINT, ctrl_c);

	while(sizeof(event) == read(fd, &event, sizeof(event))) {
		printf("KEYBOARD:\t{t=%u, c=%u, v=%d}\n",
		event.type, event.code, event.value);
	}

	close(fd);
	
	fprintf(stderr, "exitting...\n");
	return 0;
}

