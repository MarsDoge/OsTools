#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "def.h"

static const char *const fastboot_usages[] = {
    PROGRAM_NAME" fastboot <args>",
    NULL,
};

static void fastboot_read (void)
{
    FILE *fp;
    char buf[10];

    fp = popen("efivar -d --name d2be5712-4512-5cdd-b9df-f6239a493216-FastBootCfg", "r");
    fgets(buf, sizeof(buf), fp);
    switch(buf[0]) {
        case '1':
            printf (" Current boot mode: fastboot mode \n");
            break;
        case '2':
            printf (" Current boot mode: ultra fastboot mode \n");
            break;
        default:
            printf (" Current boot mode: normal mode \n");
            break;
    }
    pclose(fp);
}

static void fastboot_set (unsigned int mode)
{
    FILE *fp = NULL;
    unsigned char buffer[2];

    buffer[0] = (unsigned char)mode;
    buffer[1] = 0;

    printf (" Config boot mode done \n");
    fp = fopen(".LsBootMode.bin", "wb");
    fwrite(buffer, sizeof(unsigned char), sizeof(buffer), fp);
    fclose(fp);
    system("efivar -w --name d2be5712-4512-5cdd-b9df-f6239a493216-FastBootCfg  -f .LsBootMode.bin ");
    fastboot_read();
    //system("rm .LsBootMode.bin ");
}

int cmd_fastboot (int argc, const char **argv)
{
    int read = 0;
    int set = 0;
    unsigned int mode = 0;
    struct argparse argparse;

    struct argparse_option options[] = {
        OPT_HELP (),
        OPT_GROUP ("Options:"),
        OPT_BOOLEAN ('r', "read", &read, "read fastboot mode", NULL, 0, 0),
        OPT_BOOLEAN ('s', "set",  &set,  "set fastboot mode", NULL, 0, 0),
        OPT_GROUP ("Arguments:"),
        OPT_INTEGER ('m', "mode", &mode, "boot mode(0:normal, 1:fastboot, 2:ultra fastboot)", NULL, 0, 0),
        OPT_END (),
    };

    argparse_init (&argparse, options, fastboot_usages, 0);
    argc = argparse_parse (&argparse, argc, argv);

    if ((!(read || set)) || (mode > 2)) {
        argparse_usage (&argparse);
        return 1;
    }

    if (read) {
        fastboot_read ();
    }
    if (set) {
        fastboot_set (mode);
    }
    return 0;
}
