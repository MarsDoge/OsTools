#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "debug.h"

int memmask;
int memoffset;
void *vtpa(unsigned long long vaddr,int fd)
{
  void *p = NULL;
  memmask = vaddr & ~(0xfff);
  memoffset = vaddr & (0xfff);
	p = (void*)mmap(NULL,1, PROT_READ|PROT_WRITE,MAP_SHARED,fd,memmask);
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
