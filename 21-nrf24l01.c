// http://shah32768.sdf.org/pdf/nRF24L01_Product_Specification_v2_0-9199.pdf
// http://shah32768.sdf.org/pdf/nRF%20Performance%20Test%20Instructions.%20nRF24L01+.%20Application%20Note.pdf
// http://shah32768.sdf.org/pdf/Application%20note,Nordic%20nRF24L01%20with%20Bascom-Avr.pdf

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pigpio.h>

static int running = 1;

static void ctrl_c(int sig) {
    signal(SIGINT, ctrl_c);
    running = 0;
    fprintf(stderr, "\n");
}

static int __argc__ = 0;
static char** __argv__ = 0;
static int spi = 0;

static int nrf24_read_reg(uint8_t reg, uint8_t* val)
{
    char rx[2] = {0, 0}, tx[2] = {reg, 0};
    if(0 > spiXfer(spi, tx, rx, 2)) {
        perror(__func__);
        return 4;
    }
    *val = rx[1];
    return 0;
}

static int nrf24_write_reg(uint8_t reg, uint8_t val)
{
    char rx[2] = {0, 0}, tx[2] = {0x20 | (reg & 0x1F), val};
    if(0 > spiXfer(spi, tx, rx, 2)) {
        perror(__func__);
        return 5;
    }
    return 0;
}

static int nrf24_print_regs()
{
    int res = 0;
    uint8_t reg = 0;
    for(reg = 0; !res && reg < 0x18; reg ++) {
        uint8_t val = 0;
        if((res = nrf24_read_reg(reg, &val))) {
            perror(__func__);
            break;
        }
        fprintf(stderr, "R0x%02X=0x%02X ", reg, val);
    }
    fprintf(stderr, "\n");
    return res;
}

static void show_usage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t%s scan\n", *__argv__);
    fprintf(stderr, "\t%s carrier <channel> <power level>\n", *__argv__);
    fprintf(stderr, "\t\t%s channel: 0-127\n", *__argv__);
    fprintf(stderr, "\t\t%s power level: 0-3\n", *__argv__);
}

//based on "nRF Performance Test Instructions nRF24L01+ Application Note" => 6 Receiver sensitivity
static int f_scan()
{
    nrf24_write_reg(0x00, 0x03); //power up, RX
    usleep(1500); //data sheet p.20, f.3
    nrf24_write_reg(0x01, 0x00); //Disable all auto acknowledge
    nrf24_write_reg(0x04, 0x00); //Disable all auto retransmit
    nrf24_write_reg(0x06, 0b1001); //LNA on, 2Mbit/s
    uint32_t hist[128] = {0};
    uint8_t ch = 0;
    while(running) {
        int line = 32;
        printf(" 2400 Mhz | 2410 Mhz | 2420 Mhz | 2430 Mhz | 2440 Mhz | 2450 Mhz | 2460 Mhz | 2470 Mhz | 2480 Mhz | 2490 Mhz | 2500 Mhz | 2520 Mhz | 2520 Mhz \n");
        while(running && line --) {
            for(ch = 0; running && ch < 128; ch ++) {
                nrf24_write_reg(0x05, ch); //set channel (data sheet page 54)
                gpioWrite(17, 1); //chip enable
                usleep(500); //data sheet p.20, f.3
                gpioWrite(17, 0); //chip disable
                uint8_t cd = 0;
                nrf24_read_reg(0x09, &cd); //carrier detect (data sheet page 55)
                if(ch && !(ch % 10))
                    printf("|");
                printf(cd & 1 ? "*" : " ");
                hist[ch] += (cd & 1);
            }
            printf("\n");
        }
    }
    gpioWrite(17, 0); //chip disable
    nrf24_write_reg(0x00, 0x01); //power down, RX
    for(ch = 0; ch < 128; ch ++) {
        printf("Channel %u, %d Mhz -- %u\n", ch, 2400 + ch, hist[ch]);
    }
    return 0;
}

static int f_carrier()
{
    if(__argc__ != 4) {
        show_usage();
        return 1;
    }
    int ch = atoi(__argv__[2]);
    int power = atoi(__argv__[3]);
    if(ch < 0 || ch > 127)
        ch = 0;
    if(power < 0 || power > 3)
        power = 3;
    fprintf(stderr, "%s: using channel %d (%d Mhz) and power level %d\n", __argv__[1], ch, 2400 + ch, power);
    nrf24_write_reg(0x00, 0x02); //power up, TX
    usleep(1500); //data sheet p.20, f.3
    nrf24_write_reg(0x05, (uint8_t)ch); //set channel (data sheet page 54)
    nrf24_write_reg(0x06, 0x90 | (power << 1)); //AN p.5. Bit 7 - "Cont wave" is not documented in datasheet.
    gpioWrite(17, 1); //chip enable
    while(running) {
        pause();
    }
    gpioWrite(17, 0); //chip disable
    nrf24_write_reg(0x00, 0x00); //power down, TX
    return 0;
}

static int spi_op(int (*f)())
{
    if(0 > gpioInitialise()) {
        return 3;
    }
    signal(SIGINT, ctrl_c);
    spi = spiOpen(0, 3200000, 0);
    if(0 > spi) {
        gpioTerminate();
        return 4;
    }
    gpioSetMode(17, PI_OUTPUT);
    gpioWrite(17, 0); //chip disable
    usleep(10300); //data sheet p.20, f.3
    nrf24_print_regs();
    int res = f();
    nrf24_write_reg(0x00, 0x00); //power down
    nrf24_print_regs();
    spiClose(spi);
    gpioTerminate();
    return res;
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
    if(!strcmp("carrier", argv[1]))
        return spi_op(f_carrier);
    fprintf(stderr, "Unknown operation: %s\n", argv[1]);
    show_usage();
    return 2;
}

