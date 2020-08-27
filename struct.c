#include "def.h"



/*  return devtail
 *  //if return is NULL, no dev in DevList.
 *  if return is not NULL, and if return.name is NULL,this list is no create.
 *  if return is not NULL, and if return.name is not NULL, this taildev is returned.
 * */
DevNode *GetDevListTail(void)
{
printf(" %s %d  \n",__func__,__LINE__);
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
printf(" %s %d  \n",__func__,__LINE__);
      TmpNode = DevListInstance->nextdev;
printf(" %s %d  \n",__func__,__LINE__);
      while(TmpNode->nextdev != NULL)
      {
        TmpNode = TmpNode->nextdev;
      }
printf(" %s %d  \n",__func__,__LINE__);
      return TmpNode;
    }
  }
}

int DevNodeInsert(DevNode *DevNodeInstance)
{
  DevNode *TailNode;

  printf(" %s %d  \n",__func__,__LINE__);
  TailNode = GetDevListTail();
  if(TailNode == NULL)
  {
    printf("Error: Tail is not must NULL!!!!!!\n");
    return 1;
  }
  printf(" %s %d  \n",__func__,__LINE__);
  TailNode->nextdev = DevNodeInstance;
  DevNodeInstance->prevdev = TailNode;
  printf("Insert Success: DevNodeInstance insert in List.\n");
  return 0;
}

void DevInstanceInsert(DevNode *DevInstance){
  int status;

printf(" %s %d  \n",__func__,__LINE__);
  status = DevNodeInsert (DevInstance);
printf(" %s %d  \n",__func__,__LINE__);
  if(status == 1){
    printf("Error:%s Instance Insert Fail!!!\n",DevInstance->devname);
    while(1);
  }
}

DevNode* GetDevNodeInstance(char *devname)
{
  DevNode *NodeTmp = NULL;

  if(DevListInstance == NULL){
    printf("Error: List is NULL,Not Allocate!!!\n");
    return NULL;
  }else{
    NodeTmp = DevListInstance->nextdev;
  printf(" %s %d  \n",__func__,__LINE__);
    while(NodeTmp!=NULL)
    {
      if(!strcmp(NodeTmp->devname,devname)){
  printf(" %s %d  \n",__func__,__LINE__);
        //printf("Error: Next Node is NULL,please be careful!!!\n");
        /*match dev success*/
        return NodeTmp;
        //break;
      }else{
  printf(" %s %d  \n",__func__,__LINE__);
        NodeTmp = NodeTmp->nextdev;
        continue;
      }
    }
  printf(" %s %d  \n",__func__,__LINE__);
    if(NodeTmp == NULL){
      printf("Warning: Not dev did not mattch !!!\n");
      return NULL;
    }
  }
}


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
