#include <stdio.h>
#include <unistd.h>
#include <arm_neon.h>


int main() {
	while(1) {
		float32x4_t c = {1, 1, 1, 1}
		, s = {0, 0, 0, 0}
		, f = {0.22, 0.26, 0.38, 0.04}
		, a = {0.99998, 0.999985, 0.99996, 0.99997};
		int j = 40;
		while(j--) {
			int i = 4800;
			float buff[4800];
			float *pp = buff;
			while(i--) {
				c = vmlsq_f32(c, s, f);
				s = vmlaq_f32(s, c, f);
				c = vmulq_f32(c, a);
				s = vmulq_f32(s, a);

				*pp++ = (s[0] + s[1] + s[2] + s[3])/4;
			}
			write(1, buff, sizeof(buff));
		}
	}
    return 0;
}

