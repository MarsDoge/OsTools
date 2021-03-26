#include "acpi.h"

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000
#define PCI_CONFUSE_SPACE 0x0efdfe000000

DevNode PciInstance = {
    "pci",
    NULL,
    PCI_CONFUSE_SPACE,
    NULL,
    NULL
};



void PciReadOps(DevNode *this, int fd)
{
	void * p = NULL;
  int status ;
  char RecordName[30] = {0};
  int c = 0;

  p = vtpa(this->devaddr,fd);

	printf("Read Pci Bar Mem Start, Please Input Address offset composed of Bus,dev,func!" );
  status = scanf("%s",RecordName);
  c = atoi(RecordName);
  
	printf(" Auto Read Pci(Bus:%lx:%lx:%lx) 0x10 Bar Mem Start, %lx \n",(c>>16),((c>>11)&0x1f),((c>>8)&7), *(volatile unsigned int *)(p + c + 0x10));
  //lack auto know device name array

		status = munmap(p-memoffset, 1);
    if(status == -1){
		  printf("----------  Release mem Map Error !!! ------\n");
    }
		printf("--------------Release mem Map----------------\n");
}

Cmd PciCmd[2] = {
  {"-r",PciReadOps},
  {NULL,NULL}
};

void PciInitInstance(void)
{
   PciInstance.PciInstance = PciCmd;
   DevInstanceInsert(&PciInstance);
}
