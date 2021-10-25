#include "acpi.h"

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000
#define CPU_I2C0 0x1fe00120
#define NODE0_CPU_I2C0 0x1fe00120
#define NODE4_CPU_I2C0 0x40001fe00120ULL
#define NODE8_CPU_I2C0 0x80001fe00120ULL
#define NODE12_CPU_I2C0 0xC0001fe00120ULL

DevNode Node0SpdInstance = {
    "spd0",
    NULL,
    NODE0_CPU_I2C0,
    NULL,
    NULL
};

DevNode Node4SpdInstance = {
    "spd1",
    NULL,
    NODE4_CPU_I2C0,
    NULL,
    NULL
};

DevNode Node8SpdInstance = {
    "spd2",
    NULL,
    NODE8_CPU_I2C0,
    NULL,
    NULL
};

DevNode Node12SpdInstance = {
    "spd3",
    NULL,
    NODE12_CPU_I2C0,
    NULL,
    NULL
};



//#define MPS_ADDR 0x3b //single
#define MPS_ADDR 0x4b

#define I2C_CTL_CLK 100000
#define I2C_BUS_RATE 42
#define ALGORITHM_3A 0x3a
typedef unsigned short           UINT16;
typedef unsigned int             UINT32;
typedef unsigned long long            UINT64;

void SpdReadOps(DevNode *this, int fd)
{
	void * p = NULL;
	int status ;
	char RecordName[30] = {0};
	int c = 0;
	char str[30] = {0};

	p = vtpa(this->devaddr,fd);

	UINT64 I2cRegBaseAddr, NodeId;
	UINT16 Val16;

	I2cRegBaseAddr = p;
	I2cInitSetFreq (I2cRegBaseAddr, 100000, 42, 0x3a); //Lock Freq

	while (1) {
		printf ("please input Spdaddr: ");
		scanf("%s",str);
		int a = 0;
		sscanf (str,"%x",&a);
		printf ("\n");

		//Vddn IOUT
		I2cCtlRead (I2cRegBaseAddr, a, 0x02, 0x1, &Val16);
		switch (Val16) {
			case 0xc: 
				printf (" DDR4 Type:%d \n", Val16);
				break;
			default:
				printf (" No know Type:%d ! \n", Val16);
				break;
		}

		I2cCtlRead (I2cRegBaseAddr, a, 0x03, 0x1, &Val16);
		switch (Val16) {
			case 0x1: 
				printf (" DRAM Type RDIMM:%d \n", Val16);
				break;
			case 0x2: 
				printf (" DRAM Type UDIMM:%d \n", Val16);
				break;
			default:
				printf (" No know Type:%d ! \n", Val16);
				break;
		}
	}
}

Cmd SpdCmd[2] = {
  {"-r",SpdReadOps},
  {NULL,NULL}
};

void SpdInitInstance(void)
{
   Node0SpdInstance.CmdInstance = SpdCmd;
   DevInstanceInsert(&Node0SpdInstance);

   Node4SpdInstance.CmdInstance = SpdCmd;
   DevInstanceInsert(&Node4SpdInstance);

   Node8SpdInstance.CmdInstance = SpdCmd;
   DevInstanceInsert(&Node8SpdInstance);

   Node12SpdInstance.CmdInstance = SpdCmd;
   DevInstanceInsert(&Node12SpdInstance);
}
