#include <unistd.h>
#include <stdio.h>
#include "def.h"

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000
#define CPU_HT0_LO 0xefdfa000000ULL
#define CPU_HT0_HI 0xefdfb000000ULL
#define CPU_HT1_LO 0xefdfe000000ULL
#define CPU_HT1_HI 0xefdff000000ULL

DevNode Node0_HT0_LO_Instance = {
    "ht0_lo",
    NULL,
    CPU_HT0_LO,
    NULL,
    NULL
};
DevNode Node0_HT0_HI_Instance = {
    "ht0_hi",
    NULL,
    CPU_HT0_HI,
    NULL,
    NULL
};
DevNode Node0_HT1_LO_Instance = {
    "ht1_lo",
    NULL,
    CPU_HT1_LO,
    NULL,
    NULL
};
DevNode Node0_HT1_HI_Instance = {
    "ht1_hi",
    NULL,
    CPU_HT1_HI,
    NULL,
    NULL
};






void HtReadOps(DevNode *this, int fd)
{
    /*Save Input History Record*/
    FILE *pfile = fopen("./ToolRecord.txt", "a+");
    void *p = NULL;
    unsigned long long node_id = 0;
    unsigned long long vaddr = 0;
    unsigned int c = 0;

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
    }
    close(fd);
}

Cmd HtCmd[2] = {
    {"-r",HtReadOps},
    {NULL,NULL}
};

void HtInstance(void)
{
    Node0_HT0_LO_Instance.CmdInstance = HtCmd;
    DevInstanceInsert(&Node0_HT0_LO_Instance);

    Node0_HT0_HI_Instance.CmdInstance = HtCmd;
    DevInstanceInsert(&Node0_HT0_HI_Instance);

    Node0_HT1_LO_Instance.CmdInstance = HtCmd;
    DevInstanceInsert(&Node0_HT1_LO_Instance);

    Node0_HT1_HI_Instance.CmdInstance = HtCmd;
    DevInstanceInsert(&Node0_HT1_HI_Instance);

}





