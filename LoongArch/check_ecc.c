// SPDX-License-Identifier: GPL-2.0
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "def.h"



#define DEBUG
#ifdef DEBUG
#define dbg_printf(fmt,arg...) printf(fmt,##arg)
#else
#define dbg_printf(fmt,arg...)
#endif

unsigned char *vaddr;

#define inb(addr) *(unsigned int *)(addr)
#define ins(addr) *(unsigned short *)(addr)
#define inl(addr) *(unsigned long long *)(addr)
#define outb(addr, val) *(unsigned int *)(addr) = val

static const char *const ecc_usages[] = {
    PROGRAM_NAME" ecc <args>",
    NULL,
};

int check_ecc(int num)
{
	int fd;
	int reg=0;
	char *str;
	int data=0;
	unsigned long long val = 0;
	unsigned long long val1 = 0;
	unsigned long long i = 0, j;
	
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		printf("\033[0;31m[ERROR]\033[0m Open /dev/mem!\n");
		return -1;
	}
	
	printf("########CHECK ECC########\n");
#if 1
	for(i = 0; i < num; i++) {
	   for(j = 0; j < 4; j++) {
		/*  mmap the pcie config space */
		vaddr = vtpa(0x1fe00000 | (i << 44) | (j << 16), fd);
		if (vaddr < 0) {
			printf("\033[0;31m[ERROR]\033[0m Unable to mmap\n");
			return -1;
		}
		printf("node%d mc%d : \n", i, j);

		val = inl(vaddr + 0x604);
		printf("0x600 : 0x%08x", val);
		val = inl(vaddr + 0x600);
		printf("%08x", val);
		val = inl(vaddr + 0x60c);
		printf("  0x%08x", val);
		val = inl(vaddr + 0x608);
		printf("%08x\n", val);

		val = inl(vaddr + 0x614);
		printf("0x610 : 0x%08x", val);
		val = inl(vaddr + 0x610);
		printf("%08x", val);
		val = inl(vaddr + 0x61c);
		printf("  0x%08x", val);
		val = inl(vaddr + 0x618);
		printf("%08x\n", val);

		val = inl(vaddr + 0x624);
		printf("0x620 : 0x%08x", val);
		val = inl(vaddr + 0x620);
		printf("%08x", val);
		val = inl(vaddr + 0x62c);
		printf("  0x%08x", val);
		val = inl(vaddr + 0x628);
		printf("%08x\n", val);
		releaseMem (vaddr);
   	   }
	}
#endif
	close(fd);

	return 0;
}

int cmd_ecc (int argc, const char **argv)
{
	int check_retry = 0;
	int Number = 0;
	uid_t uid;
	struct argparse argparse;

	struct argparse_option options[] = {
		OPT_HELP(),
		OPT_BOOLEAN ('c', "check retry", &check_retry, "check ecc", NULL, 0, 0),
		OPT_INTEGER ('n', "Numer", &Number, "Node Number", NULL, 0, 0),
		OPT_END(),
	};

	argparse_init(&argparse, options, ecc_usages, 0);
	argc = argparse_parse(&argparse, argc, argv);

	if (!(check_retry)) {
		argparse_usage(&argparse);
		return 1;
	}

	uid = geteuid ();
	if (uid != 0) {
		printf("Please run with root!\n");
		return -1;
	}

	if(Number > 16){
		printf("\033[0;31m[ERROR]\033[0m need node total num param!\n");
		printf("example ./a.out 1/2/4/8/16 \n");
		return -1;
	}

	if (check_retry) {
		check_ecc (Number);
	}

	return 0;
}
