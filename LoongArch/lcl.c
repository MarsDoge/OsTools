// SPDX-License-Identifier: GPL-2.0
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include "def.h"

#define CPU_LCL_CHECK_RETRY 0xa0000010000ULL

static const char *const lcl_usages[] = {
	PROGRAM_NAME" lcl <args>",
	NULL,
};

static int check_lcl_retry (int Number, bool watch_phy)
{
	/*Save Input History Record*/
	FILE *pfile = fopen("./ToolRecord.txt", "a+");
	void *p = NULL;
	unsigned long long node_id = 0;
	unsigned long long vaddr = 0;
	unsigned int c = 0;
	int fd;

	fd = open ("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		printf("can't open file,please use root .\n");
		return 1;
	}

	do {
		vaddr = CPU_LCL_CHECK_RETRY | (node_id << 44);
		for (int i = 0; i < 4; i++) {
			vaddr = CPU_LCL_CHECK_RETRY | (node_id << 44) | (i << 28ULL);
			p = (void*)mmap(NULL,0x1000/*4K*/, PROT_READ|PROT_WRITE,MAP_SHARED,fd,vaddr);
			c =  *(volatile unsigned int *)(p + 0xc4); //retry
			printf("NodeId:%d: LclId:%d checkretry 0xc4: %x \n", node_id, i, c);
			fwrite( (void *)&c, sizeof(unsigned int),1,pfile);
			if (watch_phy) {
				c =  *(volatile unsigned int *)(p + 0xb8); //retry
				printf("                 ----watch phy 0xb8: %x \n", c);
				fwrite( (void *)&c, sizeof(unsigned int),1,pfile);
				c =  *(volatile unsigned int *)(p + 0xbc); //retry
				printf("                 ----watch phy 0xbc: %x \n", c);
				fwrite( (void *)&c, sizeof(unsigned int),1,pfile);
			}
			munmap (p,0x1000);
		}
		node_id++;
	} while (node_id < Number);

	close(fd);
	return 0;
}

int cmd_lcl (int argc, const char **argv)
{
	int check_retry = 0;
	int Number = 0;
	int Mode = 0;
	bool watch_phy = 0;
	uid_t uid;
	struct argparse argparse;

	struct argparse_option options[] = {
		OPT_HELP(),
		OPT_BOOLEAN ('c', "check retry", &check_retry, "check lcl retry", NULL, 0, 0),
		OPT_INTEGER ('n', "Numer", &Number, "Lcl Tot Number", NULL, 0, 0),
		OPT_INTEGER ('m', "Mode", &Mode, "check mode", NULL, 0, 0),
		OPT_END(),
	};

	argparse_init(&argparse, options, lcl_usages, 0);
	argc = argparse_parse(&argparse, argc, argv);

	switch (Mode)
	{
		case 1:
		 watch_phy = true;
		 break;

		default:
		 break;
	}


	if (!(check_retry)) {
		argparse_usage(&argparse);
		return 1;
	}

	uid = geteuid ();
	if (uid != 0) {
		printf("Please run with root!\n");
		return -1;
	}

	if (check_retry) {
		check_lcl_retry (Number, watch_phy);
	}
	return 0;
}
