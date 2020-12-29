#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

//https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf

//rpi4:
//https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2711/rpi_DATA_2711_1p0.pdf

struct bcm2835_peripherial {
    uint8_t* base;
    uint32_t* gpio_base;
    uint32_t* cm_gp0ctl;
    uint32_t* cm_gp0div;
} static bcm2835_peripherial = {0};

static void bcm2835_gpio04_set_input()
{
    bcm2835_peripherial.gpio_base[0 / 4] &= ~(0b111 << 12);
}

static void bcm2835_gpio04_set_output()
{
    bcm2835_peripherial.gpio_base[0x00 / 4] |= (0b001 << 12);
}

static void bcm2835_gpio04_set_gpclk0()
{
    bcm2835_peripherial.gpio_base[0x00 / 4] |= (0b100 << 12);
}

static void bcm2835_gpio04_set()
{
    bcm2835_peripherial.gpio_base[0x1C / 4] |= (0b1 << 4);
}

static void bcm2835_gpio04_unset()
{
    bcm2835_peripherial.gpio_base[0x28 / 4] |= (0b1 << 4);
}

//static int bcm2835_gpio04_get()
//{
//    !!(bcm2835_peripherial.gpio_base[13] & 0b10000);
//}

static int running = 1;
static int _argc = 0;
static char** _argv = 0;

static int f_blink()
{
    bcm2835_gpio04_set_input();
    bcm2835_gpio04_set_output();

    while(running) {
        bcm2835_gpio04_set();
        if(usleep(1e6))
            break;

        bcm2835_gpio04_unset();
        if(usleep(1e6))
            break;
    }
    return 0;
}

static void show_usage_freq()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t%s %s <frequency>\n", _argv[0], _argv[1]);
    fprintf(stderr, "\tfrequency in Hz, positive number.\n");
    fprintf(stderr, "\tfrequency 0, to stop the generation.\n");
}

static int f_freq()
{
    if(3 != _argc) {
        show_usage_freq();
        return 5;
    }

    double freq = atof(_argv[2]);
    if(freq < 0) {
        fprintf(stderr, "Frequency must be positive.\n");
        show_usage_freq();
        return 6;
    }

    bcm2835_gpio04_set_gpclk0();
    //TODO: check PLLA
    struct {
        uint32_t src : 4;
        uint32_t enab : 1;
        uint32_t kill : 1;
        uint32_t : 1;
        uint32_t busy : 1;
        uint32_t flip : 1;
        uint32_t mash : 2;
        uint32_t : 13;
        uint32_t passwd : 8;
    } cm_gp0ctl = {
        .src = 6,
        .enab = 1,
        .mash = 1,
        .passwd = 0x5a
    };

    if(0 == freq) {
        cm_gp0ctl.enab = 0;
        *bcm2835_peripherial.cm_gp0ctl = *(uint32_t*)&cm_gp0ctl;
        return 0;
    }

    uint32_t div = (uint32_t)(0.5 + 500e6 / freq * 4096.0);
    if(div < 1 || div > 0x1000000 - 1) {
        fprintf(stderr, "Frequency value is out of range.\n");
        show_usage_freq();
        return 7;
    }

    *bcm2835_peripherial.cm_gp0ctl = *(uint32_t*)&cm_gp0ctl;

    struct {
        uint32_t div : 24;
        uint32_t passwd : 8;
    } cm_gp0div = {
        .div = div,
        .passwd = 0x5a
    };
    *bcm2835_peripherial.cm_gp0div = *(uint32_t*)&cm_gp0div;
    return 0;
}

static int ll_operation(int (*f)())
{
    int res = 0;
    const char* dev_mem = "/dev/mem";
    int fd = open(dev_mem, O_RDWR | O_SYNC);
    if(fd < 0) {
        perror(dev_mem);
        return 3;
    }
    //bcm2711 (rpi4) - 0xfe000000
    bcm2835_peripherial.base = mmap(0, 0x200000 + 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x3f000000);
    close(fd);

    if(MAP_FAILED == bcm2835_peripherial.base) {
        perror("mmap");
        return 4;
    }
    bcm2835_peripherial.gpio_base = (uint32_t*)(bcm2835_peripherial.base + 0x200000);
    bcm2835_peripherial.cm_gp0ctl = (uint32_t*)(bcm2835_peripherial.base + 0x101070);
    bcm2835_peripherial.cm_gp0div = (uint32_t*)(bcm2835_peripherial.base + 0x101074);

    res = f();

    munmap(bcm2835_peripherial.base, 0x200000 + 0x1000);
    return res;
}

static void show_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s blink\n", _argv[0]);
    fprintf(stderr, "%s freq <param>\n", _argv[0]);
}

static void ctrl_c(int sig)
{
    running = 0;
    signal(SIGINT, ctrl_c);
}

int main(int argc, char* argv[]) {
    _argc = argc;
    _argv = argv;
    signal(SIGINT, ctrl_c);

    if(2 > argc) {
        show_usage();
        return 1;
    }

    if(!strcmp("blink", argv[1]))
        return ll_operation(f_blink);
    if(!strcmp("freq", argv[1]))
        return ll_operation(f_freq);

    fprintf(stderr, "Unknown sub-commnd: %s\n", argv[1]);
    return 2;
}

