
#include "def.h"
#include "acpi.h"
#include "rtc.h"

char *VersString = "V1.0";

DevList *DevListInstance = NULL;


int main(int argc,char *argv[]){
	int tmp = 0,flag = 0,status = 0;
  unsigned char index;
	if(argc!=1){
		printfQ("Access Func Fail, please get help !!! \n");
		return;
	}

  int fd = open("/dev/mem",O_RDWR|O_SYNC);
	if(fd<0){
		printfQ("can't open file,please use root .\n");
		exit(1);
	}

  /* OsTools Version */
  printf("*** OsTools Version to %s ***\n",VersString);

	/*connect dev and cmd list*/
	ConfInitInstance();
	GpioInitInstance();
	RtcInitInstance();
	AcpiInitInstance();
	SpiInitInstance();
	PciInitInstance();
	I2cInitInstance();
	SpdInitInstance();
	HtInstance();

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
  /*Get Cmd Num*/
  index = 0;
  while((Cmd*)(NodeTmp->CmdInstance + index)->CmdName != NULL)
  {
    printf("%d : %s \n",index,(Cmd*)(NodeTmp->CmdInstance + index)->CmdName);
    index++;
  }

  printf("Please Input Ops Num...\n");
  status = scanf("%s",RecordName);
  index = atoi(RecordName);
  ((DualParam)(NodeTmp->CmdInstance[index].CmdOps))(NodeTmp,fd);


  close(fd);
}

