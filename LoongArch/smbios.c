#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "def.h"

#ifdef PLATFORM_LA64
UINT32 offset = 0x3D000; //smbiosheader addr
UINT32 nameoffset = 0x4E000; //nameheader addr
#elif defined(PLATFORM_MIPS64) // mips64el
UINT32 offset = 0x1E000; //smbiosheader addr
UINT32 nameoffset = 0x2F000; //nameheader addr
#else
UINT32 offset = 0x3D000; //smbiosheader addr
UINT32 nameoffset = 0x4E000; //nameheader addr
#endif

#pragma pack(1)

typedef struct {
    UINT8     Type;
    UINT8     StrNum;
    UINT64    HOffset;
    UINT64    NOffset;
    UINT64    ResultH;
    UINT64    ResultL;
    UINT32    StrSize;
} SmbiosHeader;
#pragma pack()

const char *commandLineHelp =
    "\n"
    "Usage: spi -s (-type) number (-number) number (-string) string\n"
    "\n"
    "Now only support below command, Examples:\n"
    "  spi -s 1 1 Manufacture                               # update smbios type1 Manufacture\n"
    "  spi -s 1 2 ProductName                               # update smbios type1 ProductName\n"
    "  spi -s 1 3 Version                                   # update smbios type1 Version\n"
    "  spi -s 1 4 SerialNumber                              # update smbios type1 SerialNumber\n"
    "  spi -s 1 5 SKUNumber                                 # update smbios type1 SKUNumber\n"
    "  spi -s 1 6 Family                                    # update smbios type1 Family\n"
    "  spi -s 1 7 0011223344556677 8899aabbccddeeff         # update smbios type1 UUID\n"
    "  spi -s 2 1 Manufacture                               # update smbios type2 Manufacture\n"
    "  spi -s 2 2 Product                                   # update smbios type2 Product\n"
    "  spi -s 2 3 Version                                   # update smbios type2 Version\n"
    "  spi -s 2 4 SerialNumber                              # update smbios type2 SerialNumber\n"
    "  spi -s 2 5 AssetTag                                  # update smbios type2 AssetTag\n"
    "  spi -s 3 1 Manufacture                               # update smbios type3 Manufacture\n"
    "  spi -s 3 2 Version                                   # update smbios type3 Version\n"
    "  spi -s 3 3 SerialNumber                              # update smbios type3 SerialNumber\n"
    "  spi -s 3 4 AssetTag                                  # update smbios type3 AssetTag\n"
    "  spi -s 3 5 SKUNumber                                 # update smbios type3 SKUNumber\n"
    "\n"
    ;

extern UINT64 SPI_REG_BASE;
VOID  SpiFlashSafeWrite (
        UINTN        Offset,
        VOID         *Buffer,
        UINTN        Num
        );

int spi_update_smbios(const char *addr)
{
    void * p = NULL;
    unsigned long long devaddr;
    char RecordName[0x40];
    char RecordName1[0x40];
    wchar_t RecordName2[16];
    wchar_t RecordName3[16];
    int status;
    SmbiosHeader *smbiosheader;
    UINT64 t = 0;
    UINT64 n = 0;
    UINT64 s = 0;
    UINT64 l = 0;
    UINT32 size = 1024*4;
    UINT32 namesize = 1024*4;
    UINT32 Index;
    UINT8  temptype;
    UINT8  tempnum;

    Index = 0;
    smbiosheader = NULL;
    if(addr != NULL){
        sscanf (addr,"%lx", &devaddr);
        devaddr &= 0xfffffffffffffff0ULL;
        offset = 0x41000;   //7A flash smbiosheader offset
        nameoffset = 0x52000;   //7A flash nameheader offset
    } else {
        devaddr = 0x1fe001f0;
    }

    int fd = open ("/dev/mem", O_RDWR|O_SYNC);
    if(fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    /*Transfer Virtul to Phy Addr*/
    p = vtpa(devaddr,fd);
    SPI_REG_BASE = (UINTN)p;

    printf("%s", commandLineHelp);

    printf("Please input smbios type (only support 1/2/3):");

    status = scanf("%s",RecordName); //1.smbios type
    if (RecordName != NULL) {
        //printf("RecordName: (%s)\n", RecordName);
        t = atoi(RecordName);
        printf("t: (%x)\n", t);
        if ((t > 3) || (t <= 0)){
            printf("Unsupport smbios type! Please retry!\n");
            return 1;
        }

        printf("\n");
        printf("Note: if you input smbiostype is 1 in the previous step, the string number only support 1/2/3/4/5/6/7.\n");
        printf("      if you input smbiostype is 2 or 3 in the previous step, the string number only support 1/2/3/4/5.\n");
        printf("\n");
        printf("Please input smbios string number (only support 1/2/3/4/5/6/7):");

        status = scanf("%s",RecordName1); //2. smbios string number
        if (RecordName1 != NULL) {
            //printf("RecordName1: (%s)\n", RecordName1);
            n = atoi(RecordName1);
            printf("n: (%x)\n", n);
            if ((n > 7) || (n <= 0)){
                printf("Unsupport string number! Please retry!");
                return 1;
            }

            if ((t > 1) && (n > 5)){
                printf("Unsupport string number! Please retry!");
                return 1;
            }

            if ((t == 1) && (n == 1))  Index = 0;
            if ((t == 1) && (n == 2))  Index = 1;
            if ((t == 1) && (n == 3))  Index = 2;
            if ((t == 1) && (n == 4))  Index = 3;
            if ((t == 1) && (n == 5))  Index = 4;
            if ((t == 1) && (n == 6))  Index = 5;
            if ((t == 1) && (n == 7))  Index = 6;
            if ((t == 2) && (n == 1))  Index = 7;
            if ((t == 2) && (n == 2))  Index = 8;
            if ((t == 2) && (n == 3))  Index = 9;
            if ((t == 2) && (n == 4))  Index = 10;
            if ((t == 2) && (n == 5))  Index = 11;
            if ((t == 3) && (n == 1))  Index = 12;
            if ((t == 3) && (n == 2))  Index = 13;
            if ((t == 3) && (n == 3))  Index = 14;
            if ((t == 3) && (n == 4))  Index = 15;
            if ((t == 3) && (n == 5))  Index = 16;

            offset += Index * size;
            nameoffset += Index * namesize;

            smbiosheader = malloc(sizeof(SmbiosHeader));
            if (smbiosheader == NULL) {
                return 1;
            }

            //printf("(%x)-(0x%lx)-(0x%lx)\n", Index, offset, nameoffset);

            smbiosheader->Type = t;
            smbiosheader->StrNum = n;
            smbiosheader->HOffset = offset;
            smbiosheader->NOffset = nameoffset;

            if (Index == 6) {
                printf("Please Input smbios HEX1(64):");
            } else {
                printf("Please Input smbios strings:");
            }
            status = scanf("%s",RecordName2); //3.smbios string

            if (RecordName2 != NULL) {
                //printf("RecordName2: (%s)-(%d)\n", RecordName2, (strlen((char *)RecordName2) + 1) * 2);
                smbiosheader->StrSize = (strlen((char *)RecordName2) + 1) * 2;
                if (Index == 6) {
                    printf("Please Input smbios HEX2(64):");
                    status = scanf("%s",RecordName3);
                    if (RecordName3 != NULL) {
                        //printf("RecordName3: (%s)\n", RecordName3);
                        char *endptr1;
                        char *endptr2;

                        s = strtoull((char *)RecordName2, &endptr1, 16);
                        l = strtoull((char *)RecordName3, &endptr2, 16);

                        if ((*endptr1 == '\0') && (*endptr2 == '\0')) {
                            //printf("(0x%lx)-(0x%lx)\n", s, l);
                            smbiosheader->ResultH = s;
                            smbiosheader->ResultL = l;
                            printf("\n");
                        } else {
                            printf("Unsupport string! please retry!\n");
                            return 1;
                        }
                    }
                } else {
                    smbiosheader->ResultH = 0;
                    smbiosheader->ResultL = 0;
                }

                SpiFlashSafeWrite(0 + nameoffset, RecordName2, namesize);
                SpiFlashSafeWrite(0 + offset, smbiosheader, size);
            }
        }
    }

    status = releaseMem(p);
    close(fd);
    free(smbiosheader);
    printf("Program OK.\n");

    return status;
}
