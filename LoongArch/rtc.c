// SPDX-License-Identifier: GPL-2.0
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"

#define RTC_BASE_ADDR				0x50100

static const char *const rtc_usages[] = {
    PROGRAM_NAME" rtc <args>",
    NULL,
};

int rtc_read (void)
{
    void * p = NULL;
    int status ;
    unsigned long long devaddr;

    int fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    devaddr = LS7A_MISC_BASE_ADDR + RTC_BASE_ADDR;

    /*Transfer Virtul to Phy Addr*/
    p = vtpa (devaddr, fd);

    /*Debug Rtc*/
    printf("Rtc Reg Read Start ...\n");
    int i = 0;
    unsigned char j = 0;
    unsigned int tmp_tmp = 0;
#if 1
    unsigned char regbuf[] = {0x20,0x24,0x28,0x2c,0x30,0x34,0x38,0x3c,0x40,0x60,0x64,0x68,0x6c,0x70,0x74};
    unsigned char buflen = sizeof(regbuf);
    for(i = 0, j = 0; i < buflen; i++){
        j = regbuf[i];
        tmp_tmp = (*(volatile unsigned int *)(p + j));
        printf ("RegNum:%x    RegVal:%x \n", j, tmp_tmp);
    }
#endif
    status = releaseMem(p);
    close(fd);
    return status;
}

int rtc_write (void)
{
    printf("-w Func\n");
    return 0;
}

int cmd_rtc (int argc, const char **argv)
{
    int read = 0;
    int write = 0;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN ('r', "read", &read, "read rtc", NULL, 0, 0),
        OPT_BOOLEAN ('w', "write", &write, "write rtc", NULL, 0, 0),
        OPT_END(),
    };

    argparse_init(&argparse, options, rtc_usages, 0);
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
        rtc_read ();
    } else if (write) {
        rtc_write ();
    }
    return 0;
}
