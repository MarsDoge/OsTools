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
  char str[30] = {0};

  p = vtpa(this->devaddr,fd);

	printf("Read Pci Bar Mem Start, Please Input Address offset composed of Bus,dev,func: " );
  status = scanf("%s",str);
  //c = atoi(RecordName);
  int a = 0;
  sscanf (str,"%x",&a);
  printf("your input Pci Offset 0x%lx \n",a);
  
	printf(" Auto Read Pci(Bus:%d Device:%d Function:%d) 0x10 Bar Mem Start, 0x%lx \n",(a>>16),((a>>11)&0x1f),((a>>8)&7), *(volatile unsigned long *)(p + a + 0x10));
	printf(" Auto Read Pci(Bus:%d Device:%d Function:%d) 0x10 Bar Mem Start, 0x%lx \n",(a>>16),((a>>11)&0x1f),((a>>8)&7), *(volatile unsigned long *)(p + a + 0x4));
  //lack auto know device name array
}

Cmd PciCmd[2] = {
  {"-r",PciReadOps},
  {NULL,NULL}
};

void PciInitInstance(void)
{
   PciInstance.CmdInstance = PciCmd;
   DevInstanceInsert(&PciInstance);
}
