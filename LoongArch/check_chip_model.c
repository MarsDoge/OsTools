// SPDX-License-Identifier: GPL-2.0
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"

#define REG_3C6000_BA_DIFF 0x1fe00470ULL

static const char *const check_model_usages[] = {
    PROGRAM_NAME" check_model <args>",
    NULL,
};

int check_model_read (void)
{
    void * p = NULL;
    int status ;
    unsigned long long devaddr;

    int fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    devaddr = REG_3C6000_BA_DIFF;

    /*Transfer Virtul to Phy Addr*/
    p = vtpa (devaddr, fd);

    printf("Check Chip Model Start ...\n");
    unsigned long long tmp_tmp = 0;

    tmp_tmp = *(volatile unsigned long long *)p;
    printf ("Reg470: %lx :%lx \n",tmp_tmp, (tmp_tmp & (0xfULL << 57)));
    if (((tmp_tmp & (0xfULL << 57)) >> 57) == 4) {
      printf ("This is a \033[32m BA version \033[0m, it's right. \n");
    } else {
      printf ("Warning, This is a \033[31m AB version \033[0m, please check !\n");
    }

    status = releaseMem(p);
    close(fd);
    return status;
}

int cmd_chip_model (int argc, const char **argv)
{
    int read = 0;
    int write = 0;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN ('r', "read", &read, "read model", NULL, 0, 0),
        OPT_END(),
    };

    argparse_init(&argparse, options, check_model_usages, 0);
    argc = argparse_parse(&argparse, argc, argv);

    if (!read) {
        argparse_usage(&argparse);
        return 1;
    }

    uid = geteuid ();
    if (uid != 0) {
        printf("Please run with root!\n");
        return -1;
    }

    if (read) {
        check_model_read ();
    }
    return 0;
}
