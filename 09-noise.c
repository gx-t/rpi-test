// ./07-chirp | ./08-resonance | play -r 48k -t f32 -c 1 -q -

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define SAMP_RATE		48000 //Fs = 48KHZ
#define BLOCK_SIZE		(SAMP_RATE / 10) //100 ms block

static int running = 1;

static void ctrl_c(int sig)
{
	running = 0;
	signal(SIGINT, ctrl_c);
}

static void fill_buff(float buff[BLOCK_SIZE])
{
	float* pp = buff;
	int count = BLOCK_SIZE;
	while(running && count --)
		*pp ++ = (drand48() * 10.0 - 5.0);
}

int main()
{
	float buff[BLOCK_SIZE];

	signal(SIGINT, ctrl_c);

	while(running) {
		fill_buff(buff);
		if(sizeof(buff) != write(1, buff, sizeof(buff)))
			break;
	}

	return 0;
}

