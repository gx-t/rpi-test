#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define POP_COUNT		1024
#define MIN_VAL			-1000.0
#define MAX_VAL			1000.0

struct INHABIT {
	float val;
	float rfitness;
} pop[POP_COUNT];

static void fill()
{
	struct INHABIT* pp = pop;
	int count = POP_COUNT;
	srand48(time(0));
	while(count --) {
		pp->val = (MAX_VAL - MIN_VAL) * drand48() + MIN_VAL;
		pp ++;
	}
}

static void crossover()
{
}

static void mutate()
{
}

static void fitness()
{
	static const float a = 2;
	static const float b = 6;
	static const float c = 3;

	struct INHABIT* pp = pop;
	int count = POP_COUNT;

	while(count --) {
		float x = pp->val;
		float res = a * x * x + b * x + c;
		pp->rfitness = (res * res);
		pp ++;
	}
}

static void sort()
{
}

static void print()
{
}

int main()
{
	int gen_count = 100;

	fill();

	while(gen_count --) {
		crossover();
		mutate();
		fitness();
		sort();
	}
	print();
	return 0;
}


