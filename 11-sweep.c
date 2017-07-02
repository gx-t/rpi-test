// ./09-noise | ./11-sweep | play -r 48k -t f32 -c 1 -

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#define PI				3.14159265358979323846
#define SAMP_RATE		48000 //Fs = 48KHZ
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
	float s = 0
	, c = 0
	, f0 = FREQ_F32(300.0)
	, f1 = FREQ_F32(7100.0)
	, f = f0
	, buff[BLOCK_SIZE]
	, q = 33.3;

	signal(SIGINT, ctrl_c);

	while(running) {
		while(running && f < f1) {
			int num_bytes = read(0, buff, sizeof(buff));
			if(0 > num_bytes || num_bytes % sizeof(float))
				break;

			int i = num_bytes / sizeof(float);
			float *pp = buff;

			while(i--) {

				s += (*pp - s) / q;
				c += s * f;
				s -= c * f;

				*pp ++ = s;
				f *= 1.00001;
			}

			if(num_bytes != write(1, buff, num_bytes))
				break;
		}


		while(running && f > f0) {
			int num_bytes = read(0, buff, sizeof(buff));
			if(0 > num_bytes)
				break;

			int i = num_bytes / sizeof(float);
			float *pp = buff;

			while(i--) {

				s += (*pp - s) / q;
				c += s * f;
				s -= c * f;

				*pp ++ = s;
				f /= 1.00001;
			}

			if(num_bytes != write(1, buff, num_bytes))
				break;
		}
	}
	return 0;
}

