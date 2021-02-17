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
    if(2 != spiXfer(spi, tx, rx, 2)) {
        perror(__func__);
        return 4;
    }
    *val = rx[1];
    return 0;
}

static int nrf24_write_reg(uint8_t reg, uint8_t val)
{
    char rx[2] = {0, 0}, tx[2] = {0x20 | (reg & 0x1F), val};
    if(2 != spiXfer(spi, tx, rx, 2)) {
        perror(__func__);
        return 5;
    }
    return 0;
}

static int nrf24_clear_irq_flags()
{
   return  nrf24_write_reg(0x07, 0b01110000); //clear interrupt bits
}

static void nrf24_pulse_ce()
{
    gpioWrite(17, 1); //chip enable
    usleep(10);
    gpioWrite(17, 0); //chip disable
}

static void nrf24_wait_irq()
{
    while(running && gpioRead(25)) {
        usleep(10);
    }
}

static int nrf24_tx_fill_fifo()
{
    static int i = 0;
    char rx[33] = {0}, tx[33] = {0xA0};
    memset(tx + 1, i++ & 0xFF, 32);
    if(33 != spiXfer(spi, tx, rx, 33)) {
        perror(__func__);
        return 7;
    }
    return 0;
}

static int nrf24_tx_reuse_pl()
{
    char rx = 0, tx = 0b11100011;
    if(1 != spiXfer(spi, &tx, &rx, 1)) {
        perror(__func__);
        return 8;
    }
    return 0;
}

//TODO: check "illegal" 0x00 => 0x03 (zero length address ?)
static int nrf24_tx_set_addr()
{
    char rx[6] = {0}, tx[6] = {0x20 | 0x10, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
    if(nrf24_write_reg(0x03, 0x03) || 6 != spiXfer(spi, tx, rx, 6)) { //address length=5, address=...
        perror(__func__);
        return 6;
    }
    return 0;
}

static void nrf24_tx_setup(uint8_t channel, uint8_t power, uint8_t bitrate)
{
    channel &= 0b01111111;
    power &= 0b11;
    bitrate = !!bitrate;

    fprintf(stderr, "%s  %s: using channel %d (%d Mhz) and power level %d, bitrate %d\n", __argv__[0], __argv__[1], channel, 2400 + channel, power, bitrate);
    nrf24_write_reg(0x00, 0b01111110); //no interrupts, CRC 2 bytes, power up, TX
    usleep(1500); //data sheet p.20, f.3
    nrf24_write_reg(0x00, 0b01011110); //interrupt TX_DS, CRC 2 bytes, power up, TX
    nrf24_write_reg(0x01, 0b00000000); //no auto-acknowledgement
    nrf24_write_reg(0x04, 0b00000000); //no auto-retransmit
    nrf24_write_reg(0x05, (uint8_t)channel); //set channel (data sheet page 54)
    nrf24_write_reg(0x06, 0b00000000 | (power << 1) | (bitrate << 3)); //No PLL lock, set bitrate, set power, no LNA
    nrf24_tx_set_addr();
}

static void nrf24_tx_send_block()
{
    nrf24_tx_fill_fifo();
    nrf24_clear_irq_flags();
    nrf24_pulse_ce();
    nrf24_wait_irq();
    nrf24_clear_irq_flags();
}

//TODO: check "illegal" 0x00 => 0x03 (zero length address ?)
static int nrf24_rx_set_p0_addr()
{
    char rx[6] = {0}, tx[6] = {0x20 | 0x0A, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
    if(nrf24_write_reg(0x03, 0x03) || 6 != spiXfer(spi, tx, rx, 6)) { //address length=5, address=...
        perror(__func__);
        return 6;
    }
    return 0;
}

static int nrf24_rx_read_fifo()
{
    int i = 0;
    char rx[33] = {0}, tx[33] = {0x61};
    if(33 != spiXfer(spi, tx, rx, 33)) {
        perror(__func__);
        return 7;
    }
    for(i = 1; i < sizeof(rx); i ++) {
        printf("%02X", rx[i]);
    }
    printf("\n");
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
    fprintf(stderr, "\t%s scan\n", __argv__[0]);
    fprintf(stderr, "\t%s carrier <channel> <power level>\n", __argv__[0]);
    fprintf(stderr, "\t%s tx <channel> <power level> <bitrate>\n", __argv__[0]);
    fprintf(stderr, "\t%s rx <channel> <bitrate>\n", __argv__[0]);
    fprintf(stderr, "\t\tchannel: 0-127\n");
    fprintf(stderr, "\t\tpower level: 0-3\n");
    fprintf(stderr, "\t\tbitrate: 0-1 (0 = 1Mbps, 1 = 2Mbps)\n");
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
    fprintf(stderr, "%s %s: using channel %d (%d Mhz) and power level %d\n", __argv__[0], __argv__[1], ch, 2400 + ch, power);
    nrf24_write_reg(0x00, 0x02); //power up, TX
    usleep(1500); //data sheet p.20, f.3
    nrf24_write_reg(0x05, (uint8_t)ch); //set channel (data sheet page 54)
    nrf24_write_reg(0x06, 0x90 | (power << 1)); //AN p.5. Bit 7 - "Cont wave" is not documented in datasheet, PLL lock
    gpioWrite(17, 1); //chip enable
    while(running) {
        pause();
    }
    gpioWrite(17, 0); //chip disable
    nrf24_write_reg(0x00, 0x00); //power down, TX
    return 0;
}

static int f_tx()
{
    if(__argc__ != 5) {
        show_usage();
        return 1;
    }
    int channel = atoi(__argv__[2]);
    int power = atoi(__argv__[3]);
    int bitrate = atoi(__argv__[4]);
    nrf24_tx_setup(channel, power, bitrate);
    while(running) {
        //TODO: Check if power down after each block send requires setup for everything
        nrf24_tx_send_block();
        sleep(1);
    }
    nrf24_write_reg(0x00, 0x00); //power down, TX
    return 0;
}

static int f_rx()
{
    if(__argc__ != 4) {
        show_usage();
        return 1;
    }
    int channel = atoi(__argv__[2]);
    int bitrate = atoi(__argv__[3]);
    channel &= 0b01111111;
    bitrate &= 1;
    nrf24_write_reg(0x00, 0b01111111); //no interrupts, CRC 2 bytes, power up, RX
    usleep(1500); //data sheet p.20, f.3
    nrf24_write_reg(0x00, 0b00111111); //interrupt RX_DR, CRC 2 bytes, power up, RX
    nrf24_write_reg(0x01, 0x00); //Disable all auto acknowledge
    nrf24_write_reg(0x02, 0b00000001); //Enable only data pipe 0
    nrf24_write_reg(0x04, 0x00); //Disable all auto retransmit
    nrf24_write_reg(0x05, channel); //set channel (data sheet page 54)
    nrf24_write_reg(0x06, 0b00000001 | (bitrate << 3)); //No PLL lock, set bitrate, LNA on
    nrf24_rx_set_p0_addr();
    nrf24_write_reg(0x11, 0x20); //Number of bytes in RX payload in data pipe 0
    while(running) {
        gpioWrite(17, 1); //chip enable
        nrf24_wait_irq();
        gpioWrite(17, 0); //chip disable
        nrf24_clear_irq_flags();
        nrf24_rx_read_fifo();
    }
    nrf24_write_reg(0x00, 0x01); //power down, RX
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
    gpioSetMode(25, PI_INPUT);
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
    if(!strcmp("tx", argv[1]))
        return spi_op(f_tx);
    if(!strcmp("rx", argv[1]))
        return spi_op(f_rx);
    fprintf(stderr, "Unknown operation: %s\n", argv[1]);
    show_usage();
    return 2;
}

