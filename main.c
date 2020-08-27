
#include "def.h"
#include "acpi.h"
#include "rtc.h"

DevList *DevListInstance = NULL;


int main(int argc,char *argv[]){
	int tmp = 0,flag = 0,status = 0;
	if(argc!=1){
		printf("Access Func Fail, please get help !!! \n");
		return;
	}

  int fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0){
		printf("can't open file,please use root .\n");
		exit(1);
	}
  /*connect dev and cmd list*/
  AcpiInitInstance();
printf(" %s %d  \n",__func__,__LINE__);
  RtcInitInstance();
printf(" %s %d  \n",__func__,__LINE__);
  
	/*Draw Dev and Cmd Tree*/
  DrawDevTree();
printf(" %s %d  \n",__func__,__LINE__);

	/*Test*/
	//((AcpiRead)(DevListInstance->nextdev->CmdInstance[0].CmdOps))(fd);
	((AcpiRead)(DevListInstance->nextdev->nextdev->CmdInstance[0].CmdOps))(fd);
  //-------Only Rw-----------------
  
    //  printf("%s RW_FUNC Support, please enter access ..\n",funcSet[j].regname);
		//printf("Function not currently supported, Please contact the developer !!!\n");
	

  //((AcpiRead)(DevListInstance->nextdev->CmdInstance[0].CmdOps))(fd);
  //printf("Please Input Reg Name:  ");
#if 0
  //-----------func  select------------------
	
	else{
    for(j = 0;j<(sizeof(funcSet)/sizeof(funcstruct));j++){
      if(funcSet[j].setflag == 1)
      {
        printf("Find function succeeded :%s...\n",funcSet[j].regname);
        funcSet[j].Func(fd);
				break;
      }
      if((j == (sizeof(funcSet)/sizeof(funcstruct))) && funcSet[j].setflag != 0)
      {
         printf("####### Find function Fail , not such Func !!! ########\n ");
      }
		}

		printf("--------------end----------------\n");
	}

	return 0;
#endif
	close(fd);
}

