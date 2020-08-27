#include "acpi.h"


#define GPIO_BASE_ADDR				0x60000

DevNode GpioInstance = {
    "gpio",
    NULL,
    LS7A_MISC_BASE_ADDR + GPIO_BASE_ADDR,
    NULL,
    NULL
};



void GpioReadOps(DevNode *this,int fd)
{
	void * p = NULL;
  int status ;
	int memmask = this->devaddr & ~(0xfff);
	int memoffset = this->devaddr & (0xfff);
	/*Transfer mem Addr*/
printf(" %s %d  ,%x\n",__func__,__LINE__,memmask);
printf(" %s %d  ,%x\n",__func__,__LINE__,memoffset);
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
while(1){

	//start
	*(volatile unsigned int *)(p + 0x10 + 4) = 0xffffffff;
	sleep(1);//500ms
	//getchar();
	*(volatile unsigned int *)(p + 0x10 + 4) = 0x0;
	sleep(1);//500ms
	//getchar();
	*(volatile unsigned int *)(p + 0x10 + 4) = 0xffffffff;
	//*(volatile unsigned int *)(p + 0x10 + 4) = 0x0;
	//sleep(5);
	//getchar();
	sleep(100);
	*(volatile unsigned int *)(p + 0x10 + 4) = 0x0;
	sleep(5);
	//restore High
	*(volatile unsigned int *)(p + 0x10 + 4) |= 0xffffffff;
	printf(" cycle : %d \n",++i);
	//sleep(6);
}
//printf(" %s %d gpio:%x \n",__func__,__LINE__,tmp_tmp);
	printf("---------------GPio Set %s %d  \n",__func__,__LINE__);

		status = munmap(p-memoffset, 4096);
    if(status == -1){
		  printf("----------  Release mem Map Error !!! ------\n");
    }
		printf("--------------Release mem Map----------------\n");
}

Cmd GpioCmd[2] = {
  {"-r",GpioReadOps},
  {NULL,NULL}
};

void GpioInitInstance(void)
{
   GpioInstance.CmdInstance = GpioCmd;
   DevInstanceInsert(&GpioInstance);
}
