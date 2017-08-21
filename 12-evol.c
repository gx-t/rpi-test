#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

struct XF {
	float x, val;
};

struct ABC {
	float a, b, c;
};

static void evol_quad_eq(struct ABC* abc, unsigned gen_count, unsigned pop_count, float min, float max, float mut)
{
	register float a = abc->a;
	register float b = abc->b;
	register float c = abc->c;
	struct XF pop[pop_count * 2];
	struct XF* index[pop_count * 2];

	void set_calc_val(struct XF* xf, float x)
	{
		float v = a * x * x + b * x + c;
		xf->val = v * v;
		xf->x = x;
	};
	
	int select_proc(const void* p1, const void* p2)
	{
		register const struct XF* xf1 = *(const struct XF**)p1;
		register const struct XF* xf2 = *(const struct XF**)p2;
		if(xf1->val > xf2->val)
			return 1;

		if(xf1->val < xf2->val)
			return -1;

		return 0;
	}

	void fill()
	{
		float step = (max - min) / pop_count;
		float val = min;
		unsigned i;
		for(i = 0; i < pop_count; i ++, val += step) {
			index[i] = &pop[i];
			set_calc_val(index[i], val);
		}
		for(i = pop_count; i < 2 * pop_count; i ++) {
			index[i] = &pop[i];
		}
	}

	void reproduce_mutate()
	{
		unsigned i;
		for(i = 0; i < pop_count; i ++)
			set_calc_val(index[i + pop_count], index[i]->x - mut + mut * 2.0 * drand48());
	}

	void print() {
		unsigned i;
		for(i = 0; i < pop_count; i ++) {
			printf("%g -- %g ", index[i]->x, index[i]->val);
		}
		printf("\n----------------------------\n");
	}

	void sort_select()
	{
		qsort(index, pop_count * 2, sizeof(index[0]), select_proc);
	}


	fill();

	srand48(time(0));

	struct timespec t1, t2;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
	
	while(gen_count --) {
		reproduce_mutate();
		sort_select();
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
	print();
	fprintf(stderr, "CPU TIME: %g us\n", ((double)t2.tv_nsec - (double)t1.tv_nsec) / 1000);
}

static void evol_quad_eq_reverse(float x, unsigned gen_count, unsigned pop_count, float min, float max, float mut)
{
	struct ABC pop[pop_count * 2];
/*	
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
		printf("\n----------------------------\n");
	}

	void sort_select()
	{
		qsort(pop, pop_count * 2, sizeof(pop[0]), select_proc);
	}


	fill();

	srand48(time(0));

	struct timespec t1, t2;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
	
	while(gen_count --) {
		reproduce_mutate();
		sort_select();
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
	print();
	fprintf(stderr, "CPU TIME: %g us\n", ((double)t2.tv_nsec - (double)t1.tv_nsec) / 1000);
	*/
}

static int cmd_evol_quad_eq(int argc, char* argv[]) {
	if(argc != 9) {
		fprintf(stderr, "Usage: %s a, b, c, <generation count> <poplation count> <min value> <max value> <mutation>\n", argv[0]);
		fprintf(stderr, "Example: 2 -4 -2 256 256 -100.0 100.0 1.0\n");
		return 3;
	}
	argc --;
	argv ++;

	float min, max, mut;
	struct ABC abc = {0};
	unsigned gen_count, pop_count;
	if(1 != sscanf(argv[0], "%g", &abc.a)
	|| 1 != sscanf(argv[1], "%g", &abc.b)
	|| 1 != sscanf(argv[2], "%g", &abc.c)
	|| 1 != sscanf(argv[3], "%u", &gen_count)
	|| 1 != sscanf(argv[4], "%u", &pop_count)
	|| 1 != sscanf(argv[5], "%g", &min)
	|| 1 != sscanf(argv[6], "%g", &max)
	|| 1 != sscanf(argv[7], "%g", &mut)) {
		fprintf(stderr, "Invalid value\n");
		return 4;
	}
	evol_quad_eq(&abc, gen_count, pop_count, min, max, mut);
	return 0;
}

static int cmd_evol_quad_eq_reverse(int argc, char* argv[]) {
	if(argc != 7) {
		fprintf(stderr, "Usage: %s x, <generation count> <poplation count> <min value> <max value> <mutation>\n", argv[0]);
		fprintf(stderr, "Example: 2 -4 -2 256 256 -100.0 100.0 1.0\n");
		return 3;
	}
	argc --;
	argv ++;

	float x, min, max, mut;
	unsigned gen_count, pop_count;
	if(1 != sscanf(argv[0], "%g", &x)
	|| 1 != sscanf(argv[1], "%u", &gen_count)
	|| 1 != sscanf(argv[2], "%u", &pop_count)
	|| 1 != sscanf(argv[3], "%g", &min)
	|| 1 != sscanf(argv[4], "%g", &max)
	|| 1 != sscanf(argv[5], "%g", &mut)) {
		fprintf(stderr, "Invalid value\n");
		return 4;
	}
	evol_quad_eq_reverse(x, gen_count, pop_count, min, max, mut);
	return 0;
}

int main(int argc, char* argv[])
{
	if(argc < 2) {
		fprintf(stderr, "Usage: %s <subcommand> <arguments>\n", argv[0]);
		fprintf(stderr, "Subcommands:\n");
		fprintf(stderr, "\tevol-quad-eq\n");
		fprintf(stderr, "\tevol-quad-reverse\n");
		return 1;
	}
	argc --;
	argv ++;
	if(!strcmp(*argv, "evol-quad-eq"))
		return cmd_evol_quad_eq(argc, argv);
	if(!strcmp(*argv, "evol-quad-eq-reverse"))
		return cmd_evol_quad_eq_reverse(argc, argv);
	fprintf(stderr, "Unknown subcommand: %s\n", *argv);
	return 2;
}


