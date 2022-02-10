#include <stdio.h>
#include <stdint.h>
#include <time.h>
#ifdef __ARM_ARCH
#include <arm_neon.h>
#endif // __ARM_ARCH

#define PI              
#define FREQ_F32(f_)	((f_) * 2 * 3.14159265358979323846 / 96000)

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
    const float f = FREQ_F32(1000.0);
    uint32_t i = 100000000;
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
    const double f = FREQ_F32(1000.0);
    uint32_t i = 100000000;
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

#ifdef __ARM_ARCH
static void bench_neon_32x4()
{
    volatile float32x4_t d1, d2;
    float32x4_t c = {1, 1, 1, 1}, s = {0, 0, 0, 0};
    const float32x4_t f = {FREQ_F32(1680.0), FREQ_F32(1986), FREQ_F32(2903), FREQ_F32(305.6)};
    uint32_t i = 99999999;
    while(i --)
    {
        c = vmlsq_f32(c, s, f);
        s = vmlaq_f32(s, c, f);
    }
    d1 = c;
    d2 = s;
}
#endif // __ARM_ARCH

int main()
{
    run_under_clock("float", bench_float);
    run_under_clock("double", bench_double);
    run_under_clock("uin64, recursion", bench_uint64_fibonacci);
#ifdef __ARM_ARCH
    run_under_clock("neon 32x4", bench_neon_32x4);
#endif // __ARM_ARCH
    return 0;
}

