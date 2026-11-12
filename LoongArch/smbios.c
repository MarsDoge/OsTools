// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
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
// smbios info range max size in byte
UINT32 grangemaxsize = 0x3000;

#define SMBIOS_STORE_VERSION         1
#define SMBIOS_STORE_SIGNATURE       0x45534253

#define SMBIOS_STORE_TOTAL_LENGTH(a)  ((sizeof(((SMBIOS_STORE *)(a))->Header) + ((SMBIOS_STORE *)(a))->Header.BodyLength))
#define BODY_ALIGNMENT  4
#define ALIGN_VALUE_ADDEND(Value, Alignment)  (((Alignment) - (Value)) & ((Alignment) - 1U))
#define ALIGN_VALUE(Value, Alignment)  ((Value) + ALIGN_VALUE_ADDEND (Value, Alignment))

typedef enum {
  bodytype_string = 0,
  bodytype_uuid   = 1
} SMBIOS_STORE_BODYTYPE;

typedef enum {
  bodystatus_invalid = 0
} SMBIOS_STORE_BODYSTATUS;

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

typedef struct {
  UINT32    Signature;
  UINT16    Version;
  UINT16    BodyNumber;
  UINT32    BodyLength;
  UINT16    BodyChecksum;
  UINT16    HeaderChecksum;
} SMBIOS_STORE_HEADER;

typedef struct {
  UINT32    Length;
  UINT8     SmbiosType;
  UINT8     SmbiosStringIndex;
  UINT8     BodyType;
  UINT8     BodyStatus;
  UINT8     Body[];
} SMBIOS_STORE_BODY;

typedef struct {
  SMBIOS_STORE_HEADER   Header;
  SMBIOS_STORE_BODY     BodyList[];
} SMBIOS_STORE;
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
    "  spi -s 1 7 00112233-4455-6677-8899-AABBCCDDEEFF      # update smbios type1 UUID\n"
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
UINTN SpiFlashRead (
        UINTN      Offset,
        VOID       *Buffer,
        UINTN      Num
        );

UINT16
CalculateSum16 (
  UINT16  *Buffer,
  UINTN   Length
  )
{
  UINT16  Sum;
  UINTN   Count;
  UINTN   Total;

  if (Buffer == NULL) return -1;
  if (((UINTN)Buffer & 0x1) != 0) return -1;
  if ((Length & 0x1) != 0) return -1;
  if (Length > (0xFFFFFFFFFFFFFFFFULL - ((UINTN)Buffer) + 1)) return -1;

  Total = Length / sizeof (*Buffer);
  for (Sum = 0, Count = 0; Count < Total; Count++) {
    Sum = (UINT16)(Sum + *(Buffer + Count));
  }

  return Sum;
}

int checksumstore (SMBIOS_STORE *smbiosstore)
{
    if (smbiosstore == NULL) {
        return -1;
    }

    smbiosstore->Header.BodyChecksum = CalculateSum16((UINT16 *)smbiosstore->BodyList, smbiosstore->Header.BodyLength);
    smbiosstore->Header.HeaderChecksum = 0;
    smbiosstore->Header.HeaderChecksum = CalculateSum16((UINT16 *)&smbiosstore->Header, sizeof(SMBIOS_STORE_HEADER));

    return 0;
}

int get_body_info (SMBIOS_STORE *smbiosstore, UINT32 *Length, UINT16 *Number)
{
    SMBIOS_STORE_BODY *Item;
    UINT32 BodyLength = 0;
    UINT16 BodyNumber = 0;

    if ((smbiosstore == NULL) || (Length == NULL) || (Number == NULL)) {
        return -1;
    }

    Item = smbiosstore->BodyList;
    while ((Item->Length != 0) && (Item->Length != (UINT32)(-1)) && (BodyLength < smbiosstore->Header.BodyLength)) {
        BodyLength += Item->Length;
        BodyNumber ++;
        Item = (SMBIOS_STORE_BODY *)((UINT8 *)Item + Item->Length);
    }
    *Length = BodyLength;
    *Number = BodyNumber;

    return 0;
}

int get_body_item (SMBIOS_STORE *smbiosstore, UINT8 Type, UINT8 Index, SMBIOS_STORE_BODY **FindBody)
{
    SMBIOS_STORE_BODY *Item;
    SMBIOS_STORE_BODY *FindItem;
    UINT32            BodyOffset;

    if ((smbiosstore == NULL) || (FindBody == NULL)) {
        return -1;
    }

    Item       = smbiosstore->BodyList;
    FindItem   = NULL;
    BodyOffset = 0;
    while ((Item->Length != 0) && (Item->Length != (UINT32)(-1)) && (BodyOffset < smbiosstore->Header.BodyLength)) {
        if (Item->SmbiosType == Type && Item->SmbiosStringIndex == Index) {
            FindItem = Item;
            break;
        }
        BodyOffset += Item->Length;
        Item = (SMBIOS_STORE_BODY *)((UINT8 *)Item + Item->Length);
    }

    if (FindItem != NULL) {
        *FindBody = FindItem;
        return 0;
    } else {
        return -1;
    }
}

int new_body_item (SMBIOS_STORE *smbiosstore, UINT8 Type, UINT8 Index, char *bodystring, SMBIOS_STORE_BODY **newbody)
{
    SMBIOS_STORE_BODY *body;
    UINT32            bodylength;

    if ((smbiosstore == NULL) || (bodystring == NULL) || (newbody == NULL)) {
        return -1;
    }

    bodylength = ALIGN_VALUE (sizeof(SMBIOS_STORE_BODY) + strlen(bodystring) + 1, BODY_ALIGNMENT);
    body = (SMBIOS_STORE_BODY *) malloc (bodylength);
    if (body == NULL) {
        return -1;
    }
    memset(body, 0, bodylength);

    // init new body
    body->Length = bodylength;
    body->SmbiosType = Type;
    body->SmbiosStringIndex = Index;
    if ((Type == 1) && (Index == 7)) {
        body->BodyType = bodytype_uuid;
    } else {
        body->BodyType = bodytype_string;
    }
    body->BodyStatus = 0;
    memcpy(body->Body, bodystring, strlen(bodystring));

    *newbody = body;

    return 0;
}

int cleanup_body_item (SMBIOS_STORE *smbiosstore)
{
    SMBIOS_STORE      *newstore;
    SMBIOS_STORE_BODY *Item;
    UINT32 BodyOffset = 0;
    UINT32 BodyNumber = 0;
    UINT32 BodyLength = 0;

    if (smbiosstore == NULL) {
        return -1;
    }

    newstore = (SMBIOS_STORE *) malloc (SMBIOS_STORE_TOTAL_LENGTH(smbiosstore));
    if (newstore == NULL) {
        return -1;
    }
    memcpy (newstore, &smbiosstore->Header, sizeof(SMBIOS_STORE_HEADER));

    Item = smbiosstore->BodyList;
    while ((Item->Length != 0) && (Item->Length != (UINT32)(-1)) && (BodyOffset < smbiosstore->Header.BodyLength)) {
        if ((Item->BodyStatus & (1 << bodystatus_invalid)) == 0) {
            memcpy ((UINT8 *) newstore->BodyList + BodyLength, Item, Item->Length);
            BodyNumber ++;
            BodyLength += Item->Length;
        }
        BodyOffset += Item->Length;
        Item = (SMBIOS_STORE_BODY *)((UINT8 *)Item + Item->Length);
    }

    memset (smbiosstore, 0, SMBIOS_STORE_TOTAL_LENGTH(smbiosstore));
    memcpy (smbiosstore, newstore, sizeof(SMBIOS_STORE_HEADER) + BodyLength);
    smbiosstore->Header.BodyLength = BodyLength;
    smbiosstore->Header.BodyNumber = BodyNumber;
    checksumstore(smbiosstore);

    free(newstore);

    return 0;
}

int combine_body_item (SMBIOS_STORE *smbiosstore, SMBIOS_STORE_BODY *body, SMBIOS_STORE **newstore)
{
    SMBIOS_STORE      *store;
    UINT32            newlength;

    if ((smbiosstore == NULL) || (body == NULL) || (newstore == NULL)) {
        return -1;
    }

    newlength = SMBIOS_STORE_TOTAL_LENGTH(smbiosstore) + body->Length;

    // combine new body
    store = (SMBIOS_STORE *) malloc (newlength);
    if (store == NULL) {
        return -1;
    }
    memcpy(store, smbiosstore, SMBIOS_STORE_TOTAL_LENGTH(smbiosstore));
    memcpy((UINT8 *)store + SMBIOS_STORE_TOTAL_LENGTH(smbiosstore), body, body->Length);

    // Reorganize the "body" data and remove the invalid data.
    store->Header.BodyLength += body->Length;
    store->Header.BodyNumber ++;
    checksumstore(store);

    *newstore = store;

    return 0;
}

int check_guid_format_valid(const char *s)
{
    if (!s) return -1;

    const char *p = s;
    int dash_pos[] = {8, 13, 18, 23};
    int dash_idx = 0;

    for (int i = 0; i < 36; i++, p++)
    {
        if (*p == '\0') return -1;
        if (dash_idx < 4 && i == dash_pos[dash_idx])
        {
            if (*p != '-') return -1;
            dash_idx++;
        }
        else
        {
            if (!isxdigit((unsigned char)*p))
                return -1;
        }
    }

    if (*p != '\0') return -1;

    return 0;
}

int check_smbiosstore_valid(SMBIOS_STORE *smbiosstore)
{
    int status;
    UINT32 BodyLength = 0;
    UINT16 BodyNumber = 0;
    UINT16 checksum16;

    if (smbiosstore->Header.Signature != SMBIOS_STORE_SIGNATURE) {
        return -1;
    }

    status = get_body_info(smbiosstore, &BodyLength, &BodyNumber);
    if ((status == 0) && ((smbiosstore->Header.BodyLength != BodyLength) || (smbiosstore->Header.BodyNumber != BodyNumber))) {
        return -1;
    }

    checksum16 = smbiosstore->Header.HeaderChecksum;
    smbiosstore->Header.HeaderChecksum = 0;
    smbiosstore->Header.HeaderChecksum = CalculateSum16((UINT16 *)&smbiosstore->Header, sizeof(SMBIOS_STORE_HEADER));
    if (smbiosstore->Header.HeaderChecksum != checksum16) {
        smbiosstore->Header.HeaderChecksum = checksum16;
        return -1;
    }

    checksum16 = CalculateSum16((UINT16 *)smbiosstore->BodyList, smbiosstore->Header.BodyLength);
    if (smbiosstore->Header.BodyChecksum != checksum16) {
        return -1;
    }

    return 0;
}

int smbiosstore_select_version ()
{
    void *p = NULL;
    unsigned long long devaddr;
    UINT32 Signature;

    int fd = open ("/dev/mem", O_RDWR|O_SYNC);
    if(fd < 0) {
        printf("can't open file,please use root .\n");
        return 1;
    }

    devaddr = 0x1fe001f0;
    /*Transfer Virtul to Phy Addr*/
    p = vtpa(devaddr, fd);
    SPI_REG_BASE = (UINTN)p;

    SpiFlashRead (offset, &Signature, sizeof(Signature));
    releaseMem(p);
    close(fd);

    if (Signature == SMBIOS_STORE_SIGNATURE) {
        return 1;
    } else {
        return 0;
    }
}

int smbiosstore_v0 (UINT8 Type, UINT8 StringIndex, char *RecordString)
{
    int status;
    UINT32 Index;
    SmbiosHeader *smbiosheader;
    UINT32 size = 1024*4;
    UINT32 namesize = 1024*4;
    UINT64 s = 0;
    UINT64 l = 0;
    char uuid_str[18];

    if ((Type == 1) && (StringIndex == 1))  Index = 0;
    if ((Type == 1) && (StringIndex == 2))  Index = 1;
    if ((Type == 1) && (StringIndex == 3))  Index = 2;
    if ((Type == 1) && (StringIndex == 4))  Index = 3;
    if ((Type == 1) && (StringIndex == 5))  Index = 4;
    if ((Type == 1) && (StringIndex == 6))  Index = 5;
    if ((Type == 1) && (StringIndex == 7))  Index = 6;
    if ((Type == 2) && (StringIndex == 1))  Index = 7;
    if ((Type == 2) && (StringIndex == 2))  Index = 8;
    if ((Type == 2) && (StringIndex == 3))  Index = 9;
    if ((Type == 2) && (StringIndex == 4))  Index = 10;
    if ((Type == 2) && (StringIndex == 5))  Index = 11;
    if ((Type == 3) && (StringIndex == 1))  Index = 12;
    if ((Type == 3) && (StringIndex == 2))  Index = 13;
    if ((Type == 3) && (StringIndex == 3))  Index = 14;
    if ((Type == 3) && (StringIndex == 4))  Index = 15;
    if ((Type == 3) && (StringIndex == 5))  Index = 16;

    offset += Index * size;
    nameoffset += Index * namesize;

    smbiosheader = malloc(size);
    if (smbiosheader == NULL) {
        return -1;
    }
    memset(smbiosheader, 0, sizeof(SmbiosHeader));

    //printf("(%x)-(0x%lx)-(0x%lx)\n", Index, offset, nameoffset);

    smbiosheader->Type = Type;
    smbiosheader->StrNum = StringIndex;
    smbiosheader->HOffset = offset;
    smbiosheader->NOffset = nameoffset;
    smbiosheader->StrSize = (strlen((char *)RecordString) + 1) * 2;
    if (Index == 6) {
        int i, j;
        char *endptr1;
        char *endptr2;

        memset(uuid_str, 0, sizeof(uuid_str));
        for (i = 0, j = 0; i < sizeof(uuid_str); i ++)
        {
            if (RecordString[i] == '-')
                continue;
            uuid_str[j++] = RecordString[i];
        }
        s = strtoull((char *)uuid_str, &endptr1, 16);

        memset(uuid_str, 0, sizeof(uuid_str));
        for (i = 0, j = 0; i < sizeof(uuid_str); i ++)
        {
            if (RecordString[i+sizeof(uuid_str)] == '-')
                continue;
            uuid_str[j++] = RecordString[i+sizeof(uuid_str)];
        }
        l = strtoull((char *)uuid_str, &endptr2, 16);

        if ((*endptr1 == '\0') && (*endptr2 == '\0')) {
            // printf("(0x%lx)-(0x%lx)\n", s, l);
            smbiosheader->ResultH = s;
            smbiosheader->ResultL = l;
            printf("\n");
        } else {
            printf("Unsupport string! please retry!\n");
            return 1;
        }
    } else {
        smbiosheader->ResultH = 0;
        smbiosheader->ResultL = 0;
    }

    SpiFlashSafeWrite(0 + nameoffset, RecordString, namesize);
    SpiFlashSafeWrite(0 + offset, smbiosheader, sizeof(SmbiosHeader));
    printf("V0 Program OK.\n");

    return 0;
}

int smbiosstore_v1 (UINT8 Type, UINT8 StringIndex, char *RecordString)
{
    int status;
    SMBIOS_STORE *smbiosstore;
    SMBIOS_STORE_BODY *body;
    UINT8  needcleanup;
    UINT8 *prebuffer, *newbuffer, *tempbuffer;
    UINT32 length;
    UINTN  i;
    UINT32 smbioslength = grangemaxsize;

    body = NULL;
    prebuffer = NULL;
    newbuffer = NULL;
    smbiosstore = NULL;
    needcleanup = 0;

    prebuffer = malloc (sizeof(SMBIOS_STORE));
    smbiosstore = (SMBIOS_STORE *) prebuffer;
    SpiFlashRead (offset, smbiosstore, sizeof(SMBIOS_STORE));
    if (smbiosstore->Header.Signature == SMBIOS_STORE_SIGNATURE) {
        length    = SMBIOS_STORE_TOTAL_LENGTH(smbiosstore);
        prebuffer = malloc (length);
        free(smbiosstore);
        smbiosstore = (SMBIOS_STORE *) prebuffer;
        SpiFlashRead (offset, smbiosstore, length);
    }
    if (check_smbiosstore_valid(smbiosstore) != 0) {
        printf("init smbios store area!\n");
        memset(smbiosstore, 0, sizeof(SMBIOS_STORE));
        smbiosstore->Header.Signature = SMBIOS_STORE_SIGNATURE;
        smbiosstore->Header.Version = SMBIOS_STORE_VERSION;
    } else {
        status = get_body_item (smbiosstore, Type, StringIndex, &body);
        if ((status == 0) && (body != NULL)) {
            body->BodyStatus |= (1 << bodystatus_invalid);
            needcleanup = 1;
        }
    }

    body = NULL;
    status = new_body_item (smbiosstore, Type, StringIndex, RecordString, &body);
    if (status == -1) {
        goto done;
    }

    // combine new body
    status = combine_body_item (smbiosstore, body, (SMBIOS_STORE **) &newbuffer);
    if (status == -1) {
        goto done;
    }
    smbiosstore = (SMBIOS_STORE *) newbuffer;

    // update smbiosstore->Header.BodyLength, smbiosstore->Header.BodyNumber and smbiosstore->BodyList
    if (needcleanup) {
        status = cleanup_body_item(smbiosstore);
        if (status == -1) {
            goto done;
        }
    }

    // check max length
    if (SMBIOS_STORE_TOTAL_LENGTH(smbiosstore) > smbioslength) {
        printf("smbioslength is out of range, 0x%x > 0x%x, please check !!! \n", SMBIOS_STORE_TOTAL_LENGTH(smbiosstore), smbioslength);
        status = -1;
        goto done;
    }

    //
    // Save to spi flash, try 3 times
    //
    tempbuffer = malloc (SMBIOS_STORE_TOTAL_LENGTH(smbiosstore));
    for (i = 0; i < 3; i ++)
    {
        SpiFlashSafeWrite(offset, smbiosstore, SMBIOS_STORE_TOTAL_LENGTH(smbiosstore));
        SpiFlashRead (offset, tempbuffer, SMBIOS_STORE_TOTAL_LENGTH(smbiosstore));
        if (memcmp(tempbuffer, smbiosstore, SMBIOS_STORE_TOTAL_LENGTH(smbiosstore)) == 0) {
            break;
        }
    }
    free (tempbuffer);
    if (i == 3) {
        printf("ERROR: V1 Program FAIL!\n");
        status = -1;
    } else {
        printf("V1 Program OK.\n");
        status = 0;
    }

done:
    if (body)
        free(body);
    if (prebuffer)
        free(prebuffer);
    if (newbuffer)
        free(newbuffer);

    return status;
}

int spi_update_smbios(const char *addr, unsigned int maxsize, int command, const char **argv)
{
    void * p = NULL;
    unsigned long long devaddr;
    char RecordName[0x40];
    char RecordName1[0x40];
    char RecordName2[4096];
    int status;
    UINT64 t = 0;
    UINT64 n = 0;
    int  version;

    // check version on bios flash
    version = smbiosstore_select_version();

    if(addr != NULL){
        sscanf (addr,"%lx", &devaddr);
        devaddr &= 0xfffffffffffffff0ULL;
        offset = 0x41000;   //7A flash smbiosheader offset
        nameoffset = 0x52000;   //7A flash nameheader offset
    } else {
        devaddr = 0x1fe001f0;
    }
    if (maxsize != 0) {
        grangemaxsize = maxsize;
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

    memset(RecordName, 0, sizeof(RecordName));
    memset(RecordName1, 0, sizeof(RecordName1));
    memset(RecordName2, 0, sizeof(RecordName2));
    if (command) {
        if (argv[0]) {
            status = sscanf(argv[0],"%63[^\n]",RecordName);
            printf("param0: %s\n", RecordName);
        }
        if (argv[1]) {
            status = sscanf(argv[1],"%63[^\n]",RecordName1);
            printf("param1: %s\n", RecordName1);
        }
        if (argv[2]) {
            status = sscanf(argv[2],"%4095[^\n]",RecordName2);
            printf("param2: %s\n", RecordName2);
        }
        if (!(argv[0] && argv[1] && argv[2])) {
            printf("This may be a mistake, string is null, please check !!! \n");
            goto done;
        }
    }

    printf("Please input smbios type (only support 1/2/3):");

    if (!command) {
        status = scanf("%63[^\n]%*c",RecordName); //1.smbios type
    }
    if (RecordName != NULL) {
        printf("RecordName: (%s)\n", RecordName);
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

        if (!command) {
            status = scanf("%63[^\n]%*c",RecordName1); //2. smbios string number
        }
        if (RecordName1 != NULL) {
            printf("RecordName1: (%s)\n", RecordName1);
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

            if ((t == 1) && (n == 7)) {
                printf("Please Input smbios GUID format string:");
            } else {
                printf("Please Input smbios strings:");
            }
            if (!command) {
                status = scanf("%4095[^\n]%*c",RecordName2); //3.smbios string
            }

            if ((RecordName2 != NULL) && (strlen((char *)RecordName2) >= 1)) {
                printf("RecordName2: (%s)-(%d)\n", RecordName2, strlen((char *)RecordName2) + 1);
                if ((t == 1) && (n == 7)) {
                    if (check_guid_format_valid (RecordName2)) {
                        printf("guid format error, eg: 00112233-4455-6677-8899-AABBCCDDEEFF, please check !!! \n");
                        goto done;
                    }
                }

                if (version == 0) {
                    status = smbiosstore_v0(t, n, RecordName2);
                } else if (version == 1) {
                    status = smbiosstore_v1(t, n, RecordName2);
                }
            } else {
                printf("This may be a mistake, string is null, please check !!! \n");
                goto done;
            }
        } else {
            printf("This may be a mistake, string is null, please check !!! \n");
            goto done;
        }
    } else {
        printf("This may be a mistake, string is null, please check !!! \n");
        goto done;
    }

done:
    status = releaseMem(p);
    close(fd);

    return status;
}
