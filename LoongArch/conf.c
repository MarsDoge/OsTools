#include "acpi.h"


#define REUSE_BASE_ADDR				0x440

DevNode ConfInstance = {
    "conf",
    NULL,
    LS7A_CONF_BASE_ADDR + REUSE_BASE_ADDR,
    NULL,
    NULL
};



void ReuseSetOps(DevNode *this, int fd)
{
    void * p = NULL;
    int status ;
    int memmask = ConfInstance.devaddr & ~(0xfff);
    int memoffset = ConfInstance.devaddr & (0xfff);
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

Cmd ConfCmd[2] = {
    {"-r",ReuseSetOps},
    {NULL,NULL}
};

void ConfInitInstance(void)
{
    ConfInstance.CmdInstance = ConfCmd;
    DevInstanceInsert(&ConfInstance);
}
