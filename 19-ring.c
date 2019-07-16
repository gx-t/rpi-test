#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#define PI				3.14159265358979323846
#define SAMP_RATE		96000 //Fs = 48KHZ
#define BLOCK_SIZE		(SAMP_RATE / 10) //100 ms block
#define FREQ_F32(f_)	((f_) * 2 * PI / SAMP_RATE)

static int running = 1;

static void ctrl_c(int sig)
{
	running = 0;
	signal(SIGINT, ctrl_c);
}

int main()
{
	float s0 = 0, c0 = 0.75, s1 = 0, c1 = 0.75, f0 = FREQ_F32(19900), f1 = FREQ_F32(20000), buff[BLOCK_SIZE];

	signal(SIGINT, ctrl_c);
	
	while(running && f0 > FREQ_F32(3100)) {
		int i = BLOCK_SIZE;
		float *pp = buff;

		while(i--) {
			c0 += s0 * f0;
			s0 -= c0 * f0;
			c1 += s1 * f1;
			s1 -= c1 * f1;
			*pp ++ = ((s0 + s1) / 2);
            f0 /= 1.00001;
            f1 /= 1.00001;
		}
		if(sizeof(buff) != write(1, buff, sizeof(buff)))
			break;
	}
	while(running && f0 < FREQ_F32(19900)) {
		int i = BLOCK_SIZE;
		float *pp = buff;

		while(i--) {
			c0 += s0 * f0;
			s0 -= c0 * f0;
			c1 += s1 * f1;
			s1 -= c1 * f1;
			*pp ++ = ((s0 + s1) / 2);
            f0 *= 1.00002;
            f1 *= 1.00002;
		}
		if(sizeof(buff) != write(1, buff, sizeof(buff)))
			break;
	}
	return 0;
}

