#ifndef __DEF_H_
#define __DEF_H_
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "debug.h"
#include "util.h"
#include "argparse.h"

#define PROGRAM_NAME "OsTools"
#define PROGRAM_VERSION "1.0"

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

int cmd_ht (int argc, const char **argv);
int cmd_mps (int argc, const char **argv);
int cmd_spd (int argc, const char **argv);
int cmd_spi (int argc, const char **argv);
int cmd_rtc (int argc, const char **argv);
int cmd_pci (int argc, const char **argv);
int cmd_acpi (int argc, const char **argv);
int cmd_conf (int argc, const char **argv);
int cmd_gpio (int argc, const char **argv);

#endif
