#include "acpi.h"


#define ACPI_BASE_ADDR				0x50000

DevNode AcpiInstance = {
    "acpi",
    NULL,
    LS7A_MISC_BASE_ADDR + ACPI_BASE_ADDR,
    NULL,
    NULL
};



void AcpiReadOps(DevNode *this,int fd)
{
	char * p = NULL;
  int status ;
	int memmask = this->devaddr & ~(0xfff);
	int memoffset = this->devaddr & (0xfff);
	/*Transfer mem Addr*/
	p = (void*)mmap(NULL,1, PROT_READ|PROT_WRITE,MAP_SHARED,fd,memmask);
	p = p + memoffset;
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
#define writeb(addr,val)  (*(volatile unsigned char *)(addr)) = val
void AcpiReboot(DevNode *this,int fd)
{
  char * p = NULL;
  int status ;
  p = vtpa(this->devaddr,fd);

  writeb (p + 0x30, 0x1);

  status = releaseMem(p);
}

Cmd AcpiCmd[3] = {
  {"-r",AcpiReadOps},
  {"-b",AcpiReboot},
  {NULL,NULL} //Leave blank address
};

void AcpiInitInstance(void)
{
   AcpiInstance.CmdInstance = AcpiCmd;
   DevInstanceInsert(&AcpiInstance);
}
