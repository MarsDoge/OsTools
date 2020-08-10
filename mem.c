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
enum regfunc{
  ACPI,RTC
};

typedef void (*deffunc)(int fd);
typedef struct __funcstruct
{
  enum regfunc regflag;
  deffunc Func;
  char *regname;
  int setflag;

}funcstruct;

void drawfunclist(funcstruct funclist[],int size)
{
  int i,j;
	printf("############ Func Support : ");
  for(i = 0; i<size;i++)
  {
    printf("%s ",funclist[i].regname);
  }
  printf("############\n");

  for(i = 0; i<size;i++)
  {
    printf("%5s: flag=%d ,FuncSupportLoad=%p\n",funclist[i].regname,funclist[i].regflag,funclist[i].Func);
  }
  printf("############ Func List End ############\n");
}
void Func1Example(int fd);
void Func1Example(int fd)
{
	char * p = NULL;
  int status ;
	//tmp = 1;
	p = (char*)mmap(NULL,4096, PROT_READ|PROT_WRITE,MAP_SHARED,fd, LS7A_MISC_BASE_ADDR+ACPI_BASE_ADDR);
	printf("mmap addr start : %p \n",p);
	/*Debug ACPI*/
	int i = 0;
	unsigned int tmp_tmp = 0;
	for(i = 0; i< 0x50 ; i=i+4){
		tmp_tmp = (*(volatile unsigned int *)(p + i));
		printf("RegNum:%x    RegVal:%x \n",i,tmp_tmp);
	}

	/*Debug ACPI*/
	for(i=0;i<5;i++){
		tmp_tmp = (*(volatile unsigned int *)(p + 0xc));
		printf("AcpiStatus..  RegNum:0xc    RegVal:%x \n",tmp_tmp);
		(*(volatile unsigned int *)(p + 0xc)) |= tmp_tmp;
		tmp_tmp = (*(volatile unsigned int *)(p + 0x28));
		printf("AcpiStatus..  RegNum:0x28    RegVal:%x \n",tmp_tmp);
		(*(volatile unsigned int *)(p + 0x28)) |= tmp_tmp;
		printf("AcpiStatus.. Clear Event Status \n");
	}
		status = munmap(p, 4096);
    if(status == -1){
		  printf("----------  Release mem Map Error !!! ------\n");
    }
		printf("--------------Release mem Map----------------\n");
}
void Func2Example(int fd)
{
		printf("-------------Test 222----------------\n");
}
//deffunc Func1 = Func1Example;
// Tools func
#define FLAG_IO_R_MASK 0x80
#define FLAG_IO_W_MASK 0x40
#define FLAG_IO_SELECT_MASK 0x20

int main(int argc,char *argv[]){
	int tmp = 0,flag = 0,status = 0;
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

//-------Only Rw-----------------
  funcstruct funcSet[] = {{ACPI,Func1Example,"acpi",0},{RTC,Func2Example,"rtc",0}};
  //char *regname[]={"acpi","rtc"/*regfunc*/};
  enum regfunc funcselect;
  int size = sizeof(funcSet)/sizeof(funcstruct);
  /*draw Func LIST*/
  drawfunclist(funcSet,size);

	printf("Please Input Reg Name:  ");
	char *string = calloc(1,20);
	scanf("%s",string);
	//printf("%s \n",string);
    /* select reg name */
  int j = 0;
  for(j = 0;j< (sizeof(funcSet)/sizeof(funcstruct));j++)
  {
    if(!strcmp(string,funcSet[j].regname))
    {
      printf("%s RW_FUNC Support, please enter access ..\n",funcSet[j].regname);
      funcSet[j].setflag = 1;
      flag |= FLAG_IO_SELECT_MASK;
      break;
    }
  }

	printf("flag Read Value : %x ..\n",flag);
	if(!(flag&FLAG_IO_SELECT_MASK)){
		printf("Function not currently supported, Please contact the developer !!!\n");
		return;
	}

//-----------func  select------------------
	printf("--------------start----------------\n");
	int fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0){
		printf("can't open file,please use root .\n");
		exit(1);
	}
	else{
    for(j = 0;j<(sizeof(funcSet)/sizeof(funcstruct));j++){
      if(funcSet[j].setflag == 1)
      {
        printf("Find function succeeded :%s...\n",funcSet[j].regname);
        funcSet[j].Func(fd);
				break;
      }
      if((j == (sizeof(funcSet)/sizeof(funcstruct))) && funcSet[j].setflag != 0)
      {
         printf("####### Find function Fail , not such Func !!! ########\n ");
      }
		}

		printf("--------------end----------------\n");
	}

	return 0;
}

