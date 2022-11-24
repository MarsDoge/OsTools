#include <unistd.h>
#include <stdio.h>
#include "def.h"

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000
#define CPU_HT0_LO 0xefdfa000000ULL
#define CPU_HT0_HI 0xefdfb000000ULL
#define CPU_HT1_LO 0xefdfe000000ULL
#define CPU_HT1_HI 0xefdff000000ULL

static const char *const ht_usages[] = {
    PROGRAM_NAME" ht <args>",
    NULL,
};

static int ht_read (void)
{
    /*Save Input History Record*/
    FILE *pfile = fopen("./ToolRecord.txt", "a+");
    void *p = NULL;
    unsigned long long node_id = 0;
    unsigned long long vaddr = 0;
    unsigned int c = 0;
    int fd;

    fd = open ("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    while (1) {
        vaddr = CPU_HT0_LO | (node_id <<44);
        p = (void*)mmap(NULL,0x10000/*64K*/, PROT_READ|PROT_WRITE,MAP_SHARED,fd,vaddr);
        c =  *(volatile unsigned int *)(p + 0x68); //retry
        fwrite( (void *)&c, sizeof(unsigned int),1,pfile);
        munmap (p,0x10000);

        vaddr = CPU_HT0_HI | (node_id <<44);
        p = (void*)mmap(NULL,0x10000/*64K*/, PROT_READ|PROT_WRITE,MAP_SHARED,fd,vaddr);
        c =  *(volatile unsigned int *)(p + 0x68); //retry
        fwrite( (void *)&c, sizeof(unsigned int),1,pfile);
        munmap (p,0x10000);

        vaddr = CPU_HT1_LO | (node_id <<44);
        p = (void*)mmap(NULL,0x10000/*64K*/, PROT_READ|PROT_WRITE,MAP_SHARED,fd,vaddr);
        c =  *(volatile unsigned int *)(p + 0x68); //retry
        fwrite( (void *)&c, sizeof(unsigned int),1,pfile);
        munmap (p,0x10000);

        vaddr = CPU_HT1_HI | (node_id <<44);
        p = (void*)mmap(NULL,0x10000/*64K*/, PROT_READ|PROT_WRITE,MAP_SHARED,fd,vaddr);
        c =  *(volatile unsigned int *)(p + 0x68); //retry
        fwrite( (void *)&c, sizeof(unsigned int),1,pfile);
        munmap (p,0x10000);

        node_id ++;
        if (node_id == 8) {
            node_id = 0;
        }
        break;
    }
    close(fd);
    return 0;
}

int cmd_ht (int argc, const char **argv)
{
    int read = 0;
    int reboot = 0;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN ('r', "read", &read, "read ht", NULL, 0, 0),
        OPT_END(),
    };

    argparse_init(&argparse, options, ht_usages, 0);
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
        ht_read ();
    }
    return 0;
}
