#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"
#include "i2c.h"

extern int is3d;
extern int is3c;

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000

//3D5000
#define CPU_3C5000_NODE0_I2C0 0x1fe00160
#define CPU_3C5000_NODE1_I2C0 0x10001fe00160ULL
#define CPU_3C5000_NODE2_I2C0 0x20001fe00160ULL
#define CPU_3C5000_NODE3_I2C0 0x30001fe00160ULL

#define Write64(addr, data)     (*(volatile UINT64*)(addr) = (data))
#define Write32(addr, data)     (*(volatile UINT32*)(addr) = (data))
#define Write16(addr, data)     (*(volatile UINT16*)(addr) = (data))
#define Writel(addr, data)      (*(volatile UINT32*)(addr) = (data))
#define Writew(addr, data)      (*(volatile UINT16*)(addr) = (data))
#define Writeb(addr, data)      (*(volatile UINT8*)(addr) = (data))
#define Read64(addr)            (*(volatile UINT64*)(addr))
#define Read32(addr)            (*(volatile UINT32*)(addr))
#define Read16(addr)            (*(volatile UINT16*)(addr))
#define Readl(addr)             (*(volatile UINT32*)(addr))
#define Readw(addr)             (*(volatile UINT16*)(addr))
#define Readb(addr)             (*(volatile UINT8*)(addr))


UINTN AvsRegBaseAddr;

static const char *const avs_usages[] = {
    PROGRAM_NAME" avs <args>",
    NULL,
};

unsigned long long devaddr_3c5000_avs[4] = {
    CPU_3C5000_NODE0_I2C0,
    CPU_3C5000_NODE1_I2C0,
    CPU_3C5000_NODE2_I2C0,
    CPU_3C5000_NODE3_I2C0
};

#define AVS_BASE AvsRegBaseAddr
#define AVS_CSR  AVS_BASE
#define AVS_MREG AVS_BASE + 0x4
#define AVS_SREG AVS_BASE + 0x8

/* p: 1    n: 0*/
VOID
AvsSetVol(int rail_sel, int vol, int rx_delay, int clk_div)
{
  if ((vol < 600) || (vol > 1300)) {
    printf("\r\nAVS: Set vol range error!!!\r\n");
    return;
  }

  Writel(AVS_CSR, 0x10000 | (clk_div << 17) | (0x7 << 25) | (rx_delay << 20));
  Writel(AVS_MREG, 0x80000000 | (vol << 4) | (rail_sel << 20));

  while(Readl(AVS_SREG) & 0x80000000);

  if((Readl(AVS_SREG) & 0x60000000)) {
    printf("set avs_vol erro!\n");
  }
}

/* p: 1    n: 0*/
UINTN
AvsGetVol(int rail_sel, int rx_delay, int clk_div)
{
  Writel(AVS_CSR, 0x10000 | (clk_div << 17) | (0x7 << 25) | (rx_delay << 20));
  Writel(AVS_MREG, 0xe0000000 | (rail_sel << 20));

  while(Readl(AVS_SREG) & 0x80000000);

  if((Readl(AVS_SREG) & 0x60000000))
    return 0;
  else
    return (Readl(AVS_SREG) & 0xffff);
}

/* p: 1    n: 0*/
VOID
AvsVolPrint()
{
  UINT32 Val;
  Val = AvsGetVol(0, 0, 4);
  if (!Val) {
    printf("AVS: Get Vddn error!\n");
  } else {
    printf("AVS: Get Vddn value is: %d \n",Val);
  }
  Val = AvsGetVol(1, 0, 4);
  if (!Val) {
    printf("AVS: Get Vddp error!\n");
  } else {
    printf("AVS: Get Vddp value is: %d \n",Val);
  }
}

int avs_read (int id)
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

    p = vtpa(devaddr_3c5000_avs[id], fd);

    UINT64 NodeId;
    UINT16 Val16;

    AvsRegBaseAddr = (UINT64) p;

    AvsVolPrint();

    close(fd);

    return status;
}

int avs_write_vdd (int id, int vol, int channel)
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

    p = vtpa(devaddr_3c5000_avs[id], fd);

    UINT64 NodeId;
    UINT32 Val16;

    AvsRegBaseAddr = (UINT64) p;

    AvsSetVol(channel, vol, 0, 4);

    close (fd);
    return 0;
}

int cmd_avs (int argc, const char **argv)
{
    int read = 0;
    int write = 0;
    int id = 0;
    int vol = 0;
    int channel = 0;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN ('r', "read", &read, "read avs", NULL, 0, 0),
        OPT_BOOLEAN ('w', "write", &write, "write avs", NULL, 0, 0),
        OPT_INTEGER ('i', "id", &id, "avs id(0-3)", NULL, 0, 0),
        OPT_INTEGER ('d', "vol", &vol, "vol(800-1200)", NULL, 0, 0),
        OPT_INTEGER ('c', "channel", &channel, "channel(0-1)", NULL, 0, 0),
        OPT_END(),
    };

    argparse_init(&argparse, options, avs_usages, 0);
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
        avs_read (id);
    } else if (write) {
        if (id > 3) {
            printf("id is 0~3\n");
            return -1;
        }
        if ((vol < 800) || (vol > 1200)) {
            printf("vol is 800~1200\n");
            return -1;
        }
        avs_write_vdd (id,vol,channel);
    }
    return 0;
}
