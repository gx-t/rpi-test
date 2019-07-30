#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#define PI              3.14159265358979323846
#define SAMP_RATE       96000 //Fs = 96KHZ
#define BLOCK_SIZE      (SAMP_RATE / 10) //100 ms block
#define FREQ_F32(f_)    ((f_) * 2 * PI / SAMP_RATE)
#define NOTE_CNT        84

static int running = 1;

static void ctrl_c(int sig)
{
    running = 0;
    signal(SIGINT, ctrl_c);
}

static float f_vmlasr_f32(float c[], float s[], const float f[], int cnt)
{
    float sum = 0;
    int i;

    for(i = 0; i < cnt; i ++) {
        c[i] += s[i] * f[i];
        s[i] -= c[i] * f[i];
        sum += s[i];
    }

    return sum / (float)cnt;
}

static float f_vmlamr_f32(float c[], float s[], const float f[], int cnt)
{
    float mul = 1.0;
    int i;

    for(i = 0; i < cnt; i ++) {
        c[i] += s[i] * f[i];
        s[i] -= c[i] * f[i];
        mul *= s[i];
    }

    return mul;
}

static void f_vdiv_f32(float dst[], const float op1[], const float op2[], int cnt)
{
    int i;
    for(i = 0; i < cnt; i ++)
        dst[i] = op1[i] / op2[i];
}

static void f_vmul_f32(float dst[], const float op1[], const float op2[], int cnt)
{
    int i;
    for(i = 0; i < cnt; i ++)
        dst[i] = op1[i] * op2[i];
}

static void f_fill_all_notes(float s[], float c[], float q[], float f[])
{
    int i;
    float f0;
    for(i = 0, f0 = 32.70320; i < NOTE_CNT; i ++, f0 *= 1.0594630943592953) {
        s[i] = c[i] = 0.0;
        q[i] = 1.0001;
        f[i] = FREQ_F32(f0);
    }
}

static void f0_tone()
{
    float c[2] = {0.75, 0.75};
    float s[2] = {0.0, 0.0};
    float f[2] = {FREQ_F32(19900), FREQ_F32(20000)};
    const float k_down[2] = {1.00001, 1.00001};
    const float k_up[2] = {1.00002, 1.00002};
    float  buff[BLOCK_SIZE], *pp;
    int i;

    while(running && f[0] > FREQ_F32(3100)) {
        i = BLOCK_SIZE;
        pp = buff;

        while(i--) {
            *pp ++ = f_vmlasr_f32(c, s, f, 2);
            f_vdiv_f32(f, f, k_down, 2);
        }
        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }

    while(running && f[0] < FREQ_F32(19900)) {
        i = BLOCK_SIZE;
        pp = buff;

        while(i--) {
            *pp ++ = f_vmlasr_f32(c, s, f, 2);
            f_vmul_f32(f, f, k_up, 2);
        }
        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }
}

static void f1_tone()
{
    float c[4] = {0.75, 0.75, 0.0, 0.0};
    float s[4] = {0.0, 0.0, 0.0, 0.0};
    float f[4] = {FREQ_F32(2093.005), FREQ_F32(2637.020), FREQ_F32(1864.655), FREQ_F32(2349.318)};
    float d[4] = {1.00004, 1.00004, 1.00004, 1.00004};
    float buff[BLOCK_SIZE], *pp;
    int i = 0, cnt = 5;

    while(cnt --) {
        i = BLOCK_SIZE;
        pp = buff;

        while(i --) {
            *pp ++ = f_vmlasr_f32(c, s, f, 4);
            f_vdiv_f32(s, s, d, 4);
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
            *pp ++ = f_vmlasr_f32(c, s, f, 4);
            f_vdiv_f32(s, s, d, 4);
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

            *pp ++ = f_vmlasr_f32(&c0, &s0, &f0, 1) * a;
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
    float c[2] = {0.75, 0.75};
    float s[2] = {0.0, 0.0};
    float f[2] = {FREQ_F32(2900.0), FREQ_F32(10.0)};

    float buff[BLOCK_SIZE], *pp = buff;
    int i, count;

    for(count = 0; count < 10; count ++) {
        pp = buff;
        i = BLOCK_SIZE;
        while(i --)
            *pp ++ = f_vmlamr_f32(c, s, f, 2);

        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }
    for(count = 0; count < 10; count ++) {
        pp = buff;
        i = BLOCK_SIZE / 2;
        while(i --) {
            *pp ++ = f_vmlamr_f32(c, s, f, 2);
            s[1] /= 1.0004;
        }
        i = BLOCK_SIZE / 2;
        while(i --) {
            *pp ++ = f_vmlamr_f32(c, s, f, 2);
        }

        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }
}

static void f5_tone()
{
    float c[NOTE_CNT], s[NOTE_CNT], f[NOTE_CNT], q[NOTE_CNT], buff[BLOCK_SIZE], *pp;
    int i, note_idx;

    f_fill_all_notes(c, s, q, f);

    for(note_idx = 0; note_idx < NOTE_CNT; note_idx ++) {

        s[note_idx] = 10;
        pp = buff;
        for(i = 0; i < BLOCK_SIZE; i ++) {
            *pp ++ = f_vmlasr_f32(c, s, f, NOTE_CNT);
            f_vdiv_f32(s, s, q, NOTE_CNT);
        }

        if(sizeof(buff) != write(1, buff, sizeof(buff)))
            break;
    }
}

static void show_usage(const char* cmd, int max_tone_idx)
{
    fprintf(stderr, "play:      %s [0-%d] | play -r 96k -t f32 -c 1 -\n", cmd, max_tone_idx);
    fprintf(stderr, "flac file: %s [0-%d] | sox -r 96k -t f32 -c 1 - out.flac\n", cmd, max_tone_idx);
    fprintf(stderr, "ogg file:  %s [0-%d] | sox -r 96k -t f32 -c 1 - out.ogg\n", cmd, max_tone_idx);
}

int main(int argc, char* argv[])
{
    void (*f_arr[])() = {f0_tone, f1_tone, f2_tone, f3_tone, f4_tone, f5_tone};
    signal(SIGINT, ctrl_c);
    if(2 != argc) {
        show_usage(*argv, sizeof(f_arr) / sizeof(f_arr[0]) - 1);
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

