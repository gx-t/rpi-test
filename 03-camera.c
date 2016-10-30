// !VoCam264 USB 0c45:6366
// /dev/video0 - MJPEG
// /dev/video1 - H264
// https://gist.github.com/maxlapshin/1253534

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>

#define BUFF_COUNT		4

struct {
	void* data;
	size_t len;
} static buff[BUFF_COUNT] = {{0}};

static int fd = 0;
static int running = 1;
static int ss = -1, cc = 1;

static const char* dev_name = "/dev/video0";

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

static const char* curr_time() {
	time_t tt = time(0);
	return asctime(localtime(&tt));
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

	int res = buff[buf.index].len == write(cc, buff[buf.index].data, buff[buf.index].len);
	res = !res;

	if(ioctl(fd, VIDIOC_QBUF, &buf)) {
		perror("VIDIOC_QBUF");
		return -1;
	}

	if(res && (-1 != cc)) {
		close(cc);
		cc = -1;
		fcntl(ss, F_SETFL, fcntl(ss, F_GETFL, 0) & ~O_NONBLOCK);
		fprintf(stderr, "%s\tDisconnected\n", curr_time());
	}
	return res;
}

static void ctrl_c(int sig) {
	signal(SIGINT, ctrl_c);

	running = 0;

	if(-1 != ss) {
		close(ss);
		ss = -1;
	}

	if(-1 != cc) {
		close(cc);
		cc = -1;
	}
}

static void set_conn_timeout() {
	struct timeval tt = {
		.tv_sec = 1,
		.tv_usec = 0
	};      
	setsockopt(cc, SOL_SOCKET, SO_SNDTIMEO, (char*)&tt, sizeof(tt));
}

static int update_conn() {
	struct sockaddr_in sin = {0};
	socklen_t addr_len = sizeof(sin);

	int new_conn = accept(ss, (struct sockaddr*)&sin, &addr_len);

	if(-1 == new_conn) {
		return 0;
	}

	fprintf(stderr, "%s\tUpdated to: %s:%d\n", curr_time(), inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

	set_conn_timeout();

	return cc != dup2(new_conn, cc);
}

static int run_camera() {
	fd = open(dev_name, O_RDWR);

	if(-1 == fd) {
		perror(dev_name);
		return 1;
	}

	if(set_fmt() || req_buff() || map_device()) {
		close(fd);
		return 2;
	}

	if(!start_capture()) {
		while(running && !handle_block() && !update_conn());
		stop_capture();
	}

	unmap_device();
	close(fd);

	return 0;
}

static int start_server() {
	ss = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == ss) {
		perror("socket");
		return 3;
	}
	struct sockaddr_in sin = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = INADDR_ANY,
		.sin_port = htons(8888)
	};
	int on = 1;
	setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));
	if(-1 == bind(ss, (struct sockaddr *) &sin, sizeof(sin))) {
		perror("bind");
		return 4;
	}
	if(-1 == listen(ss, 1)) {
		perror("listen");
		return 5;
	}
	return 0;
}

static int wait_client() {
	struct sockaddr_in sin = {0};
	socklen_t addr_len = sizeof(sin);
	cc = accept(ss, (struct sockaddr*)&sin, &addr_len);
	if(-1 == cc) {
		if(ss != -1) {
			perror("accept");
		}
		return 6;
	}
	fcntl(ss, F_SETFL, fcntl(ss, F_GETFL, 0) | O_NONBLOCK);
	set_conn_timeout();
	fprintf(stderr, "%s\tConnected: %s:%d\n", curr_time(), inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
	return 0;
}

static void stop_server() {
	if(-1 != ss) {
		close(ss);
	}
}

int main() {
	signal(SIGINT, ctrl_c);
	int res = start_server();
	if(res) {
		return res;
	}
	while(!(res = wait_client()) && !(res = run_camera()));
	stop_server();
	fprintf(stderr, "\n.END\n");
	return res;
}

