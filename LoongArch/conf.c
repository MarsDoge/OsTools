#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"

#define REUSE_BASE_ADDR				0x440

static const char *const conf_usages[] = {
    PROGRAM_NAME" conf <args>",
    NULL,
};

static int conf_read (void)
{
    void * p = NULL;
    unsigned long long devaddr;
    int fd;
    int status ;

    devaddr = LS7A_CONF_BASE_ADDR + REUSE_BASE_ADDR;

    int memmask = devaddr & ~(0xfff);
    int memoffset = devaddr & (0xfff);

    fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    /*Transfer mem Addr*/
    p = (void*)mmap(NULL,1, PROT_READ|PROT_WRITE,MAP_SHARED,fd,memmask);
    p = p + memoffset;
    printfQ("mmap addr start : %p \n",p);
    /*Debug Rtc*/
    printfQ("Rtc Reg Read Start ...\n");
    int i = 0;
    unsigned char j = 0;
    unsigned int tmp_tmp = 0;

    tmp_tmp = *(volatile unsigned int *)p;
    *(volatile unsigned int *)(p) &= ~(1<<24);
    *(volatile unsigned int *)(p) &= ~(1<<20);
    tmp_tmp = *(volatile unsigned int *)p;
    printf(" %s %d gpio:%x \n",__func__,__LINE__,tmp_tmp);

    printf(" %s %d  \n",__func__,__LINE__);
    status = munmap(p-memoffset, 1);
    if(status == -1){
        printf("----------  Release mem Map Error !!! ------\n");
    }
    printf("--------------Release mem Map----------------\n");
}

int cmd_conf (int argc, const char **argv)
{
    int read = 0;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP (),
        OPT_BOOLEAN ('r', "read", &read, "read conf", NULL, 0, 0),
        OPT_END (),
    };

    argparse_init (&argparse, options, conf_usages, 0);
    argc = argparse_parse (&argparse, argc, argv);

    if (!read) {
        argparse_usage (&argparse);
        return 1;
    }

    uid = geteuid ();
    if (uid != 0) {
        printf ("Please run with root!\n");
        return -1;
    }

    if (read) {
        conf_read ();
    }
    return 0;
}
