// SPDX-License-Identifier: GPL-2.0
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000
#define PCI_CONFUSE_SPACE 0x0efdfe000000

static const char *const pci_usages[] = {
    PROGRAM_NAME" pci <args>",
    NULL,
};

static int pci_read (const char* addr)
{
    void * p = NULL;
    int status ;
    char RecordName[30] = {0};
    int c = 0;
    char str[30] = {0};
    unsigned long long devaddr;
    int fd;

    devaddr = PCI_CONFUSE_SPACE;

    fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    p = vtpa(devaddr,fd);

    int a = 0;
    sscanf (addr, "%x", &a);

    printf("your input Pci Offset 0x%lx \n",a);

    printf(" Auto Read Pci(Bus:%d Device:%d Function:%d) 0x10 Bar Mem Start, 0x%lx \n",(a>>16),((a>>11)&0x1f),((a>>8)&7), *(volatile unsigned long *)(p + a + 0x10));
    printf(" Auto Read Pci(Bus:%d Device:%d Function:%d) 0x10 Bar Mem Start, 0x%lx \n",(a>>16),((a>>11)&0x1f),((a>>8)&7), *(volatile unsigned long *)(p + a + 0x4));
    //lack auto know device name array
    return 0;
}

int cmd_pci (int argc, const char **argv)
{
    int read = 0;
    const char *addr = NULL;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN ('r', "read", &read, "Read Pci Bar Mem Start", NULL, 0, 0),
        OPT_STRING  ('a', "address", &addr, "Address offset composed of Bus", NULL, 0, 0),
        OPT_END(),
    };

    argparse_init(&argparse, options, pci_usages, 0);
    argc = argparse_parse(&argparse, argc, argv);

    if (!(read)) {
        argparse_usage(&argparse);
        return 1;
    }

    uid = geteuid ();
    if (uid != 0) {
        printf("Please run with root!\n");
        return -1;
    }

    if (read) {
        if (addr == NULL) {
            printf ("Please setup the offset address.\n");
            return 1;
        }
        pci_read (addr);
    }
    return 0;
}
