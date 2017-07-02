// ./09-noise | ./10-resonance | play -r 48k -t f32 -c 1 -

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
	float s[3] = {0, 0, 0}
	, c[3] = {0, 0, 0}
	, f[3] = {FREQ_F32(760.0), FREQ_F32(1320.0), FREQ_F32(2500.0)}
	, buff[BLOCK_SIZE]
	, q = 300.0;

	signal(SIGINT, ctrl_c);
	
	while(running) {
		int num_bytes = read(0, buff, sizeof(buff));
		if(0 >= num_bytes || num_bytes % sizeof(float))
			break;

		int i = num_bytes / sizeof(float);
		float *pp = buff;

		while(i--) {

			s[0] += (*pp - s[0]) / q;
			c[0] += s[0] * f[0];
			s[0] -= c[0] * f[0];

			s[1] += (*pp - s[1]) / q;
			c[1] += s[1] * f[1];
			s[1] -= c[1] * f[1];

			s[2] += (*pp - s[2]) / q;
			c[2] += s[2] * f[2];
			s[2] -= c[2] * f[2];

			*pp ++ = s[0] + s[1] + s[2];
		}
		
		if(num_bytes != write(1, buff, num_bytes))
			break;
	}
	return 0;
}

