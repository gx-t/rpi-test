#include <stdio.h>
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
} static bcm2835_peripherial = {0};

static int bcm2835_peripherial_open()
{
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
    return 0;
}

static void bcm2835_peripherial_close()
{
    munmap(bcm2835_peripherial.base, 0x200000 + 0x1000);
}

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

static int f_blink()
{
    // Define pin 7 as output
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

static int ll_operation(int (*f)())
{
    int res = 0;
    if((res = bcm2835_peripherial_open()))
        return res;
    res = f();
    bcm2835_peripherial_close();
    return res;
}

static void show_usage(const char* argv_0)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "%s blink\n", argv_0);
}

static void ctrl_c(int sig)
{
    running = 0;
    signal(SIGINT, ctrl_c);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, ctrl_c);

    if(2 != argc) {
        show_usage(argv[0]);
        return 1;
    }

    if(!strcmp("blink", argv[1]))
        return ll_operation(f_blink);

    fprintf(stderr, "Unknown sub-commnd: %s\n", argv[1]);
    return 2;
}

