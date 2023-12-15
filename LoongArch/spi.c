#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "def.h"
#include "file.h"
#include "process.h"

#define readw(addr)  (*(volatile unsigned int *)(addr))

//#define  SPI_ADDR 0x1fe001f0
#define SPI_ADDR  0  //Get at runtime
#define FLASH_SIZE 0x800000

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

#define INVALID_OFFSET(x)     ((x > FLASH_SIZE)||(x < 0x0)) ? TRUE:FALSE
#define TCM_INVALID_OFFSET(x)     ((x > 0xffffffff)||(x < 0x0)) ? TRUE:FALSE
#define INVALID_NUM(x)        ((x > FLASH_SIZE)||( x <= 0x0)) ? TRUE:FALSE
#define IS_SST25VF032B(M,D,C) ((M == 0xBF)&&(D == 0x25)&&(C == 0x4A)) ? TRUE:FALSE

static UINT8 ValueRegSpcr  = 0xFF;
static UINT8 ValueRegSpsr  = 0xFF;
static UINT8 ValueRegSper  = 0xFF;
static UINT8 ValueRegParam = 0xFF;
UINT64 SPI_REG_BASE = 0 ;
int is3c = 0;
int is3d = 0;
int debug_1 = 1;

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
Fopen_File (
  char *Path, char *Str
  )
{
 char Ch;
 FILE *fp;
 int i = 0;

 fp=fopen(Path,"r");
 while ((Ch=fgetc(fp)) != EOF)
 {
   Str[i] = Ch;
   i++;
 }
// printf("Cpu_Tame:%s", Str);
// printf("Cpu_Type0:%s", Cpu_Type[0]);
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

    if (is3c || is3d) {
      if (debug_1) {
        printf("Because it is 3C5000 or 3D5000, it will be speed down! \n");
        debug_1 = 0;
      }
      //[spre:spr] [01:00] means clk_div=8
      REGSET(REG_SPCR, 0x52);//[1:0] [0:0]spr
      REGSET(REG_SPSR, 0xc0);
      REGSET(REG_SPER, 0x04);//[1:0] [0:1]spre
      REGSET(REG_PARAM, 0x20);
      REGSET(REG_TIME, 0x01); //1/16
    } else {
      REGSET(REG_SPCR, 0x50);//[1:0] [0:0]spr
      REGSET(REG_SPSR, 0xc0);
      REGSET(REG_SPER, 0x05);//[1:0] [0:1]spre
      REGSET(REG_PARAM, 0x40);
      REGSET(REG_TIME, 0x01); //1/8
    }
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
    printf("Erase   : \n");
    ProgressInit();
    do {
        if ((Pos % (4 * BLKSIZE)) == 0) {
            //printf("*");
            ProgressShow(Pos * 100 / Num);
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
    ProgressDone();
    //printf("   Erase OK.\n");

    /* Write blocks step */
    Pos = Offset;
    printf("Program : \n");
    ProgressInit();
    while (Index < Num) {
        if ((Index % 0x4000) == 0) {
            ProgressShow(Index * 100 / Num);
            //printf("*");
            fflush(stdout);
        }
        Addr0 = (Pos + Index) & 0xff;
        Addr1 = ((Pos + Index) >> 8) & 0xff;
        Addr2 = ((Pos + Index) >> 16) & 0xff;

        SpiFlashWriteByte (Addr0, Addr1, Addr2, Buf[Index]);
        Index++;
    }
    ProgressDone();
    //printf("   Program OK.\n");
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

UINTN
SpiTcmRead (
        UINTN      Offset,
        VOID       *Buffer,
        UINTN      Num
        // UINTN      BaseRegAddr
        )
{
    // UINTN Ret;
    UINTN      addr=Offset;
    UINTN      count=0;
    UINT8      data[4]={0};

    if(!Buffer ||(TCM_INVALID_OFFSET(Offset)) || INVALID_NUM(Num)){
        // ASSERT(0);
        return 0;
    }
    // DEBUG((DEBUG_INFO,"func %a,BaseRegAddr=0x%lx, offset=0x%x,num=%d\n",__FUNCTION__,BaseRegAddr,Offset,Num));
    // SpiFlashSetRegBase(BaseRegAddr);
    SpiFlashInit ();
    REGSET(REG_SPER, 0x01);//spre:01  mode 0
    REGSET(REG_SOFTCS ,0x02);  //enable cs1

    // addr
    REGSET(REG_SPDR,((addr >> 24)&0xff));
    while(((REGGET(REG_SPSR))&SPI_BUSY) == SPI_BUSY ){
    }
    REGGET(REG_SPDR);
    REGSET(REG_SPDR,((addr >> 16)&0xff));
    while(((REGGET(REG_SPSR))&SPI_BUSY)  == SPI_BUSY ){
    }
    REGGET(REG_SPDR);
    REGSET(REG_SPDR,((addr >> 8)&0xff));
    while(((REGGET(REG_SPSR))&SPI_BUSY) == SPI_BUSY ){
    }
    REGGET(REG_SPDR);
    REGSET(REG_SPDR,(addr & 0xff));
    while(((REGGET(REG_SPSR))&SPI_BUSY) == SPI_BUSY ){
    }
    REGGET(REG_SPDR);// addr end

    // REGSET(REG_SPDR,0x00);
    for(count=0;count<Num;count++)
    {
        REGSET(REG_SPDR,0x00);
        while(((REGGET(REG_SPSR))&SPI_BUSY)  == SPI_BUSY ){
        }
        data[count] = REGGET(REG_SPDR);
        // printf("0x%x\n",data[count]);
    }
    REGSET(REG_SOFTCS, 0x22); //disable cs1

    memcpy(Buffer,data,4);
    SpiFlashReset ();
    ResetSfcParamReg();

    return 0;
}

static int spi_read_tcm (const char* addr)
{
    void * p = NULL;
    int status ;
    unsigned long long devaddr;
    unsigned long long c = 0;
    unsigned char *Buffer=NULL;
    int i=0;

    sscanf (addr, "%lx",&c);
    devaddr = c;

    int fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    /*Transfer Virtul to Phy Addr*/
    p = vtpa(devaddr, fd);
    SPI_REG_BASE = (UINTN)p & 0xfffffffffffffff0ULL;

    Buffer=malloc(4);
    printf("\n");
    SpiTcmRead (0x80d40000, Buffer, 1);
    printf("Tcm locallity 0 access:0x%x\n",Buffer[0]);

    printf("Tcm Vendor and device ID :");
    for(i=0;i<4;i++)
    {
        SpiTcmRead (0x80d40f00+i,Buffer,1);
        printf("%02x",Buffer[0]);
    }
    printf("\n");

    free(Buffer);
    status = releaseMem(p);
    return status;
}

/**
 * spi_update_gmac:
 * @addr: 
 * @id: 
 * @mac: mac addr
 *
 *
 **/
static int spi_update_gmac (const char* addr, int id, const char* mac)
{
    void * p = NULL;
    int status ;
    unsigned int spcr = 0;
    unsigned int spsr = 0;
    unsigned int sper = 0;
    unsigned int param = 0;
    unsigned long long c = 0;
    unsigned long long devaddr;
    char RecordName[100];

    sscanf (addr, "%lx", &c);
    devaddr = c;

    int fd = open ("/dev/mem", O_RDWR|O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    /*Transfer Virtul to Phy Addr*/
    p = vtpa (devaddr, fd);
    SPI_REG_BASE = (UINTN)p & 0xfffffffffffffff0ULL;

    //void *buf = malloc(1024*1024*4);
    unsigned char *buf3 = NULL;

    //char RecordName[30] = {0};
    int q = 0;
    for (q = 0; q < 29; q++) {
        RecordName[q] = 0;
    }

    unsigned char bufint[7] = {0};

    //parse mac string
    buf3 = parse_mac (mac);
    memcpy(bufint, buf3,6);

    if (id == 0) {
        SpiFlashSafeWrite (0, bufint, 6);
    } else if (id == 1) {
        SpiFlashSafeWrite (0x10, bufint, 6);
    } else {
        printf("------------ID Error!!!-----------\n");
        return 1;
    }

    printf ("------------ok mac-----------\n");
    status = releaseMem (p);
    return status;
}


static int spi_update_flash (const char *path)
{
    void *p = NULL;
    int status ;
    unsigned long long devaddr;

    devaddr = 0x1fe001f0;

    int fd = open ("/dev/mem", O_RDWR|O_SYNC);
    if(fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    /*Transfer Virtul to Phy Addr*/
    p = vtpa(devaddr, fd);
    SPI_REG_BASE = (UINTN)p;

    void *buf = malloc (FLASH_SIZE);

    FILE *pfile = fopen(path, "r");
    if (pfile==NULL) {
        printf("Read File Error , PATH error!!!\n");
        return 1;
    }
    fread(buf, FLASH_SIZE, 1, pfile);
    printf("------------Read Buf Get Success!-----------\n");

    UpdateBiosInSpiFlash(0, buf, FLASH_SIZE);

    status = releaseMem(p);
    close(fd);
    return status;
}

static int spi_dump_flash (const char *path)
{
    void *p = NULL;
    int status ;
    unsigned long long devaddr;

    devaddr = 0x1fe001f0;

    int fd = open ("/dev/mem", O_RDWR |O_SYNC);
    if(fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    /*Transfer Virtul to Phy Addr*/
    p = vtpa (devaddr, fd);
    SPI_REG_BASE = (UINTN)p & 0xfffffffffffffff0ULL;

    void *buf = malloc (FLASH_SIZE);
    SpiFlashRead (0, buf, FLASH_SIZE);

    FILE *pfile = fopen(path, "w");
    if (pfile==NULL) {
        printf("Read File Error , PATH error!!!\n");
        return 1;
    }

    fwrite (buf, 1, 4128768, pfile);
    fflush (pfile);
    fclose (pfile);
    free (buf);

    status = releaseMem(p);
    close(fd);
    return status;
}

static int spi_read_flash (const char* addr, int count)
{
    void * p = NULL;
    int status ;
    unsigned long long c = 0;
    unsigned long long devaddr;
    unsigned char *Buffer = NULL;

    sscanf (addr, "%lx", &c);

    devaddr = c;

    int fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    /*Transfer Virtul to Phy Addr*/
    p = vtpa (devaddr, fd);
    SPI_REG_BASE = (UINTN)p & 0xfffffffffffffff0ULL;

    Buffer = malloc (count);
    SpiFlashRead (0, Buffer, count);
    hexdump (0, Buffer, count);

    free(Buffer);
    status = releaseMem(p);
    close(fd);
    return status;
}

int spi_update_smbios(const char *addr);

static const char *const spi_usages[] = {
    PROGRAM_NAME" spi <args>",
    NULL,
};

int cmd_spi (int argc, const char **argv)
{
    int flag_read = 0;
    int flag_update = 0;
    int flag_dump = 0;
    int flag_gmac = 0;
    int flag_tcm = 0;
    int flag_smbios = 0;
    int count = 0;
    int id = 0;
    const char *file = NULL;
    const char *addr = NULL;
    const char *mac = NULL;
    uid_t uid;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP (),
        OPT_GROUP ("Options:"),
        OPT_BOOLEAN ('r', "read", &flag_read, "read ls7a spi from address", NULL, 0, 0),
        OPT_BOOLEAN ('u', "update", &flag_update, "update ls3a spi flash", NULL, 0, 0),
        OPT_BOOLEAN ('d', "dump", &flag_dump, "dump the ls3a spi flash", NULL, 0, 0),
        OPT_BOOLEAN ('g', "gmac", &flag_gmac, "update gmac flash", NULL, 0, 0),
        OPT_BOOLEAN ('t', "tcm", &flag_tcm, "read ls7a tcm from address", NULL, 0, 0),
        OPT_BOOLEAN ('s', "smbios", &flag_smbios, "update smbios ls3a spi flash", NULL, 0, 0),
        OPT_GROUP ("Arguments:"),
        OPT_STRING  ('f', "file", &file, "file path to read/write", NULL, 0, 0),
        OPT_STRING  ('a', "address", &addr, "Pci's spi control address(e.g. 1fe001f0)", NULL, 0, 0),
        OPT_INTEGER ('i', "id", &id, "Mac id", NULL, 0, 0),
        OPT_STRING  ('m', "mac", &mac, "Mac address(e.g. 00:11:22:33:44:55)", NULL, 0, 0),
        OPT_INTEGER ('c', "count", &count, "read count", NULL, 0, 0),
        OPT_END (),
    };

    argparse_init (&argparse, options, spi_usages, 0);
    argc = argparse_parse (&argparse, argc, argv);

    if (!(flag_update || flag_dump || flag_read || flag_tcm || flag_gmac || flag_smbios)) {
        argparse_usage(&argparse);
        return 1;
    }

    uid = geteuid ();
    if (uid != 0) {
        printf ("Please run with root!\n");
        return -1;
    }

    if (flag_update) {
        if (file == NULL) {
            printf ("Please setup the file.\n");
            return 1;
        }
        // Get Cpu Name to distinguish
        char *Path="./"FILE_NAME_1;
        char Cpu_Name[100];
        Fopen_File(Path, Cpu_Name);
        is3c = !strncmp("Loongson-3C5000", Cpu_Name, 15);
        is3d = !strncmp("Loongson-3D5000", Cpu_Name, 15);

        spi_update_flash (file);
    } else if (flag_dump) {
        if (file == NULL) {
            printf ("Please setup the file.\n");
            return 1;
        }
        spi_dump_flash (file);
    } else if (flag_read) {
        if (addr == NULL) {
            printf ("Please setup the address.\n");
            return 1;
        }
        if (count == 0) {
            printf ("Please setup the count.\n");
            return 1;
        }
        spi_read_flash (addr, count);
    } else if (flag_tcm) {
        if (addr == NULL) {
            printf ("Please setup the address.\n");
            return 1;
        }
        spi_read_tcm (addr);
    } else if (flag_gmac) {
        if (addr == NULL) {
            printf ("Please setup the address.\n");
            return 1;
        }
        spi_update_gmac(addr, id, mac);
    } else if (flag_smbios) {
        spi_update_smbios(addr);
    }

    return 0;
}
