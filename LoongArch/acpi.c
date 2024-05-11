// SPDX-License-Identifier: GPL-2.0

#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"

#define ACPI_BASE_ADDR				0x50000
#define writeb(addr,val)  (*(volatile unsigned char *)(addr)) = val

static const char *const acpi_usages[] = {
    PROGRAM_NAME" acpi <args>",
    NULL,
};

static int acpi_read (void)
{
    char * p = NULL;
    unsigned long long devaddr;
    int status ;
    int fd;

    fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    devaddr = LS7A_MISC_BASE_ADDR + ACPI_BASE_ADDR;

    int memmask = devaddr & ~(0xfff);
    int memoffset = devaddr & (0xfff);

    /*Transfer mem Addr*/
    p = (void*)mmap (NULL,1, PROT_READ|PROT_WRITE, MAP_SHARED, fd, memmask);
    p = p + memoffset;
    printf("mmap addr start : %p \n",p);
    /*Debug ACPI*/
    int i = 0;
    unsigned int tmp_tmp = 0;
    for(i = 0; i< 0x50 ; i=i+4) {
        tmp_tmp = (*(volatile unsigned int *)(p + i));
        printf("RegNum:%x    RegVal:%x \n", i, tmp_tmp);
    }

    /*Debug ACPI*/
    for(i=0;i<5;i++){
        tmp_tmp = (*(volatile unsigned int *)(p + 0xc));
        printf("AcpiStatus..  RegNum:0xc    RegVal:%x \n",tmp_tmp);
        (*(volatile unsigned int *)(p + 0xc)) |= tmp_tmp;
        tmp_tmp = (*(volatile unsigned int *)(p + 0x28));
        printf("AcpiStatus..  RegNum:0x28    RegVal:%x \n",tmp_tmp);
        (*(volatile unsigned int *)(p + 0x28)) |= tmp_tmp;
        printf("AcpiStatus.. Clear Event Status \n");
    }
    status = munmap(p, 4096);
    if(status == -1){
        printf("----------  Release mem Map Error !!! ------\n");
    }
    printf("--------------Release mem Map----------------\n");
    return status;
}

int acpi_reboot (void)
{
    char * p = NULL;
    unsigned long long devaddr;
    int status;
    int fd;

    fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    devaddr = LS7A_MISC_BASE_ADDR + ACPI_BASE_ADDR;

    p = vtpa(devaddr, fd);

    writeb (p + 0x30, 0x1);

    status = releaseMem(p);

    return status;
}

int cmd_acpi (int argc, const char **argv)
{
    int read = 0;
    int reboot = 0;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN ('r', "read", &read, "read acpi", NULL, 0, 0),
        OPT_BOOLEAN ('b', "reboot", &reboot, "reboot", NULL, 0, 0),
        OPT_END(),
    };

    argparse_init(&argparse, options, acpi_usages, 0);
    argc = argparse_parse(&argparse, argc, argv);

    if (!(read || reboot)) {
        argparse_usage(&argparse);
        return 1;
    }

    uid = geteuid ();
    if (uid != 0) {
        printf("Please run with root!\n");
        return -1;
    }

    if (read) {
        acpi_read ();
    } else if (write) {
        acpi_reboot ();
    }
    return 0;
}
