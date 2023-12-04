#ifndef __DEF_H_
#define __DEF_H_
#include "argparse.h"

#ifdef  RELEASE
  #define printfQ(format,...)
#else
  #define printfQ(format,...) printf(format, ##__VA_ARGS__)
#endif

#define PROGRAM_NAME "OsTools"
#define PROGRAM_VERSION "1.2"

#define LS7A_CONF_BASE_ADDR   0x10010000
#define LS7A_MISC_BASE_ADDR   0x10080000

typedef char                INT8;
typedef unsigned char       UINT8;
typedef short               INT16;
typedef unsigned short      UINT16;
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef long long           INT64;
typedef unsigned long long  UINT64;
typedef long long           INTN;
typedef unsigned long long  UINTN;
typedef void                VOID;

struct cmd_struct {
    const char *cmd;
    int (*func) (int, const char **);
};

void* vtpa (unsigned long long vaddr,int fd);
int   releaseMem (void *p);
void  hexdump (unsigned long bse, char *buf, int len);
void* parse_mac (const char *szMacStr);

int cmd_acpi (int argc, const char **argv);
int cmd_conf (int argc, const char **argv);
int cmd_gpio (int argc, const char **argv);
int cmd_ht   (int argc, const char **argv);
int cmd_mps  (int argc, const char **argv);
int cmd_pci  (int argc, const char **argv);
int cmd_rtc  (int argc, const char **argv);
int cmd_spd  (int argc, const char **argv);
int cmd_spi  (int argc, const char **argv);

#endif
