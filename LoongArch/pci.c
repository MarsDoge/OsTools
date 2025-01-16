// SPDX-License-Identifier: GPL-2.0
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000
#define PCI_EX_CONFUSE_SPACE_TYPE0 0x0efe00000000
#define PCI_EX_CONFUSE_SPACE_TYPE1 0x0efe10000000

static const char *const pci_usages[] = {
    PROGRAM_NAME" pci <args>",
    NULL,
};

static int pci_read (const char* addr, int port, int pcie_type_bus, int bus_id)
{
    void * p = NULL;
    int status ;
    char RecordName[30] = {0};
    int c = 0;
    char str[30] = {0};
    unsigned long long devaddr;
    int fd;

    if (pcie_type_bus) {
	devaddr = PCI_EX_CONFUSE_SPACE_TYPE1;
    } else {
	devaddr = PCI_EX_CONFUSE_SPACE_TYPE0;
    }
    devaddr |= bus_id << 16;
    devaddr |= port << 11;

    printf ("devaddr map phyaddress: %lx \n", devaddr);

    int a = 0;
    sscanf (addr, "%x", &a);
 
    if (a >= 0x100) {
	    devaddr |= ((a & 0xf00) >> 8) << 24;
    }
    a = a & 0xff;

    printf ("devaddr map phyaddress offset: %lx \n", a);

    fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    p = vtpa(devaddr,fd);

    printf("Get Pcie value %#x \n",*(unsigned int *)(p + a));

    //lack auto know device name array
    return 0;
}

int cmd_pci (int argc, const char **argv)
{
    int read = 0;
    const char *addr = NULL;
    uid_t uid;
    struct argparse argparse;
    int port = -1; //default 0xff invalid
    int pcie_type_bus = 0;
    int bus_id = 0;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN ('r', "read", &read, "Read Pci Bar Mem Start", NULL, 0, 0),
        OPT_STRING  ('a', "address", &addr, "Address offset composed of Bus", NULL, 0, 0),
        OPT_INTEGER ('p', "port", &port, "pcie device port id", NULL, 0, 0),
        OPT_BOOLEAN ('e', "pcie_type_bus", &pcie_type_bus, "EX Space", NULL, 0, 0),
        OPT_INTEGER ('b', "bus", &bus_id, "bus id", NULL, 0, 0),
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
        if (port == -1) {
            printf ("Please setup the device port.\n");
            return 1;
        }
        pci_read (addr, port, pcie_type_bus, bus_id);
    }
    return 0;
}
