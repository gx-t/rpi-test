#include <stdio.h>
#include <unistd.h>
#include <arm_neon.h>

#define PI				3.14159265358979323846
#define SAMP_RATE		48000 //Fs = 48KHZ
#define BLOCK_SIZE		(SAMP_RATE / 10) //100 ms block
#define FREQ_F32(f_)	((f_) * 2 * PI / SAMP_RATE)

int main() {
    int bell_count = 3;
	while(bell_count --) {

		float32x4_t c = {0.33, 0.5, 0.33, 1}
		, s = {0, 0, 0, 0}
		, f = {FREQ_F32(1680), FREQ_F32(1986), FREQ_F32(2903), FREQ_F32(305.6)}
		, a = {0.99998, 0.999985, 0.99996, 0.99997};

		int j = 40;
		while(j--) {
			float i = BLOCK_SIZE;
			float32x2_t buff[BLOCK_SIZE];
			float32x2_t *pp = buff;

			while(i--) {

				c = vmlsq_f32(c, s, f);
				s = vmlaq_f32(s, c, f);
				c = vmulq_f32(c, a);
				s = vmulq_f32(s, a);

				(*pp)[0] = (*pp)[1] = (s[0] + s[1] + s[2] + s[3])/4;
				pp ++;
			}
			if(sizeof(buff) != write(1, buff, sizeof(buff))) {
				perror("write");
				return 1;
			}
		}
	}
    return 0;
}

