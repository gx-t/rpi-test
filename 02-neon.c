// http://gcc.gnu.org/onlinedocs/gcc-4.4.1/gcc/ARM-NEON-Intrinsics.html

#include <stdio.h>
#include <stdlib.h>
#include <arm_neon.h>
#include <unistd.h>

#define PI				3.14159265358979323846
#define SAMP_RATE		48000 //Fs = 48KHZ
#define BLOCK_SIZE		(SAMP_RATE / 10) //100 ms block
#define FREQ_F32(f_)	((f_) * 2 * PI / SAMP_RATE)

static float str_to_float(char* str_val, float min, float max) {
	float val = strtod(str_val, 0);
	if(val < min) {
		fprintf(stderr, "%s is out of range, setting minimum: %g\n", str_val, min);
		return min;
	}
	if(val > max) {
		fprintf(stderr, "%s is out of range, setting maximum: %g\n", str_val, max);
		return max;
	}
	return val;
}

int main(int argc, char *argv[]) {

	if(argc != 4) {
		fprintf(stderr, "Usage: %s Fc Fb T\n"
				"\tFc - Central frequency(100-1000 Hz)\n"
				"\tFb - Beat frequency(2-20 Hz)\n"
				"\tT  - Duration (1-60 Min)\n"
				"Example:\n"
				"\t./02-neon 170 5 10 | aplay -t raw -c 2 -f FLOAT_LE -r 48000\n"
				"\t./02-neon 159 5.7 3 | play -r 48k -t raw -e floating-point -b 32 -c 2 -V3 -\n" , argv[0]);
		return 1;
	}
	float fc = str_to_float(argv[1], 100, 1000);
	float fb = str_to_float(argv[2], 2, 20);
	float t = str_to_float(argv[3], 1, 60);
	
	register float32x2_t c = {1.0, 1.0};
	register float32x2_t s = {0.0, 0.0};
	register float32x2_t f = {FREQ_F32(fc - fb / 2), FREQ_F32(fc + fb / 2)};

	t *= 600;

	while(0 < t--) {

		float32x2_t buff[BLOCK_SIZE];
		register float32x2_t *p = buff;
		register float count = BLOCK_SIZE;

		while(0 < count--) {

			c = vmls_f32(c, s, f);
			s = vmla_f32(s, c, f);

			*p ++ = s;
		}
		write(STDOUT_FILENO, buff, sizeof(buff));
	}
	return 0;
}

