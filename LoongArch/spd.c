#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"
#include "i2c.h"

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000
#define CPU_I2C0 0x1fe00120
#define NODE0_CPU_I2C0 0x1fe00120
#define NODE4_CPU_I2C0 0x40001fe00120ULL
#define NODE8_CPU_I2C0 0x80001fe00120ULL
#define NODE12_CPU_I2C0 0xC0001fe00120ULL

//#define MPS_ADDR 0x3b //single
#define MPS_ADDR 0x4b

#define I2C_CTL_CLK 100000
#define I2C_BUS_RATE 42
#define ALGORITHM_3A 0x3a

static const char *const spd_usages[] = {
    PROGRAM_NAME" spd <args>",
    NULL,
};

static unsigned long long devaddr[4] = {
    NODE0_CPU_I2C0,
    NODE4_CPU_I2C0,
    NODE8_CPU_I2C0,
    NODE12_CPU_I2C0,
};

static int spd_read (int id, const char* addr)
{
    void * p = NULL;
    int status ;
    char RecordName[30] = {0};
    int c = 0;
    char str[30] = {0};
    int fd;

    fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    p = vtpa(devaddr[id], fd);

    UINT64 I2cRegBaseAddr, NodeId;
    UINT16 Val16;

    I2cRegBaseAddr = (UINT64) p;
    I2cInitSetFreq (I2cRegBaseAddr, 100000, 42, 0x3a); //Lock Freq

    int a = 0;
    sscanf (addr, "%x", &a);

    //Vddn IOUT
    I2cCtlRead (I2cRegBaseAddr, a, 0x02, 0x1, &Val16);
    switch (Val16) {
        case 0xc: 
            printf (" DDR4 Type:%d \n", Val16);
            break;
        default:
            printf (" No know Type:%d ! \n", Val16);
            break;
    }

    I2cCtlRead (I2cRegBaseAddr, a, 0x03, 0x1, &Val16);
    switch (Val16) {
        case 0x1: 
            printf (" DRAM Type RDIMM:%d \n", Val16);
            break;
        case 0x2: 
            printf (" DRAM Type UDIMM:%d \n", Val16);
            break;
        default:
            printf (" No know Type:%d ! \n", Val16);
            break;
    }
    close (fd);

    return 0;
}

int cmd_spd (int argc, const char **argv)
{
    int read = 0;
    int id = 0;
    const char *addr = NULL;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP ("Options:"),
        OPT_BOOLEAN ('r', "read", &read, "read spd", NULL, 0, 0),
        OPT_GROUP ("Arguments:"),
        OPT_INTEGER ('i', "id", &id, "spd id(0-3)", NULL, 0, 0),
        OPT_STRING  ('a', "address", &addr, "SPD address", NULL, 0, 0),
        OPT_END(),
    };

    argparse_init(&argparse, options, spd_usages, 0);
    argc = argparse_parse(&argparse, argc, argv);

    if (!(read || write)) {
        argparse_usage(&argparse);
        return 1;
    }

    uid = geteuid ();
    if (uid != 0) {
        printf("Please run with root!\n");
        return -1;
    }

    if (read) {
        if (id > 3) {
            printf("id is 0~3\n");
            return -1;
        }
        if (addr == NULL) {
            printf ("Please setup the address.\n");
            return 1;
        }
        spd_read (id, addr);
    }
    return 0;
}
