#include "acpi.h"


#define ACPI_BASE_ADDR				0x50000

DevNode AcpiInstance = {
    "acpi",
    NULL,
    LS7A_MISC_BASE_ADDR + ACPI_BASE_ADDR,
    NULL,
    NULL
};



void AcpiReadOps(int fd)
{
	char * p = NULL;
  int status ;

	p = (char*)mmap(NULL,4096, PROT_READ|PROT_WRITE,MAP_SHARED,fd,AcpiInstance.devaddr);
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

Cmd AcpiCmd[2] = {
  {"-r",AcpiReadOps},
  {NULL,NULL}
};

void AcpiInitInstance(void)
{
   AcpiInstance.CmdInstance = AcpiCmd;
   DevInstanceInsert(&AcpiInstance);
}
