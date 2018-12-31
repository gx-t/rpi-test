#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MUTATION_DECREASE_FACTOR    1.05

// Inherit from double - fitness first
struct XF {
    double fit, x;
};

static double qe_calc_fitness(double a, double b, double c, double x)
{
    double v = a * x * x + b * x + c;
    return v * v;
}

static int qe_fitness_sort_proc(const double** f1, const double** f2)
{
    if(**f1 > **f2)
        return 1;

    if(**f1 < **f2)
        return -1;

    return 0;
}

static void qe_calc_and_fill_fitness(struct XF* xf, struct XF** idx, double a, double b, double c, unsigned count, double min, double max)
{
    double step = (max - min) / count;
    double val = min;

    while(count --) {
        *idx ++ = xf;
        xf->x = val;
        xf->fit = qe_calc_fitness(a, b, c, val);
        xf ++;
        val += step;
    }
}

static void qe_reproduce_and_mutate_x(struct XF** idx, double a, double b, double c, unsigned count, double mut)
{
    struct XF** idx1 = idx + count;

    while(count --) {
        double val = (*idx)->x - mut + mut * 2.0 * drand48();
        (*idx1)->x = val;
        (*idx1)->fit = qe_calc_fitness(a, b, c, val);
        idx ++;
        idx1 ++;
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
    printf("-------------------------------------------\n");
}

static void qe_evol(double a, double b, double c, unsigned gen_count, unsigned pop_count, double min, double max, double mut)
{
    struct XF pop[pop_count * 2];
    struct XF* index[pop_count * 2];

    qe_calc_and_fill_fitness(pop, index, a, b, c, pop_count * 2, min, max);

    srand48(time(0));

    while(gen_count --) {
        qe_reproduce_and_mutate_x(index, a, b, c, pop_count, mut);
        qsort(index, pop_count * 2, sizeof(index[0]), (int (*)(const void*, const void*))qe_fitness_sort_proc);
        mut /= MUTATION_DECREASE_FACTOR;
    }

    qe_print_x_and_fitness(index, pop_count);
    printf("Final mutation:\t\t\t%lf\nMutation decrease factor:\t%lf\n", mut, MUTATION_DECREASE_FACTOR);
}

static void qe_evol_reverse(double x, unsigned gen_count, unsigned pop_count, double min, double max, double mut)
{
    /*	
        int select_proc(const void* x1, const void* x2)
        {
        double v1 = a * *(double*)x1 * *(double*)x1 + b * *(double*)x1 + c;
        double v2 = a * *(double*)x2 * *(double*)x2 + b * *(double*)x2 + c;
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
        double step = (max - min) / pop_count;
        double val = min;
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
        printf("%lf ", pop[i]);
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
        fprintf(stderr, "CPU TIME: %lf us\n", ((double)t2.tv_nsec - (double)t1.tv_nsec) / 1000);
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

    double min, max, mut, a = 0, b = 0, c = 0;
    unsigned gen_count, pop_count;
    if(1 != sscanf(argv[0], "%lf", &a)
            || 1 != sscanf(argv[1], "%lf", &b)
            || 1 != sscanf(argv[2], "%lf", &c)
            || 1 != sscanf(argv[3], "%u", &gen_count)
            || 1 != sscanf(argv[4], "%u", &pop_count)
            || 1 != sscanf(argv[5], "%lf", &min)
            || 1 != sscanf(argv[6], "%lf", &max)
            || 1 != sscanf(argv[7], "%lf", &mut)) {
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

    double x, min, max, mut;
    unsigned gen_count, pop_count;
    if(1 != sscanf(argv[0], "%lf", &x)
            || 1 != sscanf(argv[1], "%u", &gen_count)
            || 1 != sscanf(argv[2], "%u", &pop_count)
            || 1 != sscanf(argv[3], "%lf", &min)
            || 1 != sscanf(argv[4], "%lf", &max)
            || 1 != sscanf(argv[5], "%lf", &mut)) {
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


