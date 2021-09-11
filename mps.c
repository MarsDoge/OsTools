#include "acpi.h"

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000
#define CPU_I2C0 0x1fe00120
#define NODE0_CPU_I2C1 0x1fe00130
#define NODE4_CPU_I2C1 0x40001fe00130ULL
#define NODE8_CPU_I2C1 0x80001fe00130ULL
#define NODE12_CPU_I2C1 0xC0001fe00130ULL

DevNode Node0I2cInstance = {
    "mps0",
    NULL,
    NODE0_CPU_I2C1,
    NULL,
    NULL
};

DevNode Node4I2cInstance = {
    "mps1",
    NULL,
    NODE4_CPU_I2C1,
    NULL,
    NULL
};

DevNode Node8I2cInstance = {
    "mps2",
    NULL,
    NODE8_CPU_I2C1,
    NULL,
    NULL
};

DevNode Node12I2cInstance = {
    "mps3",
    NULL,
    NODE12_CPU_I2C1,
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

void I2cReadOps(DevNode *this, int fd)
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

	Val16 = 0x0; //a2
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

  //Vddn IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x8c, 0x2, &Val16);
  printf ("qdy %s line:%d Vddn IOUT:%d \n", __func__,__LINE__,Val16);

	Val16 = 0x0; //a2
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);

  //Vddn IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x96, 0x2, &Val16);
  printf ("qdy %s line:%d Vddn Power:%d \n", __func__,__LINE__,Val16);

  //Vddn IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x8b, 0x2, &Val16);
  printf ("qdy %s line:%d Vddn Value:%d \n", __func__,__LINE__,Val16);

  /*Vp sensed*/
	Val16 = 0x1; //a2
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

  //Vddp IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x8c, 0x2, &Val16);
  printf ("qdy %s line:%d    Vddp IOUT %d\n", __func__,__LINE__,Val16);

  //Vddp IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x8b, 0x2, &Val16);
  printf ("qdy %s line:%d    Vddp IOUT %d\n", __func__,__LINE__,Val16);

  //Vddp IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x96, 0x2, &Val16);
  printf ("qdy %s line:%d    Vddp Power %d\n", __func__,__LINE__,Val16);

  /*PIN Power*/
	Val16 = 0x0; //a2
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

  //Vddn IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x97, 0x2, &Val16);
  printf ("qdy %s line:%d  Pin Power %d \n", __func__,__LINE__,Val16);

	Val16 = 0x0; //a2
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);

}

void I2cWriteOps(DevNode *this, int fd)
{
	void * p = NULL;
	int status ;
	char RecordName[30] = {0};
	int c = 0;
	char str[30] = {0};

	p = vtpa(this->devaddr,fd);

	UINT64 I2cRegBaseAddr, NodeId;
	UINT32 Val16;

	while (1) {
		I2cRegBaseAddr = p;
		I2cInitSetFreq (I2cRegBaseAddr, 100000, 42, 0x3a); //Lock Freq

		Val16 = 0x0; //a2
		I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

		Val16 = 0x210;
		//Vddn IOUT
		I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x5e, 0x2, &Val16);

		char str[30] = {0};
		scanf("%s",str);
		sscanf (str,"%d",&Val16);
		Val16 = Val16 / 10 - 49;
		I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x21, 0x2, &Val16);

		Val16 = 0x0; //a2
		I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
	}
}

Cmd I2cCmd[3] = {
  {"-r",I2cReadOps},
  {"-w",I2cWriteOps},
  {NULL,NULL}
};

void I2cInitInstance(void)
{
   Node0I2cInstance.CmdInstance = I2cCmd;
   DevInstanceInsert(&Node0I2cInstance);

   Node4I2cInstance.CmdInstance = I2cCmd;
   DevInstanceInsert(&Node4I2cInstance);

   Node8I2cInstance.CmdInstance = I2cCmd;
   DevInstanceInsert(&Node8I2cInstance);

   Node12I2cInstance.CmdInstance = I2cCmd;
   DevInstanceInsert(&Node12I2cInstance);
}
