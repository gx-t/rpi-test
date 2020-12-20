#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>

//https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf

//rpi4:
//https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2711/rpi_DATA_2711_1p0.pdf

struct bcm2835_peripherial {
    uint8_t* base;
    uint8_t* gpio_base;
} static bcm2835_peripherial = {0};

static int bcm2835_peripherial_open()
{
    const char* dev_mem = "/dev/mem";
    int fd = open(dev_mem, O_RDWR | O_SYNC);
    if(fd < 0) {
        perror(dev_mem);
        return 1;
    }
    //bcm2711 (rpi4) - 0xfe000000
    bcm2835_peripherial.base = mmap(0, 0x200000 + 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x3f000000);
    close(fd);

    if(MAP_FAILED == bcm2835_peripherial.base) {
        perror("mmap");
        return 2;
    }
    bcm2835_peripherial.gpio_base = bcm2835_peripherial.base + 0x200000;
    return 0;
}

static void bcm2835_peripherial_close()
{
    munmap(bcm2835_peripherial.base, 0x200000 + 0x1000);
}

static void bcm2835_gpio04_set_input()
{
    bcm2835_peripherial.gpio_base[0] &= ~(0b000 << 12);
}

static void bcm2835_gpio04_set_output()
{
    bcm2835_peripherial.gpio_base[0] |= (0b001 << 12);
}

static void bcm2835_gpio04_set_gpclk0()
{
    bcm2835_peripherial.gpio_base[0] |= (0b100 << 12);
}

static void bcm2835_gpio04_set()
{
    bcm2835_peripherial.gpio_base[0x1c] |= (0b1 << 4);
}

static void bcm2835_gpio04_unset()
{
    bcm2835_peripherial.gpio_base[0x28] |= (0b1 << 4);
}

//static int bcm2835_gpio04_get()
//{
//    !!(bcm2835_peripherial.gpio_base[0x34] & (0b1 << 4));
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

    if((res = bcm2835_peripherial_open()))
        return res;

    // Define pin 7 as output
    bcm2835_gpio04_set_input();
    bcm2835_gpio04_set_output();

    while(running) {
        bcm2835_gpio04_set();
        sleep(1);

        bcm2835_gpio04_unset();
        sleep(1);
    }

    bcm2835_peripherial_close();
    return 0;
}
