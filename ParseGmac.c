#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
#define uint unsigned int 
#define uchar unsigned char
typedef int bool;
#define true  1
#define false 0

#define MAC_ADDRESS_LEN 6		/* mac地址长为6位十六进制数*/
 
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
}STR_PARSE_INFO_S;
 
/* Mac 地址以字符串呈现的四种不同方式 */
STR_PARSE_INFO_S g_astMacInfo[MAC_FORMAT_BUTT]=	
{
	{"%c%c%c%c%c%c-%c%c%c%c%c%c",13},
	{"%c%c%c%c-%c%c%c%c-%c%c%c%c",14},
	{"%c%c-%c%c-%c%c-%c%c-%c%c-%c%c",17},
	{"%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",17},
	{"",0}
};
bool str_2_hex(char *szData,uchar *pucHex)
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
bool ParseMacForStr(Mac_Format enMacfmt,char *szMac,uchar *pucMac)
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
 
uchar aucResMac[MAC_ADDRESS_LEN+1]={0};
void* parse_mac(char *szMacStr)
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
  printf("------------3-----------\n");
	return aucResMac;
}

