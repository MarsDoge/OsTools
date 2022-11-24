#include <unistd.h>
#include "def.h"
#include "i2c.h"

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000
#define CPU_I2C0 0x1fe00120
#define NODE0_CPU_I2C1 0x1fe00130
#define NODE4_CPU_I2C1 0x40001fe00130ULL
#define NODE8_CPU_I2C1 0x80001fe00130ULL
#define NODE12_CPU_I2C1 0xC0001fe00130ULL

static const char *const mps_usages[] = {
    PROGRAM_NAME" mps <args>",
    NULL,
};

//#define MPS_ADDR 0x3b //single
#define MPS_ADDR 0x4b

#define I2C_CTL_CLK 100000
#define I2C_BUS_RATE 42
#define ALGORITHM_3A 0x3a

unsigned long long devaddr[4] = {
    NODE0_CPU_I2C1,
    NODE4_CPU_I2C1,
    NODE8_CPU_I2C1,
    NODE12_CPU_I2C1,
};

int mps_read (int id)
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

    Val16 = 0x0; //a2
    I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

    //Vddn IOUT
    I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x8c, 0x2, &Val16);
    printf ("qdy %s line:%d Vddn IOUT:%d \n", __func__,__LINE__,Val16);

    Val16 = 0x0; //a2
    I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);

    //Vddn IOUT
    I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x96, 0x2, &Val16);
    printf ("qdy %s line:%d Vddn Power:%d \n", __func__,__LINE__,Val16);

    //Vddn IOUT
    I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x8b, 0x2, &Val16);
    printf ("qdy %s line:%d Vddn Value:%d \n", __func__,__LINE__,Val16);

    /*Vp sensed*/
    Val16 = 0x1; //a2
    I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

    //Vddp IOUT
    I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x8c, 0x2, &Val16);
    printf ("qdy %s line:%d    Vddp IOUT %d\n", __func__,__LINE__,Val16);

    //Vddp IOUT
    I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x8b, 0x2, &Val16);
    printf ("qdy %s line:%d    Vddp IOUT %d\n", __func__,__LINE__,Val16);

    //Vddp IOUT
    I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x96, 0x2, &Val16);
    printf ("qdy %s line:%d    Vddp Power %d\n", __func__,__LINE__,Val16);

    /*PIN Power*/
    Val16 = 0x0; //a2
    I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

    //Vddn IOUT
    I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x97, 0x2, &Val16);
    printf ("qdy %s line:%d  Pin Power %d \n", __func__,__LINE__,Val16);

    Val16 = 0x0; //a2
    I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);

    close(fd);

    return status;
}

int mps_write (int id)
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
    UINT32 Val16;

    while (1) {
        I2cRegBaseAddr = (UINT64) p;
        I2cInitSetFreq (I2cRegBaseAddr, 100000, 42, 0x3a); //Lock Freq

        Val16 = 0x0; //a2
        I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

        Val16 = 0x210;
        //Vddn IOUT
        I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x5e, 0x2, &Val16);

        char str[30] = {0};

        //TODO:
        scanf("%s",str);
        sscanf (str,"%d",&Val16);
        Val16 = Val16 / 10 - 49;
        I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x21, 0x2, &Val16);

        Val16 = 0x0; //a2
        I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
    }
    close (fd);
    return 0;
}

int cmd_mps (int argc, const char **argv)
{
    int read = 0;
    int write = 0;
    int id = 0;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN ('r', "read", &read, "read mps", NULL, 0, 0),
        OPT_BOOLEAN ('w', "write", &write, "write mps", NULL, 0, 0),
        OPT_INTEGER ('i', "id", &id, "mps id(0-3)", NULL, 0, 0),
        OPT_END(),
    };

    argparse_init(&argparse, options, mps_usages, 0);
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
        mps_read (id);
    } else if (write) {
        if (id > 3) {
            printf("id is 0~3\n");
            return -1;
        }
        mps_write (id);
    }
    return 0;
}
