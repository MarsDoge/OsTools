#include "acpi.h"


#define GPIO_BASE_ADDR				0x60000

DevNode GpioInstance = {
    "gpio",
    NULL,
    LS7A_MISC_BASE_ADDR + GPIO_BASE_ADDR + 0x900,
    NULL,
    NULL
};

void delay (unsigned long a)
{
    while(a--);
}

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
