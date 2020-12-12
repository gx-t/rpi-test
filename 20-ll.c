#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>

//https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2711/rpi_DATA_2711_1p0.pdf

struct bcm2811_peripherial {
    uint8_t* base;
    uint32_t* gpio_base;
} static bcm2811_peripherial = {0};

static int bcm2811_peripherial_open()
{
    const char* dev_mem = "/dev/mem";
    int fd = open(dev_mem, O_RDWR | O_SYNC);
    if(fd < 0) {
        perror(dev_mem);
        return 1;
    }
    bcm2811_peripherial.base = mmap(0, 0x200000 + 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0xFE000000);
    close(fd);

    if(MAP_FAILED == bcm2811_peripherial.base) {
        perror("mmap");
        return 2;
    }
    bcm2811_peripherial.gpio_base = (uint32_t*)(bcm2811_peripherial.base + 0x200000);
    return 0;
}

static void bcm2811_peripherial_close()
{
    munmap(bcm2811_peripherial.base, 0x200000 + 0x1000);
}

static void bcm2811_gpio04_set_input()
{
    bcm2811_peripherial.gpio_base[0] &= 0b111000000000000;
}

static void bcm2811_gpio04_set_output()
{
    bcm2811_peripherial.gpio_base[0] |= 0b001000000000000;
}

static void bcm2811_gpio04_set()
{
    bcm2811_peripherial.gpio_base[7] |= 0b10000;
}

static void bcm2811_gpio04_unset()
{
    bcm2811_peripherial.gpio_base[10] |= 0b10000;
}

//static int bcm2811_gpio04_get()
//{
//    !!(bcm2811_peripherial.gpio_base[13] & 0b10000);
//}

static int running = 1;

static void ctrl_c(int sig)
{
    running = 0;
    signal(SIGINT, ctrl_c);
}

int main( int argc, char* argv[] ) {
    int res = 0;

    signal(SIGINT, ctrl_c);

    if((res = bcm2811_peripherial_open()))
        return res;

    // Define pin 7 as output
    bcm2811_gpio04_set_input();
    bcm2811_gpio04_set_output();

    while(running) {
        bcm2811_gpio04_set();
        sleep(1);

        bcm2811_gpio04_unset();
        sleep(1);
    }

    bcm2811_peripherial_close();
    return 0;
}

