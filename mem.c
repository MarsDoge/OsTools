#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#define LS7A_MISC_BASE_ADDR		0x10080000
#define GPIO_BASE_ADDR_OFFSET 0x60000
#define ACPI_BASE_ADDR				0x50000
#define GPIO_CTL							0x900

#define GPIO_ON		1
#define GPIO_OFF	0

#define GPIO_TMP	0


#define FLAG_IO_R_MASK 0x70
#define FLAG_IO_W_MASK 0x40
int main(int argc,char *argv[]){
	int tmp = 0,flag = 0,status = 0;
	char * p = NULL;
	if(argc==1){
		printf("Access Func Fail, please get help !!! \n");
		return;
	}
	if(!strcmp(argv[1], "-r"))
	{
			flag |= FLAG_IO_R_MASK;
			printf("Flag: Read Reg Set Success... \n");
	}
	else if(!strcmp(argv[1], "-w"))
	{
		flag |= FLAG_IO_W_MASK;
		printf("Flag: Write Reg Set Success... \n");
	}
	else{
		printf("Flag: Access Reg Fail, please get help !!! \n");
		return;
	}

//------------------------

	printf("Please Input Reg Name:  ");
	char *string = calloc(1,20);
	scanf("%s",string);
	//printf("%s \n",string);

	if(!strcmp(string,"acpi"))
	{
		printf("ACPI RW_FUNC Support, please enter access ..\n");
	}
	else{
		printf("Function not currently supported, Please contact the developer !!!\n");
		return;
	}
#if 0
	//while(1);
	if(strcmp(argv[1], "acpi")==0)
	{
		tmp = GPIO_ON;
	}
	else if(strcmp(argv[1], "beep_off")==0)
		tmp = GPIO_OFF;
	/*else if(strcmp(argv[1], "beep_off")==0)
		tmp = GPIO_TMP;//update*/
	else{
		printf("Input param error ,please check ! \n");
		exit(1);
	}
#endif
	//getchar();
	printf("--------------start----------------\n");
	int fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0){
		printf("can't open file,please use root .\n");
		exit(1);
	}
	else{
		//printf("PAGE_SIZE : %d byte  \n",4096);
		switch(0x80){
			case 0x80:
				//tmp = 1;
release:
				p = (char*)mmap(NULL,4096, PROT_READ|PROT_WRITE,MAP_SHARED,fd, LS7A_MISC_BASE_ADDR+ACPI_BASE_ADDR);
				printf("mmap addr start : %p \n",p);
				//unsigned int *tmp_4 = (unsigned int *)p;
				//printf("------mmap addr start : %p \n",tmp_4);
				//printf("------mmap addr start : %p \n",tmp_4+1);
				//printf("------mmap addr start : %p \n",tmp_4+4);
				/*Debug ACPI*/
				int i = 0;
				unsigned int tmp_tmp = 0;
				for(i = 0; i< 0x50 ; i=i+4){
					tmp_tmp = (*(volatile unsigned int *)(p + i));
					printf("RegNum:%x    RegVal:%x \n",i,tmp_tmp);
				}

				/*Debug ACPI*/
				//(*(volatile int *)(0x90000e00100d0000 + 10))=0x0;
				//(*(volatile int *)(0x90000e00100d0000 + 14))=0x0;
				for(i=0;i<5;i++){
					tmp_tmp = (*(volatile unsigned int *)(p + 0xc));
					printf("AcpiStatus..  RegNum:0xc    RegVal:%x \n",tmp_tmp);
					(*(volatile unsigned int *)(p + 0xc)) |= tmp_tmp;
					tmp_tmp = (*(volatile unsigned int *)(p + 0x28));
					printf("AcpiStatus..  RegNum:0x28    RegVal:%x \n",tmp_tmp);
					(*(volatile unsigned int *)(p + 0x28)) |= tmp_tmp;
					printf("AcpiStatus.. Clear Event Status \n");
				}

				//printf("%#x\n",*(tmp_4+1));
				//printf("%#x\n",*(tmp_4+4));
				//*(p+0x900) = tmp;//beep test
				status = munmap(p, 4096);
				printf("--------------Release mem Map----------------\n");
				close(fd);
				//if(fd==-1)
				//printf("####### Release mem error ########\n ");
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

