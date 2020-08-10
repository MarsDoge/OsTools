#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#define LS7A_MISC_BASE_ADDR		0x10080000
#define GPIO_BASE_ADDR_OFFSET 0x60000
#define GPIO_CTL							0x900
#define GPIO_ON		1
#define GPIO_OFF	0

#define GPIO_TMP	0
int main(int argc,char *argv[]){
	int tmp = 0,flag = 0,status = 0;
	char * p = NULL;
	//printf("%p\n",argv[1]);
	//printf("%p\n","beep");
	if(strcmp(argv[1], "beep_on")==0)
		tmp = GPIO_ON;
	else if(strcmp(argv[1], "beep_off")==0)
		tmp = GPIO_OFF;
	/*else if(strcmp(argv[1], "beep_off")==0)
		tmp = GPIO_TMP;//update*/
	else{
		printf("Input param error ,please check ! \n");
		exit(1);
	}
	printf("--------------start----------------\n");
	int fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0){
		printf("can't open file,please use root .\n");
		exit(1);
	}
	else{
		//printf("PAGE_SIZE : %d byte  \n",4096);
		switch(tmp){
			case GPIO_ON:
				//tmp = 1;
release:
				p = (char*)mmap(NULL,4096, PROT_READ|PROT_WRITE,MAP_SHARED,fd, LS7A_MISC_BASE_ADDR+GPIO_BASE_ADDR_OFFSET);
				printf("mmap addr start : %p \n",p);
				printf("%d\n",*(p+GPIO_CTL));
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
				status = munmap(p, 4096);
				if(fd==-1)
				printf("####### Release mem error ########\n ");
				break;
			case GPIO_OFF:
				//flag = 0;
				goto release;
			//case GPIO_TMP:
				//function logical
			default:
				printf("####### Not param function ########\n ");
				break;
		}

		printf("--------------end----------------\n");
	}

	return 0;
}

