#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static void evol_quad_eq(float a, float b, float c, unsigned gen_count, unsigned pop_count, float min, float max, float mut)
{
	float pop[pop_count * 2];
	
	int select_proc(const void* x1, const void* x2)
	{
		float v1 = a * *(float*)x1 * *(float*)x1 + b * *(float*)x1 + c;
		float v2 = a * *(float*)x2 * *(float*)x2 + b * *(float*)x2 + c;
		v1 *= v1;
		v2 *= v2;
		if(v1 > v2)
			return 1;

		if(v2 > v1)
			return -1;

		return 0;
	}

	void fill()
	{
		float step = (max - min) / pop_count;
		float val = min;
		unsigned i;
		for(i = 0; i < pop_count; i ++, val += step) {
			pop[i] = val;
		}
	}

	void reproduce_mutate()
	{
		unsigned i;
		for(i = 0; i < pop_count; i ++) {
			pop[i + pop_count] = pop[i] - mut + mut * 2.0 * drand48();
		}
	}

	void print() {
		unsigned i;
		for(i = 0; i < pop_count; i ++) {
			printf("%g ", pop[i]);
		}
		printf("\n");
	}

	void sort_select()
	{
		qsort(pop, pop_count * 2, sizeof(pop[0]), select_proc);
	}


	fill();

	srand48(time(0));

	while(gen_count --) {
		reproduce_mutate();
		sort_select();
	}

	print();
}

static int cmd_evol_quad_eq(int argc, char* argv[]) {
	if(argc != 9) {
		fprintf(stderr, "Usage: %s a, b, c, <generation count> <poplation count> <min value> <max value> <mutation>\n", argv[0]);
		fprintf(stderr, "Example: 2 -4 -2 256 256 -100.0 100.0 1.0\n");
		return 3;
	}
	argc --;
	argv ++;

	float a, b, c, min, max, mut;
	unsigned gen_count, pop_count;
	if(1 != sscanf(argv[0], "%g", &a)
	|| 1 != sscanf(argv[1], "%g", &b)
	|| 1 != sscanf(argv[2], "%g", &c)
	|| 1 != sscanf(argv[3], "%u", &gen_count)
	|| 1 != sscanf(argv[4], "%u", &pop_count)
	|| 1 != sscanf(argv[5], "%g", &min)
	|| 1 != sscanf(argv[6], "%g", &max)
	|| 1 != sscanf(argv[7], "%g", &mut)) {
		fprintf(stderr, "Invalid value\n");
		return 4;
	}
	evol_quad_eq(a, b, c, gen_count, pop_count, min, max, mut);
	return 0;
}

int main(int argc, char* argv[])
{
	if(argc < 2) {
		fprintf(stderr, "Usage: %s <subcommand> <arguments>\n", argv[0]);
		fprintf(stderr, "Subcommands:\n");
		fprintf(stderr, "\tevol-quad-eq\n");
		return 1;
	}
	argc --;
	argv ++;
	if(!strcmp(*argv, "evol-quad-eq"))
		return cmd_evol_quad_eq(argc, argv);
	fprintf(stderr, "Unknown subcommand: %s\n", *argv);
	return 2;
}


