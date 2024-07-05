#include <stdio.h>
#include <stdint.h>
#include <time.h>
#ifdef __ARM_ARCH
#include <arm_neon.h>
#endif // __ARM_ARCH

#define PI              
#define FREQ_F32(f_)	((f_) * 2 * 3.14159265358979323846 / 96000)

#define BILLION         (1000000000)

//static int run_under_clock_in_loop(const char* label, void (*proc)(), uint32_t run_count)
//{
//    double min_time = 9e9;
//    double max_time = 0;
//    double avg_time = 0;
//    uint32_t i;
//    printf("%s %d times (min/max/avg)...", label, run_count);
//    fflush(stdout);
//    for(i = 0; i < run_count; i ++)
//    {
//        clock_t t1, t2;
//        double curr_time;
//        t1 = clock();
//        proc();
//        t2 = clock();
//        curr_time = (double)(t2 - t1) / CLOCKS_PER_SEC;
//        avg_time += curr_time;
//        if(curr_time < min_time)
//            min_time = curr_time;
//        if(curr_time > max_time)
//            max_time = curr_time;
//    }
//    avg_time /= run_count;
//    printf("%f/%f/%f sec\n", min_time, max_time, avg_time);
//    return 0;
//}

static int run_under_clock(const char* label, void (*proc)())
{
    clock_t t1, t2;
    printf("%s...", label);
    fflush(stdout);
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
    float c = 1.0, s = 0.0;
    const float f = FREQ_F32(1000.0);
    uint32_t i = BILLION;
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
    double c = 1.0, s = 0.0;
    const double f = FREQ_F32(1000.0);
    uint32_t i = BILLION;
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
    volatile uint64_t d;
    d = fib_uint64(47);
}

#ifdef __ARM_ARCH
static void bench_neon_32x4()
{
    volatile float32x4_t d1, d2;
    float32x4_t c = {1, 1, 1, 1}, s = {0, 0, 0, 0};
    const float32x4_t f = {FREQ_F32(1680.0), FREQ_F32(1986), FREQ_F32(2903), FREQ_F32(305.6)};
    uint32_t i = BILLION;
    while(i --)
    {
        c = vmlsq_f32(c, s, f);
        s = vmlaq_f32(s, c, f);
    }
    d1 = c;
    d2 = s;
}

static void bench_float_32x4()
{
    volatile float32x4_t d1, d2;
    float32x4_t c = {1, 1, 1, 1}, s = {0, 0, 0, 0};
    const float32x4_t f = {FREQ_F32(1680.0), FREQ_F32(1986), FREQ_F32(2903), FREQ_F32(305.6)};
    uint32_t i = BILLION;
    while(i --)
    {
        c += s * f;
        s -= c * f;
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
    run_under_clock("float 32x4", bench_float_32x4);
#endif // __ARM_ARCH
    return 0;
}

