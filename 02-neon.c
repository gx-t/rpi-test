// ./02-neon | aplay -t raw -c 2 -f FLOAT_LE -r 48000
// http://gcc.gnu.org/onlinedocs/gcc-4.4.1/gcc/ARM-NEON-Intrinsics.html

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <arm_neon.h>

#define PI		3.14159265358979323846

#define FREQ_F32(freq_) (freq_ * 2 * PI / 48000)

static int running = 1;

void ctrl_c(int sig) {
	signal(SIGINT, ctrl_c);
	running = 0;
}

int main() {
	
	signal(SIGINT, ctrl_c);

	float32x2_t c = {1.0, 1.0};
	float32x2_t s = {0.0, 0.0};
	float32x2_t f = {FREQ_F32(150), FREQ_F32(155)};
	
	float32x2_t buff[4800];
	float32x2_t *p, *end = &buff[4800];

	while(running) {
		p = buff;

		while(p < end) {

			c = vmls_f32(c, s, f);
			s = vmla_f32(s, c, f);

			*p ++ = s;
		}
		write(1, buff, sizeof(buff));
	}
	return 0;
}

