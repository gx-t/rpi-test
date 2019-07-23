#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <arm_neon.h>

#define PI              3.14159265358979323846
#define SAMP_RATE       96000 //Fs = 96KHZ
#define BLOCK_SIZE      (SAMP_RATE / 10) //100 ms block
#define FREQ_F32(f_)    ((f_) * 2 * PI / SAMP_RATE)

static int running = 1;

static void ctrl_c(int sig)
{
    running = 0;
    signal(SIGINT, ctrl_c);
}

static void f0_tone()
{
	register float32x2_t c = {0.75, 0.75};
	register float32x2_t s = {0.0, 0.0};
	register float32x2_t f = {FREQ_F32(19900), FREQ_F32(20000)};
    register const float32x2_t k_down = {1.00001, 1.00001};
    register const float32x2_t k_up = {1.00002, 1.00002};
    float  buff[BLOCK_SIZE], *pp;
    int i;

    while(running && f[0] > FREQ_F32(3100)) {
        i = BLOCK_SIZE;
        pp = buff;

        while(i--) {
            c += s * f;
            s -= c * f;
            *pp ++ = ((s[0] + s[1]) / 2);
            f /= k_down;
        }
        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }

    while(running && f[0] < FREQ_F32(19900)) {
        i = BLOCK_SIZE;
        pp = buff;

        while(i--) {
            c += s * f;
            s -= c * f;
            *pp ++ = ((s[0] + s[1]) / 2);
            f *= k_up;
        }
        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }
}

static void f1_tone()
{
	register float32x4_t c = {0.75, 0.75, 0.0, 0.0};
	register float32x4_t s = {0.0, 0.0, 0.0, 0.0};
	register float32x4_t f = {FREQ_F32(2093.005), FREQ_F32(2637.020), FREQ_F32(1864.655), FREQ_F32(2349.318)};
	register float32x4_t d = {1.00004, 1.00004, 1.00004, 1.00004};
    float buff[BLOCK_SIZE], *pp;
    int i = 0, cnt = 5;

    while(cnt --) {
        i = BLOCK_SIZE;
        pp = buff;

        while(i --) {
            c += s * f;
            s -= c * f;

            *pp ++ = (s[0] + s[1] + s[2] + s[3]) / 4;
            s /= d;
        }
        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }

    s[3] = s[2] = 0.0;
    c[3] = c[2] = 0.75;

    cnt = 15;
    while(cnt --) {
        i = BLOCK_SIZE;
        pp = buff;

        while(i --) {
            c += s * f;
            s -= c * f;

            *pp ++ = (s[0] + s[1] + s[2] + s[3]) / 4;
            s /= d;
        }
        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }
}

static void f2_tone()
{
    float s0, c0, f0, a, buff[BLOCK_SIZE], *pp = buff;
    int i = 0, count = 0;

    s0 = 0, c0 = 0.75, f0 = FREQ_F32(7902.133); //B8

    for(count = 0; count < 30; count ++) {
        a = 1.0;
        i = BLOCK_SIZE;
        pp = buff;

        while(i --) {

            c0 += s0 * f0;
            s0 -= c0 * f0;

            *pp ++ = s0 * a;
            a /= 1.0003;
        }
        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }
}

static void f3_tone()
{
    float s0, c0, f0, a, buff[BLOCK_SIZE], *pp = buff;
    int i = 0, count = 0;

    s0 = 0, c0 = 0.75, f0 = FREQ_F32(3520.000);

    for(count = 0; count < 30; count ++) {
        a = 1.0;
        i = BLOCK_SIZE;
        pp = buff;

        while(i --) {

            c0 += s0 * f0;
            s0 -= c0 * f0;

            *pp ++ = s0 * a;
            a /= 1.0003;
        }
        f0 *= 1.04;
        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }
}

static void f4_tone()
{
	register float32x2_t c = {0.75, 0.75};
	register float32x2_t s = {0.0, 0.0};
	register float32x2_t f = {FREQ_F32(2900.0), FREQ_F32(10.0)};

    float buff[BLOCK_SIZE], *pp = buff;
    int i, count;

    for(count = 0; count < 10; count ++) {
        pp = buff;
        i = BLOCK_SIZE;
        while(i --) {
            c += s * f;
            s -= c * f;
            *pp ++ = s[0] * s[1];
        }

        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }
    for(count = 0; count < 10; count ++) {
        pp = buff;
        i = BLOCK_SIZE / 2;
        while(i --) {
            c += s * f;
            s -= c * f;
            *pp ++ = s[0] * s[1];
            s[1] /= 1.0004;
        }
        i = BLOCK_SIZE / 2;
        while(i --) {
            c += s * f;
            s -= c * f;
            *pp ++ = s[0] * s[1];
        }

        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }
}

int main(int argc, char* argv[])
{
    void (*f_arr[])() = {f0_tone, f1_tone, f2_tone, f3_tone, f4_tone};
    signal(SIGINT, ctrl_c);
    if(2 != argc) {
        fprintf(stderr, "Usage: %s [0-4]\n", *argv);
        return 1;
    }
    argv ++;
    int f_idx = **argv - '0';
    if(0 > f_idx || sizeof(f_arr) / sizeof(f_arr[0]) - 1 < f_idx) {
        fprintf(stderr, "Unknown tone: %c\n", **argv);
        return 2;
    }
    f_arr[f_idx]();
    return 0;
}

