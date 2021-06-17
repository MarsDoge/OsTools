#include "spi.h"

#define readw(addr)  (*(volatile unsigned int *)(addr))

//#define  SPI_ADDR 0x1fe001f0
#define  SPI_ADDR  0  //Get at runtime

DevNode SpiInstance = {
    "spi",
    NULL,
    SPI_ADDR,
    NULL,
    NULL
};
typedef unsigned char           UINT8;
typedef char           INT8;
typedef unsigned int           UINT32;
typedef int           INT32;
typedef unsigned short           UINT16;
typedef unsigned long long            UINT64;
typedef unsigned long long            UINTN;
typedef long long            INTN;
typedef void            VOID;
#define GPIO_0        (0x1<<0)
#define GPIO_1        (0x1<<1)
#define GPIO_2        (0x1<<2)
#define GPIO_3        (0x1<<3)
#define GPIO_12       (0x1<<12)
#define GPIO_CS_BIT           GPIO_0
#define GPIO_CPU_CS_ENABLE    GPIO_3

#define SPE_OFFSET     6
#define MSTR_OFFSET    4
#define CPOL_OFFSET    3
#define CPHA_OFFSET    2
#define SPR_OFFSET     0
#define SPE            (1<<SPE_OFFSET)
#define MSTR           (1<<MSTR_OFFSET)

#define BP0    (1 << 2)
#define BP1    (1 << 3)
#define BP2    (1 << 4)
#define BP3    0

#define REG_SPCR   0x00
#define REG_SPSR   0x01
#define REG_SPDR   0x02
#define REG_SPER   0x03
#define REG_SOFTCS 0x05
#define REG_TIME   0x06
#define REG_PARAM  0x04
#define WREN       0x06
#define WRDI       0x04
#define RDID       0x90
#define RDSR       0x05
#define WRSR       0x01
#define READ       0x03
#define AAI        0x02
#define EBSY       0x70
#define DBSY       0x80
#define EWSR       0x50
#define FAST_READ  0x0B
#define BYTE_WRITE 0x02     /* Byte Program Enable */
#define AAI_WRITE  0xad     /* Byte Program Enable */
#define BE4K       0x20     /* 4K Byte block Rrase, Sector Erase */
#define BE4KSIZE   0x1000   /* 4K Byte block Rrase, Sector Erase */
#define BE32K      0x52     /* 32K Byte block Rrase, Sector Erase */
#define BE32KSIZE  0x8000   /* 32K Byte block Rrase, Sector Erase */
#define BE64K      0xD8     /* 64K Byte block Rrase, Sector Erase */
#define BE64KSIZE  0x10000  /* 64K Byte block Rrase, Sector Erase */
#define CHIPERASE  0xC7     /* Chip Erase */
#define BLKSIZE    0x1000
#define BLKERASE   BE4K
#define MACID_ADDR 0x00
#define DEVID_ADDR 0x01
#define VOID_ADDR  0x00
#define VOID_CMD   0x00
#define CHIPERASESET               0x4
#define CSCCHIPERASET              0x5
#define PROGRAM_AAI_MODE           0x1
#define SPI_FREE                   0x00
#define SPI_BUSY                   0x01

#define STORE8(Addr, Value)    *(volatile UINT8 *)((Addr)) = ((UINT8) (Value & 0xff))
#define LOAD8(Addr)            *(volatile UINT8 *)((Addr))
#define REGSET(Id,Value)       STORE8(SPI_REG_BASE + Id, Value)
#define REGGET(Id)             LOAD8(SPI_REG_BASE + Id)

#define LS3ASPIFLASHADDR     0x900000001fe001f0
#define LS7ASPIFLASHADDR     GetLs7ASpiRegBaseAddr()
#define SpiFlashDelayUs(x) 

#define LS3APROC_PLL_REG     0x900000001fe001d0
#define LS3APROC_MISC_REG    0x900000001fe00420

#define TRUE 1
#define FALSE 0

#define INVALID_OFFSET(x)     ((x > 0x400000)||(x < 0x0)) ? TRUE:FALSE
#define INVALID_NUM(x)        ((x > 0x400000)||( x <= 0x0)) ? TRUE:FALSE
#define IS_SST25VF032B(M,D,C) ((M == 0xBF)&&(D == 0x25)&&(C == 0x4A)) ? TRUE:FALSE

static UINT8 ValueRegSpcr  = 0xFF;
static UINT8 ValueRegSpsr  = 0xFF;
static UINT8 ValueRegSper  = 0xFF;
static UINT8 ValueRegParam = 0xFF;
UINT64 SPI_REG_BASE = 0 ;

VOID
SpiFlashSetCs (
  INTN Bit
  )
{
  UINT8          Val;

  if (Bit)
    Val = 0x11;
  else
    Val = 0x01;
  REGSET(CSCCHIPERASET,Val);

  SpiFlashDelayUs (3);
}

VOID ResetSfcParamReg()
{
  REGSET(REG_PARAM,0x47);
}

UINT8
SpiFlashWriteByteCmd (
  UINT8 Value
  )
{
  UINT8 Ret;
  INT32 TimeOut = 100000;

  REGSET (REG_SPDR, Value);
  while ((REGGET(REG_SPSR) & 0x01) && TimeOut--);
  Ret = REGGET(REG_SPDR);
  if (TimeOut < 0)
    while(1);

  return Ret;
}

UINT8
SpiFlashReadByteCmd (
  VOID
  )
{
  return SpiFlashWriteByteCmd(0x00);
}

VOID
SpiFlashInit (
  VOID
  )
{
  if (ValueRegSpcr == 0xFF) {
    ValueRegSpcr = REGGET(REG_SPCR);
  }
  if (ValueRegSpsr == 0xFF) {
    ValueRegSpsr = REGGET(REG_SPSR);
  }
  if (ValueRegSper == 0xFF) {
    ValueRegSper = REGGET(REG_SPER);
  }
  if (ValueRegParam == 0xFF) {
    ValueRegParam = REGGET(REG_PARAM);
  }

  //[spre:spr] [01:00] means clk_div=8
  REGSET(REG_SPCR, 0x50);//[1:0] [0:0]spr
  REGSET(REG_SPSR, 0xc0);
  REGSET(REG_SPER, 0x05);//[1:0] [0:1]spre
  REGSET(REG_PARAM, 0x40);
  REGSET(REG_TIME, 0x01);
}

VOID
SpiFlashReset (
  VOID
  )
{

  REGSET(REG_SPCR, ValueRegSpcr);
  REGSET(REG_SPSR, ValueRegSpsr);
  REGSET(REG_SPER, ValueRegSper);
  REGSET(REG_PARAM, ValueRegParam);

  ValueRegSpcr  = 0xFF;
  ValueRegSpsr  = 0xFF;
  ValueRegSper  = 0xFF;
  ValueRegParam = 0xFF;
}

UINT8 SpiFlashReadStatus(VOID)
{
  UINT8 Val;

  REGSET(REG_SOFTCS,0x01);
  SpiFlashWriteByteCmd(RDSR);
  Val = SpiFlashReadByteCmd();
  REGSET(REG_SOFTCS,0x11);

  return Val;
}

UINTN
SpiFlashWait (
  VOID
  )
{
  UINTN Ret;
  INT32 TimeOut = 100000;
  INT32 Count = 5;
  do {
    Ret = SpiFlashReadStatus();
    if (TimeOut == 0)
    {
      Count--;
      if (Count < 0 )
        while(1);
      TimeOut = 100000;
      SpiFlashDelayUs(2);
      printf("TimeOut Count is %d, Status Reg=0x%x\n",Count,Ret);
    }
  } while ((Ret & 1) && TimeOut--);

  return Ret;
}

VOID SpiFlashRDID(UINT8 *Manu,UINT8 *Device,UINT8 *Capa)
{
  SpiFlashWait();

  REGSET(REG_SOFTCS,0x01);
  SpiFlashWriteByteCmd(0x9f);

  *Manu = SpiFlashReadByteCmd();
  //Dbgprintf(DEBUG_INFO,"Manufacturer's ID:0x%x\n",*Manu);

  *Device = SpiFlashReadByteCmd();
  //Dbgprintf(DEBUG_INFO,"Device ID-memory_type:0x%x\n",*Device);

  *Capa = SpiFlashReadByteCmd();
  //Dbgprintf(DEBUG_INFO,"Device ID-memory_capacity:0x%x\n",*Capa);

  REGSET(REG_SOFTCS,0x11);
}

VOID SpiFlashWriteEnable(VOID)
{
  SpiFlashWait();
  REGSET(REG_SOFTCS,0x01);
  SpiFlashWriteByteCmd(WREN);
  REGSET(REG_SOFTCS,0x11);
}

VOID SpiFlashEWRS(VOID)
{
  SpiFlashWait();
  REGSET(REG_SOFTCS,0x01);
  SpiFlashWriteByteCmd(EWSR);
  REGSET(REG_SOFTCS,0x11);
}

VOID SpiFlashWriteDisable(VOID)
{
  SpiFlashWait();
  REGSET(REG_SOFTCS,0x01);
  SpiFlashWriteByteCmd(WRDI);
  REGSET(REG_SOFTCS,0x11);
}

VOID SpiFlashWriteStatus(UINT8 Val)
{
  SpiFlashEWRS();
  REGSET(REG_SOFTCS,0x01);
  SpiFlashWriteByteCmd(WRSR);
  SpiFlashWriteByteCmd(Val);
  REGSET(REG_SOFTCS,0x11);

  SpiFlashWriteDisable();
}

VOID
SpiFlashDisableWriteProtection (
  VOID
  )
{

  UINT8 Val;

  Val = SpiFlashWait ();
  Val &= ~(BP0 | BP1 | BP2);
  SpiFlashWriteStatus(Val);
  SpiFlashWait ();

#if ENABLE_WRITE_PROTECTION
  /*some flash not support 0x50 need the follow code*/
  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (WREN);
  SpiFlashSetCs (1);
  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (WRSR);
  SpiFlashWriteByteCmd (Val);
  SpiFlashSetCs (1);
  SpiFlashWait ();
  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (WREN);
  SpiFlashSetCs (1);
#endif
}

VOID
SpiFlashEnableWriteProtection (
  VOID
  )
{
  UINT8 Val;

  Val = (BP0 | BP1 | BP2);
  SpiFlashWriteStatus(Val);
  SpiFlashWait ();
#if ENABLE_WRITE_PROTECTION
  /*some flash not support 0x50 need the follow code*/
  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (WREN);
  SpiFlashSetCs (1);
  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (WRSR);
  SpiFlashWriteByteCmd (Val);
  SpiFlashSetCs (1);
  SpiFlashWait ();

  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (WRDI);
  SpiFlashSetCs (1);
  /*wait command executed done*/
  SpiFlashWait();
#endif
}

VOID
SpiFlashEraseBlock (
  UINTN  Addr0,
  UINTN  Addr1,
  UINTN  Addr2
  )
{
  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (WREN);
  SpiFlashSetCs (1);
  SpiFlashWait();

  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (BLKERASE);
  SpiFlashWriteByteCmd (Addr2);
  SpiFlashWriteByteCmd (Addr1);
  SpiFlashWriteByteCmd (Addr0);
  SpiFlashSetCs (1);
}

VOID
SpiFlashWriteByte (
  UINTN  Addr0,
  UINTN  Addr1,
  UINTN  Addr2,
  UINT8  Buf
  )
{
  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (WREN);
  SpiFlashSetCs (1);

  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (BYTE_WRITE);
  SpiFlashWriteByteCmd (Addr2);
  SpiFlashWriteByteCmd (Addr1);
  SpiFlashWriteByteCmd (Addr0);
  SpiFlashWriteByteCmd (Buf);
  SpiFlashSetCs (1);
  SpiFlashWait ();
}

VOID
SpiFlashSpiEraseBlocks (
  UINTN  Off,
  UINTN  Num
  )
{
  UINTN  Offset = Off;
  UINTN  Addr0;
  UINTN  Addr1;
  UINTN  Addr2;

  do {
    Addr0 =  Offset & 0xff;
    Addr1 =  (Offset >> 8) & 0xff;
    Addr2 =  (Offset >> 16) & 0xff;

    SpiFlashEraseBlock (Addr0, Addr1, Addr2);

    Offset += BLKSIZE;
    SpiFlashWait();
  } while (Offset < Num + Off);
}

VOID
SpiFlashSpiWriteBuffer (
  UINTN    Offset,
  VOID     *Buffer,
  UINTN    Num
  )
{
  UINTN  Pos = Offset;
  UINT8  *Buf = (UINT8 *) Buffer;
  UINTN  Index = 0;
  UINTN  Addr0;
  UINTN  Addr1;
  UINTN  Addr2;

  while (Index < Num) {
    Addr0 = (Pos + Index) & 0xff;
    Addr1 = ((Pos + Index) >> 8) & 0xff;
    Addr2 = ((Pos + Index) >> 16) & 0xff;

    SpiFlashWriteByte (Addr0, Addr1, Addr2, Buf[Index]);
    Index++;
  }
}

VOID
SpiFlashEraseAndWriteBlocks (
  UINTN      Offset,
  VOID       *Buffer,
  UINTN      Num
  )
{
  UINTN  Pos = Offset;
  UINT8  *Buf = (UINT8 *) Buffer;
  UINTN  Index = 0;
  UINTN  Addr0;
  UINTN  Addr1;
  UINTN  Addr2;

  /* Erase blocks step */
  printf("Erase   : ");
  do {
    if ((Pos % (4 * BLKSIZE)) == 0) {
      printf("*");
      fflush(stdout);
    }
    Addr0 =  Pos & 0xff;
    Addr1 =  (Pos >> 8) & 0xff;
    Addr2 =  (Pos >> 16) & 0xff;

    SpiFlashEraseBlock (Addr0, Addr1, Addr2);
    SpiFlashDelayUs(1);
    Pos += BLKSIZE;
    SpiFlashWait();
  } while (Pos < Num + Offset);
  printf("   Erase OK.\n");

  /* Write blocks step */
  Pos = Offset;
  printf("Program : ");
  while (Index < Num) {
    if ((Index % 0x4000) == 0) {
      printf("*");
      fflush(stdout);
    }
    Addr0 = (Pos + Index) & 0xff;
    Addr1 = ((Pos + Index) >> 8) & 0xff;
    Addr2 = ((Pos + Index) >> 16) & 0xff;

    SpiFlashWriteByte (Addr0, Addr1, Addr2, Buf[Index]);
    Index++;
  }
  printf("   Program OK.\n");
}

UINTN
SpiFlashSpiReadBuffer (
  UINTN  Offset,
  VOID  *Buffer,
  UINTN  Num
  )
{
  UINTN   Pos = Offset;
  UINT8  *Buf = (UINT8 *) Buffer;
  UINTN   Index;

  SpiFlashSetCs (0);
  SpiFlashWriteByteCmd (FAST_READ);
  SpiFlashWriteByteCmd ((Pos >> 16) & 0xff);
  SpiFlashWriteByteCmd ((Pos >> 8) & 0xff);
  SpiFlashWriteByteCmd (Pos & 0xff);
  SpiFlashWriteByteCmd (0);
  for (Index = 0; Index < Num; Index++)
    Buf[Index] = SpiFlashReadByteCmd ();

  SpiFlashSetCs (1);

  return Index;
}

UINTN
GetLs7ASpiRegBaseAddr(
  VOID
  )
{
  UINTN BaseAddr;

  BaseAddr = *(volatile UINT32 *)(0x90000efdfe000010 | (22 << 11));
  BaseAddr &= 0xfffffff0;
  BaseAddr |=0x9000000000000000;

  return BaseAddr;
}

VOID
SpiFlashErase (
  UINTN      Offset,
  UINTN      Num
  )
{
  if((INVALID_OFFSET(Offset)) || INVALID_NUM(Num)){
    //ASSERT(0);
    while(1);
  }

  SpiFlashInit ();
  SpiFlashDisableWriteProtection ();
  SpiFlashSpiEraseBlocks (Offset, Num);
  SpiFlashEnableWriteProtection ();
  SpiFlashReset ();
  ResetSfcParamReg();
}

UINTN
SpiFlashRead (
  UINTN      Offset,
  VOID       *Buffer,
  UINTN      Num
  )
{
  UINTN Ret;

  if(!Buffer ||(INVALID_OFFSET(Offset)) || INVALID_NUM(Num)){
    //ASSERT(0);
    while(1);
  }

  SpiFlashInit ();
  Ret = SpiFlashSpiReadBuffer (Offset, Buffer, Num);
  SpiFlashReset ();
  ResetSfcParamReg();

  return Ret;
}

VOID
SpiFlashWrite (
  UINTN        Offset,
  VOID         *Buffer,
  UINTN        Num
  )
{
  if(!Buffer ||(INVALID_OFFSET(Offset)) || INVALID_NUM(Num)){
    //ASSERT(0);
    while(1);
  }

  SpiFlashInit ();
  SpiFlashDisableWriteProtection ();
  SpiFlashSpiWriteBuffer (Offset, Buffer, Num);
  SpiFlashEnableWriteProtection ();
  SpiFlashReset ();
  ResetSfcParamReg();
}

VOID
UpdateBiosInSpiFlash (
  UINTN      Offset,
  VOID       *Buffer,
  UINTN      Num
  )
{
  if(!Buffer ||(INVALID_OFFSET(Offset)) || INVALID_NUM(Num)){
    //ASSERT(0);
    while(1);
  }
#if 0
  UINT16 PllRegVal = Read16(LS3APROC_PLL_REG);
  Write16(LS3APROC_PLL_REG, (0x7777 ^ (1 << ((BOOTCORE_ID << 2) + 3))) & PllRegVal);//shutdown slave core
  UINTN MiscRegVal = Read64(LS3APROC_MISC_REG);
  if (MiscRegVal & (1 << 8))
    Write64(LS3APROC_MISC_REG, (~(1 << 8)) & MiscRegVal);//shutdown 132
#endif

  SpiFlashInit ();
  SpiFlashDisableWriteProtection ();
  SpiFlashEraseAndWriteBlocks (Offset, Buffer, Num);
  SpiFlashEnableWriteProtection ();
  SpiFlashReset ();
  ResetSfcParamReg();


#if 0

  Write16(LS3APROC_PLL_REG, PllRegVal);
  if (MiscRegVal & (1 << 8))
    Write64(LS3APROC_MISC_REG, MiscRegVal);
#endif
}

VOID
SpiFlashSafeWrite (
  UINTN        Offset,
  VOID         *Buffer,
  UINTN        Num
  )
{
  UINT64 SectorStart;
  UINT64 SectorNum;
  UINT8  *Buff;

  if(!Buffer ||(INVALID_OFFSET(Offset)) || INVALID_NUM(Num)){
    //ASSERT(0);
    while(1);
  }

  SectorStart = Offset / BLKSIZE;
  SectorNum   = ((Offset + Num - 1) / BLKSIZE) - (Offset / BLKSIZE) + 1;

  Buff = malloc(SectorNum*BLKSIZE);

  if(!Buff){
    //ASSERT(0);
    while(1);
  }

  SpiFlashRead(SectorStart*BLKSIZE,Buff,SectorNum*BLKSIZE);
  memcpy(&Buff[Offset%BLKSIZE],Buffer,Num);
  SpiFlashErase(Offset,Num);
  SpiFlashWrite(SectorStart*BLKSIZE,Buff,SectorNum*BLKSIZE);
  free(Buff);
}

void FlashUpdateOps(DevNode *this,int fd)
{
	void * p = NULL;
  int status ;
  unsigned int spcr = 0;
  unsigned int spsr = 0;
  unsigned int sper = 0;
  unsigned int param = 0;

  this->devaddr = 0x1fe001f0;

  /*Transfer Virtul to Phy Addr*/
  p = vtpa(this->devaddr,fd);
  SPI_REG_BASE = (UINTN)p;

  void *buf = malloc(1024*1024*4);

  printf("Please Input File Name: ");
  //char RecordName[30] = "LS3A50007A.fd";
  char RecordName[30] = {0};
  status = scanf("%s",RecordName);
  size_t RecordSize = strlen(RecordName);

  FILE *pfile = fopen(RecordName,"r");
  if (pfile==NULL) {
    printf("Read File Error , PATH error!!!\n");
    return 1;
  }
  fread(buf,1024*1024*4,1,pfile);
  printf("------------Read Buf Get Success!-----------\n");

  UpdateBiosInSpiFlash(0,buf,1024*1024*4);

  int tmp = 0;
  //tmp = readw(p); shutdown slave core and 132
  printf("%lx\n",tmp);

	status = releaseMem(p);
}

void* parse_mac(char *szMacStr);
void GmacUpdateOps(DevNode *this,int fd)
{
	void * p = NULL;
  int status ;
  unsigned int spcr = 0;
  unsigned int spsr = 0;
  unsigned int sper = 0;
  unsigned int param = 0;
  int c = 0;
  char RecordName[100];

  printf("Please Input Pci's Spi Control Address (obtained through Pci Access): ");
  status = scanf("%s",RecordName);
  sscanf (RecordName,"%x",&c);
  //c = atoi(RecordName);

  //write spi control Address
  this->devaddr = c;

  /*Transfer Virtul to Phy Addr*/
  p = vtpa(this->devaddr,fd);
  SPI_REG_BASE = (UINTN)p & 0xfffffffffffffff0ULL;

  //void *buf = malloc(1024*1024*4);
    unsigned char *buf3 = NULL;

  //char RecordName[30] = {0};
  int q = 0;
  for(q = 0; q<29; q++){
    RecordName[q] = 0;
  }
  printf("Please Input Gmac id: ");

  char buf[6][20] = {0};
    unsigned char bufint[7] = {0};

    int j = 0;
    int k = 0;
    int i = 0;
  //GmacUpdateOps
  c = 0;
  status = scanf("%s",RecordName);
  c = atoi(RecordName);
  printf("\n");
  if (c == 0){
    printf("Example Mac burn addr: 11:22:33:44:55:66 !!!\n");
    printf("Please Input Mac%d burn addr,Use (:) separate:",c);
    status = scanf("%s",RecordName);
    
    //parse mac string
    buf3 = parse_mac(RecordName);
    memcpy(bufint,buf3,6);

    SpiFlashSafeWrite(0,bufint,6);
  } else if (c == 1){
    printf("Example Mac burn addr: 11:22:33:44:55:66 !!!\n");
    printf("Please Input Mac%d burn addr,Use (:) separate:",c);
    status = scanf("%s",RecordName);
    
    //parse mac string
    buf3 = parse_mac(RecordName);
    memcpy(bufint,buf3,6);
    
    SpiFlashSafeWrite(0x10,bufint,6);
  } else {
  printf("------------ID Error!!!-----------\n");
  return ;
  }
  printf("------------ok mac-----------\n");
 // char buf1[3] = {0x11,0x22,0x33};
 // UpdateBiosInSpiFlash(0,buf1,2);

	status = releaseMem(p);
}



Cmd SpiCmd[3] = {
  {"-u",FlashUpdateOps},
  {"-g",GmacUpdateOps},
  {NULL,NULL}
};

void SpiInitInstance(void)
{
   SpiInstance.CmdInstance = SpiCmd;
   DevInstanceInsert(&SpiInstance);
}

