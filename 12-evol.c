#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MUTATION_DECREASE_FACTOR    1.05

/*
time ./12-evol evol-quad-eq 2 -4 -2 100000 256 -100.0 100.0 1.0
real    0m20.554s
user    0m20.539s
sys 0m0.012s

with adaptive mutation
real    0m16.885s
user    0m16.830s
sys 0m0.031s

many args instead of struct pointer
real    0m16.662s
user    0m16.649s
sys 0m0.011s
*/

// Inherit from float - fitness first
struct XF {
	float fit, x;
};

static float qe_calc_fitness(float a, float b, float c, float x)
{
    float v = a * x * x + b * x + c;
    return v * v;
}

static int qe_fitness_sort_proc(const float** f1, const float** f2)
{
    if(**f1 > **f2)
        return 1;

    if(**f1 < **f2)
        return -1;

    return 0;
}

static void qe_calc_and_fill_fitness(struct XF* xf, struct XF** idx, float a, float b, float c, unsigned count, float min, float max)
{
    float step = (max - min) / count;
    float val = min;
    unsigned i;

    for(i = 0; i < count; i ++, val += step) {
        idx[i] = &xf[i];
        xf[i].x = val;
        xf[i].fit = qe_calc_fitness(a, b, c, val);
    }

    for(i = count; i < 2 * count; i ++) {
        idx[i] = &xf[i];
    }
}

static void qe_reproduce_and_mutate_x(struct XF** idx, float a, float b, float c, unsigned count, float mut)
{
    unsigned i;
    for(i = 0; i < count; i ++) {
        float val = idx[i]->x - mut + mut * 2.0 * drand48();
        idx[i + count]->x = val;
        idx[i + count]->fit = qe_calc_fitness(a, b, c, val);
    }
}

static void qe_print_x_and_fitness(struct XF* const * idx, unsigned count)
{
    printf("x                    fitness\n");
    printf("----------------------------\n");
    while(count --) {
        printf("%-16g %11g\n", (*idx)->x, (*idx)->fit);
        idx ++;
    }
    printf("----------------------------\n");
}

static void qe_evol(float a, float b, float c, unsigned gen_count, unsigned pop_count, float min, float max, float mut)
{
	struct XF pop[pop_count * 2];
	struct XF* index[pop_count * 2];

    qe_calc_and_fill_fitness(pop, index, a, b, c, pop_count, min, max);

	srand48(time(0));

	while(gen_count --) {
        qe_reproduce_and_mutate_x(index, a, b, c, pop_count, mut);
		qsort(index, pop_count * 2, sizeof(index[0]), (int (*)(const void*, const void*))qe_fitness_sort_proc);
        mut /= MUTATION_DECREASE_FACTOR;
	}

	qe_print_x_and_fitness(index, pop_count);
    printf("Final mutation: %g\n", mut);
}

static void qe_evol_reverse(float x, unsigned gen_count, unsigned pop_count, float min, float max, float mut)
{
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

static int cmd_qe_evol(int argc, char* argv[]) {
	if(argc != 9) {
		fprintf(stderr, "Usage: %s a, b, c, <generation count> <poplation count> <min value> <max value> <mutation>\n", argv[0]);
		fprintf(stderr, "Example: 2 -4 -2 256 256 -100.0 100.0 1.0\n");
		return 3;
	}
	argc --;
	argv ++;

	float min, max, mut, a = 0, b = 0, c = 0;
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
	qe_evol(a, b, c, gen_count, pop_count, min, max, mut);
	return 0;
}

static int cmd_qe_evol_reverse(int argc, char* argv[]) {
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
	qe_evol_reverse(x, gen_count, pop_count, min, max, mut);
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
		return cmd_qe_evol(argc, argv);
	if(!strcmp(*argv, "evol-quad-eq-reverse"))
		return cmd_qe_evol_reverse(argc, argv);
	fprintf(stderr, "Unknown subcommand: %s\n", *argv);
	return 2;
}


