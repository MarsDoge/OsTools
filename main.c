
#include "def.h"
#include "acpi.h"
#include "rtc.h"

DevList *DevListInstance = NULL;


int main(int argc,char *argv[]){
	int tmp = 0,flag = 0,status = 0;
	if(argc!=1){
		printfQ("Access Func Fail, please get help !!! \n");
		return;
	}

  int fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0){
		printfQ("can't open file,please use root .\n");
		exit(1);
	}
  /*connect dev and cmd list*/
  ConfInitInstance();
  GpioInitInstance();
  RtcInitInstance();
  AcpiInitInstance();

	/*Draw Dev and Cmd Tree*/
  DrawDevTree();

  /*GetFuncDev*/
  DevNode* NodeTmp = NULL;
  printf("Please Input Dev Name:  ");

  /*Create File save it*/
  char RecordName[10] = {0};
  status = scanf("%s",RecordName);
  size_t RecordSize = strlen(RecordName);
  if(RecordSize >= (sizeof(RecordName)/sizeof(char))){
		printf("DevName is too long,please confirm!!! .\n");
    return 1;
  }
  /*Save Input History Record*/
  FILE *pfile = fopen("./.ToolSetRecord.txt", "a+");
  RecordName[9] = '\n';
  fwrite( (void *)RecordName, sizeof(char),10,pfile);
  fclose(pfile);

  NodeTmp = GetDevNodeInstance(RecordName,RecordSize);
  ((DualParam)(NodeTmp->CmdInstance[0].CmdOps))(NodeTmp,fd);




  //-------Only Rw-----------------

    //  printfQ("%s RW_FUNC Support, please enter access ..\n",funcSet[j].regname);
		//printfQ("Function not currently supported, Please contact the developer !!!\n");
	
#if 0
	else{
    for(j = 0;j<(sizeof(funcSet)/sizeof(funcstruct));j++){
      if(funcSet[j].setflag == 1)
      {
        printfQ("Find function succeeded :%s...\n",funcSet[j].regname);
        funcSet[j].Func(fd);
				break;
      }
      if((j == (sizeof(funcSet)/sizeof(funcstruct))) && funcSet[j].setflag != 0)
      {
         printfQ("####### Find function Fail , not such Func !!! ########\n ");
      }
		}

		printfQ("--------------end----------------\n");
	}

	return 0;
#endif
	close(fd);
}

