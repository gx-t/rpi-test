#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void evol_quad_eq(float a, float b, float c, unsigned gen_count, unsigned pop_count, float min, float max, float mut)
{
	int cmp_proc(const void* x1, const void* x2) {
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

	float pop[pop_count * 2];
	unsigned i;

	srand48(time(0));
	for(i = 0; i < pop_count; i ++) {
		pop[i] = min + (max - min) * drand48();
	}

	while(gen_count --) {
		for(i = 0; i < pop_count; i ++) {
			pop[i + pop_count] = pop[i] - mut + mut * 2.0 * drand48();
		}
		qsort(pop, pop_count * 2, sizeof(pop[0]), cmp_proc);
	}

	for(i = 0; i < pop_count; i ++) {
		printf("%g ", pop[i]);
	}
	printf("\n");
}

int main()
{
	evol_quad_eq(2, -4, -2, 256, 256, -100.0, 100.0, 1.0);
	return 0;
}


