#ifndef __DEF_H_
#define __DEF_H_
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>


#define LS7A_CONF_BASE_ADDR   0x10010000
#define LS7A_MISC_BASE_ADDR   0x10080000
typedef void (*AcpiRead)(int fd);

typedef struct __Cmd{
  char *CmdName;
  void *CmdOps;
}Cmd;

typedef struct __DevNode{
  char    *devname;
  Cmd     *CmdInstance;
  unsigned long long devaddr;
  struct __DevNode *nextdev;
  struct __DevNode *prevdev;
}DevNode;
#define DevList DevNode

extern DevList *DevListInstance;

/*  return devtail
 *  //if return is NULL, no dev in DevList.
 *  if return is not NULL, and if return.name is NULL,this list is no create.
 *  if return is not NULL, and if return.name is not NULL, this taildev is returned.
 * */
DevNode *GetDevListTail(void);


/*
 * Insert Node Dynamic Data.
 * return 0 is success;
 * */
int DevNodeInsert(DevNode *DevNodeInstance);


void DrawDevTree(void);
#endif
