// ./07-chirp | ./08-resonance | play -r 48k -t f32 -c 1 -q -

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

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
	float s = 0, c = 0.75, f0 = FREQ_F32(155.0), f1 = FREQ_F32(7100.0), buff[BLOCK_SIZE];

	float f = f0;
	fprintf(stderr, "\nF0=%g\n", f * SAMP_RATE / 2 / PI);
	signal(SIGINT, ctrl_c);
	
	while(running && f < f1) {
		int i = BLOCK_SIZE;
		float *pp = buff;

		while(i--) {
			c += s * f;
			s -= c * f;
			*pp ++ = s;
			f *= 1.000001;
		}
		fprintf(stderr, "F=%g\r", f * SAMP_RATE / 2 / PI);
		if(sizeof(buff) != write(1, buff, sizeof(buff)))
			break;
	}
	fprintf(stderr, "\nF1=%g\n", f * SAMP_RATE / 2 / PI);
	return 0;
}

