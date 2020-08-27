#include "acpi.h"


#define REUSE_BASE_ADDR				0x440

DevNode ConfInstance = {
    "rtc",
    NULL,
    LS7A_CONF_BASE_ADDR + REUSE_BASE_ADDR,
    NULL,
    NULL
};



void ReuseSetOps(int fd)
{
	void * p = NULL;
  int status ;
	int memmask = ConfInstance.devaddr & ~(0xfff);
	int memoffset = ConfInstance.devaddr & (0xfff);
	/*Transfer mem Addr*/
printf(" %s %d  ,%x\n",__func__,__LINE__,memmask);
printf(" %s %d  ,%x\n",__func__,__LINE__,memoffset);
	p = (void*)mmap(NULL,1, PROT_READ|PROT_WRITE,MAP_SHARED,fd,memmask);
	p = p + memoffset;
	printf("mmap addr start : %p \n",p);
	/*Debug Rtc*/
	printf("Rtc Reg Read Start ...\n");
	int i = 0;
	unsigned char j = 0;
	unsigned int tmp_tmp = 0;
printf(" %s %d  \n",__func__,__LINE__);

	tmp_tmp = *(volatile unsigned int *)p;
	printf(" %s %d gpio:%x \n",__func__,__LINE__,tmp_tmp);
	*(volatile unsigned int *)(p) &= ~(1<<24);
	*(volatile unsigned int *)(p) &= ~(1<<20);
	tmp_tmp = *(volatile unsigned int *)p;
	printf(" %s %d gpio:%x \n",__func__,__LINE__,tmp_tmp);

	printf(" %s %d  \n",__func__,__LINE__);
		status = munmap(p-memoffset, 1);
printf(" %s %d  \n",__func__,__LINE__);
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
printf(" %s %d  \n",__func__,__LINE__);
   ConfInstance.CmdInstance = ConfCmd;
printf(" %s %d  \n",__func__,__LINE__);
   DevInstanceInsert(&ConfInstance);
printf(" %s %d  \n",__func__,__LINE__);
}
