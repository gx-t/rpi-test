// Usage:
// ./02-neon | aplay -t raw -c 2 -f FLOAT_LE -r 48000
// or
// ./02-neon | play -r 48k -t raw -e floating-point -b 32 -c 2 -V3 -
// http://gcc.gnu.org/onlinedocs/gcc-4.4.1/gcc/ARM-NEON-Intrinsics.html

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <arm_neon.h>

#define PI				3.14159265358979323846
#define SAMP_RATE		48000 //Fs = 48KHZ
#define BLOCK_SIZE		(SAMP_RATE / 10) //100 ms block
#define FREQ_F32(f_)	(f_ * 2 * PI / SAMP_RATE)


static int running = 1;

void ctrl_c(int sig) {
	signal(SIGINT, ctrl_c);
	running = 0;
}

static void generate_block() {
	
	static float32x2_t c = {1.0, 1.0};
	static float32x2_t s = {0.0, 0.0};
	static float32x2_t f = {FREQ_F32(150), FREQ_F32(155)};

	float32x2_t buff[BLOCK_SIZE];
	register float32x2_t *p = buff, *end = buff + BLOCK_SIZE;

	while(p < end) {

		c = vmls_f32(c, s, f);
		s = vmla_f32(s, c, f);

		*p ++ = s;
	}
	write(STDOUT_FILENO, buff, sizeof(buff));
}

int main() {
	
	signal(SIGINT, ctrl_c);

	while(running) {
		generate_block();
	}
	return 0;
}

