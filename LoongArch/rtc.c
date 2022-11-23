#include "acpi.h"


#define RTC_BASE_ADDR				0x50100

DevNode RtcInstance = {
    "rtc",
    NULL,
    LS7A_MISC_BASE_ADDR + RTC_BASE_ADDR,
    NULL,
    NULL
};


void RtcReadOps(DevNode *this,int fd)
{
    void * p = NULL;
    int status ;

    /*Transfer Virtul to Phy Addr*/
    p = vtpa(this->devaddr,fd);

    /*Debug Rtc*/
    printf("Rtc Reg Read Start ...\n");
    int i = 0;
    unsigned char j = 0;
    unsigned int tmp_tmp = 0;
#if 1
    unsigned char regbuf[] = {0x20,0x24,0x28,0x2c,0x30,0x34,0x38,0x3c,0x40,0x60,0x64,0x68,0x6c,0x70,0x74};
    unsigned char buflen = sizeof(regbuf);
    for(i = 0,j = 0; i < buflen; i++){
        j=regbuf[i];
        tmp_tmp = (*(volatile unsigned int *)(p + j));
        printf("RegNum:%x    RegVal:%x \n",j,tmp_tmp);
    }
#endif
    status = releaseMem(p);
}

void RtcWriteOps(DevNode *this,int fd)
{
    printf("-w Func\n");
}

Cmd RtcCmd[3] = {
    {"-r",RtcReadOps},
    {"-w",RtcWriteOps},
    {NULL,NULL}
};

void RtcInitInstance(void)
{
    RtcInstance.CmdInstance = RtcCmd;
    DevInstanceInsert(&RtcInstance);
}
