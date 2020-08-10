#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
int atoi16(const char *str){
	if(*str!='0'){
		printf("please input correct physical addr : 0x00\n");
		exit(1);
	}
	const char *tmp = str+2;
	int flag = 8;
	while(!flag)
	{
		flag--;
		while(1){
		//	if(*tmp<=0x39)
		//		*tmp-0x30<<;
		}
	}
}
/*support physcial addr access*/
int main(int argc,char *argv[]){
	int tmp = 0,flag = 0,status = 0;
	int num = 0,conv = 0;
	unsigned long long addr = 0 ;
	char * p = NULL;
	num = atoi(argv[2]);
	addr = atoll(argv[1]);
	//addr = addr/10*16+addr%10;
	printf("conv 10/16 change: %lld \n",addr);
	tmp = num;
	printf("--------------start----------------\n");
	int fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0){
		printf("can't open file,please use root .\n");
		exit(1);
	}
	else{
				p = (char*)mmap(NULL,4096, PROT_READ|PROT_WRITE,MAP_SHARED,fd, addr);
				printf("mmap addr start : %p \n",p);
				while(num){
					printf("Byte %d:%#x  ",tmp-num,*p);
					num--;
		}
		printf("\n");
		status = munmap(p, 4096);
		//if(fd==-1)
			//printf("####### Release mem error ########\n ");
	}
		printf("--------------end----------------\n");

	return 0;
}

