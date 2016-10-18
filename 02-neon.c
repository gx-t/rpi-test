// ./02-neon | aplay -t raw -c 2 -f FLOAT_LE -r 48000
// http://gcc.gnu.org/onlinedocs/gcc-4.4.1/gcc/ARM-NEON-Intrinsics.html

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <arm_neon.h>

#define PI		3.14159265358979323846

#define FREQ_F32(freq_) (freq_ * 2 * PI / 48000)

static int running = 1;

#if 0
int main() {
	register float32x4_t p0 asm("q8") = {1.15, 1.16, 1.17, 1.21};
	register float32x4_t p1 asm("q9") = {1.15, 1.16, 1.17, 1.21};
	register float32x4_t p2 asm("q10");
	asm("vadd.f32 q10, q9, q8");
	asm("vadd.f32 q10, q9, q8");
	asm("vadd.f32 q10, q9, q8");
	asm("vadd.f32 q10, q9, q8");
	asm("vadd.f32 q10, q9, q8");
//	float32_t p3 = vaddvq_f32(p0);



	return 0;
}
#endif

void ctrl_c(int sig) {
	running = 0;
}

int main() {
	signal(SIGINT, ctrl_c);
    float c0 = 1.0, s0 = 0.0, f0 = FREQ_F32(150);
    float c1 = 1.0, s1 = 0.0, f1 = FREQ_F32(157);
    float buff[4800 * 2];
    float *p, *end = &buff[4800 * 2];

    while(running) {
        p = buff;

        while(p < end) {

            c0 -= s0 * f0;
            s0 += c0 * f0;

            c1 -= s1 * f1;
            s1 += c1 * f1;

            *p ++ = s0;
            *p ++ = s1;

        }
		write(1, buff, sizeof(buff));
    }
    return 0;
}


