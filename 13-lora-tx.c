#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pigpio.h>

static int lora_read_reg(int spi, uint8_t reg, uint8_t* val)
{
    char rx[2] = {0, 0}, tx[2] = {reg, 0};
    if(0 > spiXfer(spi, tx, rx, 2)) {
        perror("lora_reg_read");
        return 3;
    }
    *val = rx[1];
    return 0;
}

static int lora_write_reg(int spi, uint8_t reg, uint8_t val)
{
    char rx[2] = {0, 0}, tx[2] = {0x80 | reg, val};
    if(0 > spiXfer(spi, tx, rx, 2)) {
        perror("lora_reg_write");
        return 3;
    }
    return 0;
}

static void print_reg(int spi, uint8_t reg)
{
    uint8_t val = 0;
    lora_read_reg(spi, reg, &val);
    fprintf(stderr, "REG 0x%02X=0x%02X\n", reg, val);
}

static uint8_t lora_check_tx_done(int spi)
{
    uint8_t val = 0;
    lora_read_reg(spi, 0x12, &val);
    return !!(0b0001000 & val);
}

static void lora_reset_irq(int spi)
{
    lora_write_reg(spi, 0x12, 0xff);
}

static void lora_tx(int spi)
{
    static uint8_t lora_init_blob[] = {
        0x01, 0x88 //Sleep Mode
            , 0x06, 0x6c //MSB 433920000 Hz
            , 0x07, 0x7a //Mid.
            , 0x08, 0xe1 //LSB
            , 0x09, 0xff //Max TX Power
            , 0x0B, 0b00001011 //OCP off
            , 0x0E, 0x00 //TX base address
            , 0x0F, 0x00 //RX base address
            , 0x1D, 0x23 //BW, CR, header mode
            , 0x1E, 0xc4 //SF, CRC
            , 0x20, 0x0 //Preamble len MSB
            , 0x21, 0x6 //Preamble len LSB
            , 0x22, 0x1 //Payload length = 1
            , 0x26, 0xc //Low Data Rate Optimize, AGC
            , 0x31, 0xC3 //Data Detection Optimize for 7..12
            , 0x37, 0x0a //Data Detection Threshold for 7..12
            , 0x39, 0x12 //Synch Word
            , 0x40, 0x40 //Map TX Done to DIO0
            , 0x4D, 0x87 //PA Boost On
            , 0x01, 0x8b //Transmit (TX)

            , 0xFF, 0xFF //END
    };
    const uint8_t* pp = lora_init_blob;
    gpioWrite(4, 0);
    usleep(100);
    gpioWrite(4, 1);
    usleep(5000);
    while(0xFF != *pp) {
        lora_write_reg(spi, pp[0], pp[1]);
        pp += 2;
    }
    

    while(1 != gpioRead(17)) {
        sleep(1);
    }
    if(!lora_check_tx_done(spi)) {
        fprintf(stderr, "False interrupt!\n");
        return;
    }
    lora_reset_irq(spi);
    fprintf(stderr, "Sent!\n");
}

int main()
{
    if(0 > gpioInitialise()) {
        perror("gpioInitialise");
        return 1;
    }
    gpioSetMode(4, PI_OUTPUT);
    gpioSetMode(17, PI_INPUT);
    int spi = spiOpen(0, 32000, 0);
    if(0 > spi) {
        return 2;
    }
    print_reg(spi, 0x42);
    lora_tx(spi);
    spiClose(spi);
    return 0;
}
