// SPDX-License-Identifier: GPL-2.0
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"

#define CHIPSET0_TEMP 0xe0010000414ULL

static const char *const temp_usages[] = {
    PROGRAM_NAME" temp <args>",
    NULL,
};

int chipset_temp_read (void)
{
    void * p = NULL;
    int status ;
    unsigned long long devaddr;

    int fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    devaddr = CHIPSET0_TEMP;

    /*Transfer Virtul to Phy Addr*/
    p = vtpa (devaddr, fd);

    /*Debug Rtc*/
    printf("Chipset Temp Read Start ...\n");
    unsigned int tmp_tmp = 0;

    tmp_tmp = *(volatile unsigned int *)p;
    printf ("Chipset0 Current Temp Val:%d \n", (tmp_tmp & 0xffff0000) >> 24);

    status = releaseMem(p);
    close(fd);
    return status;
}

int cmd_temp (int argc, const char **argv)
{
    int read = 0;
    int write = 0;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN ('r', "read", &read, "read Temp", NULL, 0, 0),
        OPT_BOOLEAN ('w', "write", &write, "write No", NULL, 0, 0),
        OPT_END(),
    };

    argparse_init(&argparse, options, temp_usages, 0);
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
        chipset_temp_read ();
    } else if (write) {
        //rtc_write ();
    }
    return 0;
}
