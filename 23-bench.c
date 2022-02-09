#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define PI              3.14159265358979323846

static int run_under_clock(const char* label, void (*proc)())
{
    clock_t t1, t2;
    printf("%s...", label);
    t1 = clock();
    proc();
    t2 = clock();
    printf("%f sec\n", (double)(t2 - t1) / CLOCKS_PER_SEC);
    return 0;
}

static uint64_t fib_uint64(uint64_t n) {
  if (n <= 1) return n;
  return fib_uint64(n - 1) + fib_uint64(n - 2);
}

static void bench_float()
{
    volatile float d1, d2;
    float c, s;
    const float f = 1000.0 * 2 * PI / 96000;
    uint32_t i = 10000000;
    while(i --)
    {
        c += s * f;
        s -= c * f;
    }
    d1 = c;
    d2 = s;
}

static void bench_double()
{
    volatile double d1, d2;
    double c, s;
    const double f = 1000.0 * 2 * PI / 96000;
    uint32_t i = 10000000;
    while(i --)
    {
        c += s * f;
        s -= c * f;
    }
    d1 = c;
    d2 = s;
}

static void bench_uint64_fibonacci()
{
    fib_uint64(36);
}

int main()
{
    run_under_clock("Float", bench_float);
    run_under_clock("Double", bench_double);
    run_under_clock("uin64, recursion", bench_uint64_fibonacci);
    return 0;
}

