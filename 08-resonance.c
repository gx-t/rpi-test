// ./07-chirp | ./08-resonance | play -r 48k -t f32 -c 1 -q -

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
	float s = 0, c = 0, f = FREQ_F32(1000.0), buff[BLOCK_SIZE], q = 100.0;

	signal(SIGINT, ctrl_c);
	
	while(running) {
		if(sizeof(buff) != read(0, buff, sizeof(buff))) {
			perror("write");
			return 1;
		}
		int i = BLOCK_SIZE;
		float amp = 0;
		float *pp = buff;

		while(i--) {
			s += (*pp - s) / q;
			c += s * f;
			s -= c * f;
			*pp ++ = s;
			amp += s * s + c * c;
		}
		fprintf(stderr, "\tAMP=%g\r", sqrt(amp / BLOCK_SIZE));
		
		
		if(sizeof(buff) != write(1, buff, sizeof(buff))) {
			perror("write");
			return 2;
		}
	}
	return 0;
}

