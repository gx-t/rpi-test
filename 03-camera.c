// !VoCam264 USB 0c45:6366
// /dev/video0 - MJPEG
// /dev/video1 - H264
// https://gist.github.com/maxlapshin/1253534

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <signal.h>

#define BUFF_COUNT		4

struct {
	void* data;
	size_t len;
} static buff[BUFF_COUNT] = {{0}};

static int fd = 0;
static int running = 1;

static int set_fmt() {
	struct v4l2_format fmt = {
		.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.fmt.pix.width = 1920,
		.fmt.pix.height = 1080
	};

	if(ioctl(fd, VIDIOC_S_FMT, &fmt)) {
		perror("VIDIOC_S_FMT");
		return -1;
	}

	return 0;
}

static int req_buff() {
	struct v4l2_requestbuffers req = {
		.count = BUFF_COUNT,
		.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.memory = V4L2_MEMORY_MMAP
	};

	if(ioctl(fd, VIDIOC_REQBUFS, &req)) {
		perror("VIDIOC_REQBUFS");
		return -1;
	}

	return 0;
}

static int map_device() {
	uint32_t i;

	for(i = 0; i < BUFF_COUNT; i ++) {
		struct v4l2_buffer buf = {
			.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
			.memory = V4L2_MEMORY_MMAP,
			.index = i
		};

		if(ioctl(fd, VIDIOC_QUERYBUF, &buf)) {
			perror("VIDIOC_QUERYBUF");
			return -1;
		}

		buff[i].data = mmap(0, buf.length, PROT_READ, MAP_SHARED, fd, buf.m.offset);
		if(!buff[i].data) {
			perror("mmap");
			return -1;
		}
		buff[i].len = buf.length;
	}

	return 0;
}

static void unmap_device() {
	uint32_t i;

	for(i = 0; i < BUFF_COUNT; i ++) {
		if(!buff[i].len) {
			continue;
		}

		munmap(buff[i].data, buff[i].len);
	}
}

static int start_capture() {
	uint32_t i;

	for(i = 0; i < BUFF_COUNT; i ++) {
		struct v4l2_buffer buf = {
			.index = i,
			.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
			.memory = V4L2_MEMORY_MMAP
		};

		if(ioctl(fd, VIDIOC_QBUF, &buf)) {
			perror("VIDIOC_QBUF");
			return -1;
		}
	}

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(ioctl(fd, VIDIOC_STREAMON, &type)) {
		perror("VIDIOC_STREAMON");
		return -1;
	}
	return 0;
}

static int stop_capture() {
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(ioctl(fd, VIDIOC_STREAMOFF, &type)) {
		perror("VIDIOC_STREAMOFF");
		return -1;
	}
	return 0;
}

static int handle_block() {
	struct v4l2_buffer buf = {
		.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.memory = V4L2_MEMORY_MMAP
	};

	if(ioctl(fd, VIDIOC_DQBUF, &buf)) {
		perror("VIDIOC_DQBUF");
		return -1;
	}

	int res = buff[buf.index].len == write(STDOUT_FILENO, buff[buf.index].data, buff[buf.index].len);
	res = !res;

	if(ioctl(fd, VIDIOC_QBUF, &buf)) {
		perror("VIDIOC_QBUF");
		return -1;
	}
	return res;
}

static void ctrl_c(int sig) {
	signal(SIGINT, ctrl_c);
	running = 0;
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		fprintf(stderr, "Usage: %s /dev/video<x>\n", argv[0]);
		return 1;
	}
	signal(SIGINT, ctrl_c);
	fd = open(argv[1], O_RDWR);

	if(fd < 0) {
		perror(argv[1]);
		return 2;
	}
	if(set_fmt() || req_buff() || map_device()) {
		close(fd);
		return 3;
	}
	if(!start_capture()) {
		while(running && !handle_block());
		stop_capture();
	}
	unmap_device();
	close(fd);
	fprintf(stderr, "\n.END\n");
	return 0;
}

