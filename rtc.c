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
	int memmask = this->devaddr & ~(0xfff);
	int memoffset = this->devaddr & (0xfff);
	/*Transfer mem Addr*/
	p = (void*)mmap(NULL,1, PROT_READ|PROT_WRITE,MAP_SHARED,fd,memmask);
	p = p + memoffset;
	printf("mmap addr start : %p \n",p);
	/*Debug Rtc*/
	printf("Rtc Reg Read Start ...\n");
	int i = 0;
	unsigned char j = 0;
	unsigned int tmp_tmp = 0;
printfQ(" %s %d  \n",__func__,__LINE__);
#if 1
	unsigned char regbuf[] = {0x20,0x24,0x28,0x2c,0x30,0x34,0x38,0x3c,0x40,0x60,0x64,0x68,0x6c,0x70,0x74};
	unsigned char buflen = sizeof(regbuf);
printfQ(" %s %d buflen:%d  \n",__func__,__LINE__,buflen);
	for(i = 0,j = 0; i < buflen; i++){
		j=regbuf[i];
		tmp_tmp = (*(volatile unsigned int *)(p + j));
		printf("RegNum:%x    RegVal:%x \n",j,tmp_tmp);
	}
#endif
	printfQ(" %s %d  \n",__func__,__LINE__);
		status = munmap(p-memoffset, 1);
printfQ(" %s %d  \n",__func__,__LINE__);
    if(status == -1){
		  printf("----------  Release mem Map Error !!! ------\n");
    }
		printf("--------------Release mem Map----------------\n");
}

Cmd RtcCmd[2] = {
  {"-r",RtcReadOps},
  {NULL,NULL}
};

void RtcInitInstance(void)
{
printfQ(" %s %d  \n",__func__,__LINE__);
   RtcInstance.CmdInstance = RtcCmd;
printfQ(" %s %d  \n",__func__,__LINE__);
   DevInstanceInsert(&RtcInstance);
printfQ(" %s %d  \n",__func__,__LINE__);
}
