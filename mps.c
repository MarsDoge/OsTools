#include "acpi.h"

//#define SPI_CONFUSE_SPACE (0x0efdfe000000 + 0x8000/*Need 4K align*/ ) //b000 is Spi,so add 0x3000
#define CPU_I2C0 0x1fe00120
#define NODE0_CPU_I2C1 0x1fe00130
#define NODE4_CPU_I2C1 0x40001fe00130ULL
#define NODE8_CPU_I2C1 0x80001fe00130ULL
#define NODE12_CPU_I2C1 0xC0001fe00130ULL

DevNode Node0I2cInstance = {
    "i2c0",
    NULL,
    NODE0_CPU_I2C1,
    NULL,
    NULL
};

DevNode Node4I2cInstance = {
    "i2c1",
    NULL,
    NODE4_CPU_I2C1,
    NULL,
    NULL
};

DevNode Node8I2cInstance = {
    "i2c2",
    NULL,
    NODE8_CPU_I2C1,
    NULL,
    NULL
};

DevNode Node12I2cInstance = {
    "i2c3",
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

  while (1) {
//  for (NodeId = 0; NodeId < NodeNum; NodeId += 4) {
//	I2cRegBaseAddr = CPU_I2C1_REG_BASE | (NodeId << 44);
	I2cRegBaseAddr = p;
	I2cInitSetFreq (I2cRegBaseAddr, 100000, 42, 0x3a); //Lock Freq

	Val16 = 0x0; //a2
	//I2cCtlWriteNostop (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	//I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

	//Val16 = Vddp;
  //Vddn IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x8c, 0x2, &Val16);
  printf ("qdy %s line:%d Vddn IOUT:%d \n", __func__,__LINE__,Val16);

	Val16 = 0x0; //a2
	//I2cCtlWriteNostop (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
	//I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
  //}

  sleep (3);

//  for (NodeId = 0; NodeId < NodeNum; NodeId += 4) {
//	I2cRegBaseAddr = CPU_I2C1_REG_BASE | (NodeId << 44);
	I2cRegBaseAddr = p;
	I2cInitSetFreq (I2cRegBaseAddr, 100000, 42, 0x3a); //Lock Freq

	Val16 = 0x0; //a2
	//I2cCtlWriteNostop (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	//I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

	//Val16 = Vddp;
  //Vddn IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x96, 0x2, &Val16);
  printf ("qdy %s line:%d Vddn Power:%d \n", __func__,__LINE__,Val16);

	Val16 = 0x0; //a2
	//I2cCtlWriteNostop (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
	//I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
  //}

  sleep (3);

  /*Vp sensed*/
  //for (NodeId = 0; NodeId < NodeNum; NodeId += 4) {
	//I2cRegBaseAddr = CPU_I2C1_REG_BASE | (NodeId << 44);
	I2cRegBaseAddr = p;
	I2cInitSetFreq (I2cRegBaseAddr, I2C_CTL_CLK, I2C_BUS_RATE, ALGORITHM_3A); //Lock Freq

	Val16 = 0x1; //a2
	//I2cCtlWriteNostop (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	//I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

	//Val16 = Vddp;
  //Vddn IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x8c, 0x2, &Val16);
  printf ("qdy %s line:%d    Vddp IOUT %d\n", __func__,__LINE__,Val16);

	Val16 = 0x0; //a2
	//I2cCtlWriteNostop (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
	//I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
  //}

  sleep (3);

  /*Vp sensed*/
  //for (NodeId = 0; NodeId < NodeNum; NodeId += 4) {
	//I2cRegBaseAddr = CPU_I2C1_REG_BASE | (NodeId << 44);
	I2cRegBaseAddr = p;
	I2cInitSetFreq (I2cRegBaseAddr, I2C_CTL_CLK, I2C_BUS_RATE, ALGORITHM_3A); //Lock Freq

	Val16 = 0x1; //a2
	//I2cCtlWriteNostop (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	//I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

	//Val16 = Vddp;
  //Vddn IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x96, 0x2, &Val16);
  printf ("qdy %s line:%d    Vddp Power %d\n", __func__,__LINE__,Val16);

	Val16 = 0x0; //a2
	//I2cCtlWriteNostop (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
	//I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
  //}

  sleep (3);

  /*PIN Power*/
  //for (NodeId = 0; NodeId < NodeNum; NodeId += 4) {
	//I2cRegBaseAddr = CPU_I2C1_REG_BASE | (NodeId << 44);
	I2cRegBaseAddr = p;
	I2cInitSetFreq (I2cRegBaseAddr, I2C_CTL_CLK, I2C_BUS_RATE, ALGORITHM_3A); //Lock Freq

	Val16 = 0x0; //a2
	//I2cCtlWriteNostop (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	//I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0, 0x1, &Val16);

	//Val16 = Vddp;
  //Vddn IOUT
	I2cCtlRead (I2cRegBaseAddr, MPS_ADDR, 0x97, 0x2, &Val16);
  printf ("qdy %s line:%d  Pin Power %d \n", __func__,__LINE__,Val16);

	Val16 = 0x0; //a2
	//I2cCtlWriteNostop (I2cRegBaseAddr, MPS_ADDR, 0x1 << 16, 0x1, &Val16);
	I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
	//I2cCtlWrite (I2cRegBaseAddr, MPS_ADDR, 0x0, 0x1, &Val16);
  //}

  sleep (3);

  }
}

Cmd I2cCmd[2] = {
  {"-r",I2cReadOps},
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
