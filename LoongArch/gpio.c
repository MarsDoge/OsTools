// SPDX-License-Identifier: GPL-2.0
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"

#define GPIO_BASE_ADDR				0x60000

static const char *const gpio_usages[] = {
    PROGRAM_NAME" gpio <args>",
    NULL,
};

void delay (unsigned long a)
{
    while(a--);
}

int gpio_read (void)
{
    void * p = NULL;
    int fd;
    unsigned long long devaddr;
    int status;

    devaddr = LS7A_MISC_BASE_ADDR + GPIO_BASE_ADDR + 0x900;

    int memmask = devaddr & ~(0xfff);
    int memoffset = devaddr & (0xfff);

    printf(" %s %d  ,%x\n",__func__,__LINE__,memmask);
    printf(" %s %d  ,%x\n",__func__,__LINE__,memoffset);

    fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    /*Transfer mem Addr*/
    p = (void*)mmap(NULL,4096, PROT_READ|PROT_WRITE,MAP_SHARED,fd,memmask);
    p = p + memoffset;
    printf("mmap addr start : %p \n",p);
    /*Debug Gpio*/
    printf("Gpio Reg Read Start ...\n");
    int i = 0;
    unsigned char j = 0;
    unsigned int tmp_tmp = 0;
    printf(" %s %d  \n",__func__,__LINE__);

    /*Set Gpio*/
    *(volatile unsigned int *)(p + 4) = 0x0;
    int a = 0x1000;
    while(1){
        //start
        a = 0x1000;
        while(a--){
            *(volatile unsigned int *)(p ) = 0xffffff;
            delay(0x20000);
            *(volatile unsigned int *)(p ) = 0x0;
            delay(0x20000);
        }
        sleep(1);
    }
    //printf(" %s %d gpio:%x \n",__func__,__LINE__,tmp_tmp);
    printf("---------------GPio Set %s %d  \n",__func__,__LINE__);

    status = munmap(p-memoffset, 4096);
    if(status == -1){
        printf("----------  Release mem Map Error !!! ------\n");
    }
    printf("--------------Release mem Map----------------\n");
    return status;
}

int cmd_gpio (int argc, const char **argv)
{
    int read = 0;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN ('r', "read", &read, "read gpio", NULL, 0, 0),
        OPT_END(),
    };

    argparse_init(&argparse, options, gpio_usages, 0);
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
        gpio_read ();
    }
    return 0;
}
