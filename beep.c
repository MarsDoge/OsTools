#include <stdio.h>
#include <sys/mman.h>
#define LS7A_MISC_BASE_ADDR		0x10080000
#define ACPI_BASE_ADDR				0x50000

#define GPIO_BASE_ADDR_OFFSET 0x60000
#define GPIO_CTL							0x900
void Func3Example(int fd)
{
	char * p = NULL;
  int status ;
  p = (char*)mmap(NULL,4096, PROT_READ|PROT_WRITE,MAP_SHARED,fd, LS7A_MISC_BASE_ADDR+GPIO_BASE_ADDR_OFFSET);
	printf("mmap addr start : %p \n",p);
#if 0
        int a= 0;
				int c = atoi(argv[2]);
				while(1){
					a = c;
					*(p+0x900) = tmp;//beep test
					while(a--);
					a = c;
					*(p+0x900) = !tmp;//beep test
					while(a--);
				}
#endif
		printf("----------  Beep Func Test ----------\n");
		status = (*(volatile char*)(p+0x900)) ;;//beep test
    status ^= 1;
    (*(volatile char*)(p+0x900)) = status;
    if(!(status & 1)){
		  printf("----------  BeppFlag = 0;Gpio0,Bit0=0; BeepOn:%d------\n",status);
    }else{
		  printf("----------  BeppFlag = 0;Gpio0,Bit0=0; BeepOff:%d------\n",status);
    }
    status = munmap(p, 4096);
    if(status == -1){
		  printf("----------  Release mem Map Error !!! ------\n");
    }
		printf("--------------Release mem Map----------------\n");


}
