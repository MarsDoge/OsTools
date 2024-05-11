// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

#define uint unsigned int
#define uchar unsigned char
#define true  1
#define false 0
#define MAC_ADDRESS_LEN 6		/* mac地址长为6位十六进制数*/
typedef int bool;

static unsigned long long memmask;
static int memoffset;
uchar aucResMac[MAC_ADDRESS_LEN+1]={0};

typedef enum tagMac_Format
{
    MAC_FORMAT_2PART = 0,
    MAC_FORMAT_3PART,
    MAC_FORMAT_6PART_1,
    MAC_FORMAT_6PART_2,
    MAC_FORMAT_ANY,
    MAC_FORMAT_BUTT
}Mac_Format;

typedef struct strParseInfo
{
    char *szFmt;
    uint uiLen;
} STR_PARSE_INFO_S;

/* Mac 地址以字符串呈现的四种不同方式 */
STR_PARSE_INFO_S g_astMacInfo[MAC_FORMAT_BUTT]=
{
    {"%c%c%c%c%c%c-%c%c%c%c%c%c",13},
    {"%c%c%c%c-%c%c%c%c-%c%c%c%c",14},
    {"%c%c-%c%c-%c%c-%c%c-%c%c-%c%c",17},
    {"%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",17},
    {"",0}
};

static bool str_2_hex(char *szData,uchar *pucHex)
{
    uint uiLen = 0;
    uint i=0;
    char szByte[3]={0};
    char *pcEndptr = NULL;

    /* 参数检查 */
    if ((NULL == szData) || (NULL == pucHex))
    {
        printf("Invalid points: p1(%p),p2(%p).",szData,pucHex);
        return false;
    }

    uiLen = (uint)strlen(szData);
    if (0 != uiLen %2)		//mac地址应该有12个字符(even)
    {
        printf("Invalid strlen(mac add should be even) : %d\n",uiLen);
        return false;
    }

    for (i =0;i<uiLen;i+=2,pucHex++)
    {
        szByte[0]= szData[i];
        szByte[1]= szData[i+1];
        *pucHex = (uchar)strtoul(szByte,&pcEndptr,16); /* 将字符转化为指定格式(16进制) */
        if (2 != (uint)(pcEndptr-szByte))
        {
            printf("Invalid source str(%s).",szData);
            return false;
        }
    }
    return true;
}
/************************************************************************/
/*
   从字符串中解析Mac地址(转为6位十六进制数)
   */
/************************************************************************/
static bool ParseMacForStr(Mac_Format enMacfmt, const char *szMac, uchar *pucMac)
{
    uint uiBegin = 0;
    uint uiEnd = 0;
    uint uiLoop = 0;
    int iParseCnt =0;
    char szChs[MAC_ADDRESS_LEN*2+1]={0};

    if ((enMacfmt >= MAC_FORMAT_BUTT) || (NULL == szMac) || (NULL == pucMac))
    {
        printf("Invalid inputs: p1(%d),p2(%p),p3(%p)\n",enMacfmt,szMac,pucMac);
        return false;
    }
    if (MAC_FORMAT_ANY == enMacfmt)
    {
        uiBegin = MAC_FORMAT_2PART;
        uiEnd =MAC_FORMAT_6PART_2;
    }
    else
    {
        uiBegin = enMacfmt;
        uiEnd = enMacfmt;
    }
    for (uiLoop = uiBegin;uiLoop<=uiEnd;++uiLoop)
    {
        /* 逐字符解析并存在szChs中 */
        iParseCnt = sscanf(szMac,g_astMacInfo[uiLoop].szFmt,
                &szChs[0],&szChs[1],&szChs[2],&szChs[3],
                &szChs[4],&szChs[5],&szChs[6],&szChs[7],
                &szChs[8],&szChs[9],&szChs[10],&szChs[11]);
        if (MAC_ADDRESS_LEN *2 == iParseCnt) /* 应该解析了12个字符 */
        {
            break;
        }
    }
    /* 转化为十六进制 */
    if (true != str_2_hex(szChs,pucMac))
    {
        printf("Parse failed!\n");
        return false;
    }
    return true;
}

void* parse_mac(const char *szMacStr)
{
    //char szMacStr[20]="aa:bb:cc:dd:12:b3";
    //uchar aucResMac[MAC_ADDRESS_LEN+1]={0};
    uint i = 0;
    if (!ParseMacForStr(MAC_FORMAT_ANY,szMacStr,aucResMac))
    {
        printf("test fun parse failed !");
        return NULL;
    }
    printf("Mac addr is : \n");
    for (i=0;i<MAC_ADDRESS_LEN;++i)
    {
        printf("%x",*(aucResMac+i));
    }
    printf("\n");
    return aucResMac;
}

void *vtpa(unsigned long long vaddr,int fd)
{
    void *p = NULL;
    memmask = vaddr & ~(0xffff);
    memoffset = vaddr & (0xffff);
    p = (void*)mmap(NULL,0x10000/*64K*/, PROT_READ|PROT_WRITE,MAP_SHARED,fd,memmask);
    p = p + memoffset;
    printf("mmap addr start : %p \n",p);
    return p;
}

int releaseMem(void *p)
{
    int status ;
    status = munmap(p-memoffset, 1);
    if(status == -1){
        printf("----------  Release mem Map Error !!! ------\n");
    }
    printf("--------------Release mem Map----------------\n");
    return status;
}

void hexdump (unsigned long bse, char *buf, int len)
{
    int pos;
    char line[80];

    while (len > 0)
    {
        int cnt, i;

        pos = snprintf (line, sizeof (line), "%08lx  ", bse);
        cnt = 16;
        if (cnt > len)
            cnt = len;

        for (i = 0; i < cnt; i++)
        {
            pos += snprintf (&line[pos], sizeof (line) - pos,
                    "%02x ", (unsigned char) buf[i]);
            if ((i & 7) == 7)
                line[pos++] = ' ';
        }

        for (; i < 16; i++)
        {
            pos += snprintf (&line[pos], sizeof (line) - pos, "   ");
            if ((i & 7) == 7)
                line[pos++] = ' ';
        }

        line[pos++] = '|';

        for (i = 0; i < cnt; i++)
            line[pos++] = ((buf[i] >= 32) && (buf[i] < 127)) ? buf[i] : '.';

        line[pos++] = '|';

        line[pos] = 0;

        printf ("%s\n", line);

        /* Print only first and last line if more than 3 lines are identical.  */
        if (len >= 4 * 16
                && ! memcmp (buf, buf + 1 * 16, 16)
                && ! memcmp (buf, buf + 2 * 16, 16)
                && ! memcmp (buf, buf + 3 * 16, 16))
        {
            printf ("*\n");
            do
            {
                bse += 16;
                buf += 16;
                len -= 16;
            }
            while (len >= 3 * 16 && ! memcmp (buf, buf + 2 * 16, 16));
        }

        bse += 16;
        buf += 16;
        len -= cnt;
    }
}
