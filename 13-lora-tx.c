#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pigpio.h>

static void lora_reset()
{
    gpioSetMode(4, PI_OUTPUT);
    gpioWrite(4, 0);
    usleep(100);
    gpioWrite(4, 1);
    usleep(5000);
}

static int lora_read_reg(int spi, unsigned char reg, uint8_t* val)
{
    char rx[2] = {0, 0}, tx[2] = {reg, 0};
    if(0 > spiXfer(spi, tx, rx, 2)) {
        perror("lora_reg_read");
        return 3;
    }
    *val = rx[1];
    return 0;
}

static void print_reg(int spi, uint8_t reg)
{
    uint8_t val = 0;
    lora_read_reg(spi, reg, &val);
    fprintf(stderr, "REG 0x%02X=0x%02X\n", reg, val);
}

int main()
{
    if(0 > gpioInitialise()) {
        perror("gpioInitialise");
        return 1;
    }
    lora_reset();
    int spi = spiOpen(0, 32000, 0);
    if(0 > spi) {
        return 2;
    }
    print_reg(spi, 0x42);
    spiClose(spi);
    return 0;
}
