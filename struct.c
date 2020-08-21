#include "def.h"



/*  return devtail
 *  //if return is NULL, no dev in DevList.
 *  if return is not NULL, and if return.name is NULL,this list is no create.
 *  if return is not NULL, and if return.name is not NULL, this taildev is returned.
 * */
DevNode *GetDevListTail(void)
{
  DevNode *TmpNode;
  if (DevListInstance == NULL) {
    printf("DevListInstance is NULL,created! \n");
    //create Instance
    DevListInstance  = (DevList *) malloc (sizeof (DevList));
    DevListInstance->devname = NULL;
    DevListInstance->devaddr = 0;
    DevListInstance->nextdev = NULL;
    DevListInstance->prevdev = NULL;
    return DevListInstance;
  } else {
    if (DevListInstance->nextdev == NULL) {
      printf("DevList next no Dev ,is NULL,return DevList!\n");
      return DevListInstance;
    } else {
      TmpNode = DevListInstance->nextdev;
      while (TmpNode!=NULL)
      {
        TmpNode = TmpNode->nextdev;
      }
      return TmpNode->prevdev;
    }
  }
}

int DevNodeInsert(DevNode *DevNodeInstance)
{
  DevNode *TailNode;

  TailNode = GetDevListTail();
  if(TailNode == NULL)
  {
    printf("Error: Tail is not must NULL!!!!!!\n");
    return 1;
  }
  TailNode->nextdev = DevNodeInstance;
  printf("Insert Success: DevNodeInstance insert in List.\n");
  return 0;
}

void DevInstanceInsert(DevNode *DevInstance){
  int status;

  status = DevNodeInsert (DevInstance);
  if(status == 1){
    printf("Error:%s Instance Insert Fail!!!\n",DevInstance->devname);
    while(1);
  }
}


   // printf("line:%d, func:%s \n",__LINE__,__func__);

void DrawDevTree(void)
{
  int TreeNum = 0;
  Cmd *CmdTmp ;
  DevNode *TmpNode = DevListInstance->nextdev;
  printf("############ Func Support : ");
  while(TmpNode!=NULL){
    printf("%s ",TmpNode->devname);
    TreeNum++;
    TmpNode = TmpNode->nextdev;
  }
  printf("total amount:%d",TreeNum);
  printf("\n");

  TmpNode = DevListInstance->nextdev;
  TreeNum = 0;
  while(TmpNode!=NULL){
    /*Detailed commands*/
    printf("DevId:%2d :%5s ",TreeNum,TmpNode->devname);
    CmdTmp = TmpNode->CmdInstance;
    printf("Support ");
    while(CmdTmp->CmdName!=NULL){
      printf(" Cmd:%s, Ops:%p",CmdTmp->CmdName,CmdTmp->CmdOps);
      CmdTmp++;
    }
    TmpNode = TmpNode->nextdev;
    printf("\n");
  }
}
