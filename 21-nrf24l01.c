// http://shah32768.sdf.org/pdf/nRF24L01_Product_Specification_v2_0-9199.pdf
// http://shah32768.sdf.org/pdf/nRF%20Performance%20Test%20Instructions.%20nRF24L01+.%20Application%20Note.pdf

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pigpio.h>

static int running = 1;

static void ctrl_c(int sig) {
    signal(SIGINT, ctrl_c);
    running = 0;
    printf("CTRL+C %d\n", running);
}

static int __argc__ = 0;
static char** __argv__ = 0;
static int spi = 0;

static int nrf24l01_read_reg(uint8_t reg, uint8_t* val)
{
    char rx[2] = {0, 0}, tx[2] = {reg, 0};
    if(0 > spiXfer(spi, tx, rx, 2)) {
        perror(__func__);
        return 4;
    }
    *val = rx[1];
    return 0;
}

static int nrf24l01_write_reg(uint8_t reg, uint8_t val)
{
    char rx[2] = {0, 0}, tx[2] = {0x20 | (reg & 0x1F), val};
    if(0 > spiXfer(spi, tx, rx, 2)) {
        perror(__func__);
        return 5;
    }
    return 0;
}

static int nrf24l01_print_regs()
{
    int res = 0;
    uint8_t reg = 0;
    for(reg = 0; !res && reg < 0x18; reg ++) {
        uint8_t val = 0;
        if((res = nrf24l01_read_reg(reg, &val))) {
            perror(__func__);
            break;
        }
        fprintf(stderr, "R0x%02X=0x%02X ", reg, val);
    }
    fprintf(stderr, "\n");
    return res;
}

static int f_scan()
{
    signal(SIGINT, ctrl_c);
    gpioWrite(17, 0); //chip disable
    usleep(10300); //p.20, f.3
    nrf24l01_print_regs();
    nrf24l01_write_reg(0x00, 0x03);
    usleep(1500); //p.20, f.3
    nrf24l01_write_reg(0x01, 0x00); //Disable all auto acknowledge
    nrf24l01_write_reg(0x04, 0x00); //Disable all auto retransmit
    nrf24l01_write_reg(0x06, 0b1001); //LNA on, 2Mbit/s
    uint32_t hist[128] = {0};
    uint8_t ch = 0;
    while(running) {
        int line = 32;
        printf(" 2400 Mhz | 2410 Mhz | 2420 Mhz | 2430 Mhz | 2440 Mhz | 2450 Mhz | 2460 Mhz | 2470 Mhz | 2480 Mhz | 2490 Mhz | 2500 Mhz | 2520 Mhz | 2520 Mhz \n");
        while(running && line --) {
            for(ch = 0; running && ch < 128; ch ++) {
                nrf24l01_write_reg(0x05, ch); //set channel (page 54)
                gpioWrite(17, 1); //chip enable
                usleep(500); //p.20, f.3
                gpioWrite(17, 0); //chip disable
                uint8_t cd = 0;
                nrf24l01_read_reg(0x09, &cd); //carrier detect (page 55)
                if(ch && !(ch % 10))
                    printf("|");
                printf(cd & 1 ? "*" : " ");
                hist[ch] += (cd & 1);
            }
            printf("\n");
        }
    }
    for(ch = 0; ch < 128; ch ++) {
        printf("Channel %u, %d Mhz -- %u\n", ch, 2400 + ch, hist[ch]);
    }
    return 0;
}

static int spi_op(int (*f)())
{
    if(0 > gpioInitialise()) {
        return 3;
    }
    spi = spiOpen(0, 3200000, 0);
    if(0 > spi) {
        gpioTerminate();
        return 4;
    }
    gpioSetMode(17, PI_OUTPUT);
    int res = f();
    spiClose(spi);
    gpioTerminate();
    return res;
}

static void show_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t%s scan\n", *__argv__);
}

int main(int argc, char* argv[])
{
    __argc__ = argc;
    __argv__ = argv;
    if(argc < 2) {
        show_usage();
        return 1;
    }
    if(!strcmp("scan", argv[1]))
        return spi_op(f_scan);
    fprintf(stderr, "Unknown operation: %s\n", argv[1]);
    show_usage();
    return 2;
}

