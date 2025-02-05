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

static unsigned int pci_read (const char* addr, int port, int pcie_type_bus, int bus_id, unsigned long long bridge_id)
{
    void * p = NULL;
    int status ;
    char RecordName[30] = {0};
    int c = 0;
    char str[30] = {0};
    unsigned long long devaddr;
    int fd;
    unsigned int value = -1;

    if (pcie_type_bus) {
	devaddr = PCI_EX_CONFUSE_SPACE_TYPE1;
    } else {
	devaddr = PCI_EX_CONFUSE_SPACE_TYPE0;
    }
    devaddr |= bridge_id << 44;
    devaddr |= bus_id << 16;
    devaddr |= port << 11;

    printf ("devaddr map phyaddress: %lx \n", devaddr);

    int a = 0;
    sscanf (addr, "%x", &a);
 
    if (a >= 0x100) {
	    devaddr |= ((a & 0xf00) >> 8) << 24;
    }
    a = a & 0xff;

    fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    p = vtpa(devaddr,fd);

    printf ("devaddr map phyaddress offset: %lx \n", devaddr + a);
    value = *(unsigned int *)(p + a);
    printf("Get Pcie value %#x \n", value);

    //lack auto know device name array
    return value;
}

static unsigned int pci_check_replay (int slot_id)
{
	unsigned long long addr;
	unsigned long long value;
	int fd;
    	void * p = NULL;
	int port = 0;
	int bus_id = 0;
	int pcie_type_bus = 0;
	unsigned long long bridge_id = 0;

	switch(slot_id) {
		case 0:
			port = 0;
			bus_id = 0xc;
			pcie_type_bus = 1;
			//printf("你输入了数字 1\n");
			break;
		case 1:
			port = 1;
			bus_id = 0xc;
			pcie_type_bus = 1;
			break;
		case 2:
			port = 2;
			bus_id = 0xc;
			pcie_type_bus = 1;
			break;
		case 3:
			port = 0;
			bus_id = 0x0;
			pcie_type_bus = 0;
			bridge_id = 1;
			//printf("你输入了数字 1\n");
			break;
		case 4:
			port = 1;
			bus_id = 0x2;
			pcie_type_bus = 1;
			bridge_id = 1;
			break;
		case 5:
			port = 1;
			bus_id = 0x5;
			pcie_type_bus = 1;
			bridge_id = 1;
			break;
		case 6:
			port = 3;
			bus_id = 0x5;
			pcie_type_bus = 1;
			bridge_id = 1;
			break;
		case 7:
			bridge_id = 2;
			bus_id = 0x2;
			port = 2;
			pcie_type_bus = 0;
			break;
		case 8:
			bridge_id = 2;
			bus_id = 0x3;
			port = 1;
			pcie_type_bus = 1;
			break;
		case 9:
			bridge_id = 3;
			bus_id = 0x0;
			port = 0;
			pcie_type_bus = 0;
			break;
		case 10:
			bridge_id = 3;
			bus_id = 0x3;
			port = 1;
			pcie_type_bus = 1;
			break;

		default:
			printf("invalid slot_id, please check it! \n");
			return -1;
	}

	addr = pci_read ("0x14", port, pcie_type_bus, bus_id, bridge_id);
	addr = addr << 32;
	addr |= pci_read ("0x10", port, pcie_type_bus, bus_id, bridge_id) & 0xfffffff0;
	addr |= 0xc4;
	addr |= bridge_id << 44;
	printf ("physical address:%lx \n ", addr);

	fd = open("/dev/mem", O_RDWR|O_SYNC);
	if (fd < 0) {
		printf("can't open file,please use root .\n");
		return 1;
	}

	p = vtpa(addr,fd);
	value = *(unsigned int *) p;
	printf ("address:%lx : value:%lx \n ", addr, value);

}

int cmd_pci (int argc, const char **argv)
{
    int read = 0;
    int check_replay = 0;
    int slot_id = 0;
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
        OPT_BOOLEAN ('c', "check_replay", &check_replay, "Check replay", NULL, 0, 0),
        OPT_INTEGER ('s', "slot_id", &slot_id, "pcie slot id", NULL, 0, 0),
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
	    if (check_replay) {
		    pci_check_replay(slot_id);
		    return 0;
	    }
	    if (port == -1) {
		    printf ("Please setup the device port.\n");
		    return 1;
	    }
	    pci_read (addr, port, pcie_type_bus, bus_id, 0);
    }
    return 0;
}
