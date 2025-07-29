// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "def.h"
#include "argparse.h"
#include "file.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static const char *const usages[] = {
    PROGRAM_NAME " <command> [<args>]\n"
    "\nAll commands:\n"
    "    conf       Config\n"
    "    gpio       GPIO\n"
    "    rtc        Read or write rtc\n"
    "    acpi       Read ACPI or Reboot\n"
    "    spi        Read or write spi flash\n"
    "    pci        Read PCI Bus\n"
    "    ht         Read HT\n"
    "    mps        Read or write MPS\n"
    "    spd        Read SPD\n",
    "    fastboot   Read or set fastboot mode\n",
    "    lcl        Read LCL\n",
    "    chip       Read Chip\n",
    NULL,
};

static struct cmd_struct commands[] = {
    {"acpi", cmd_acpi},
    {"conf", cmd_conf},
    {"rtc", cmd_rtc},
    {"spi", cmd_spi},
    {"pci", cmd_pci},
    {"gpio", cmd_gpio},
    {"ht", cmd_ht},
    {"mps", cmd_mps},
    {"spd", cmd_spd},
    {"temp", cmd_temp},
    {"avs", cmd_avs},
    {"fastboot", cmd_fastboot},
    {"lcl", cmd_lcl},
    {"ecc", cmd_ecc},
    {"chip", cmd_chip_model},
};

int main (int argc, const char *argv[])
{
    int version = 0;
    struct argparse argparse;
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_BOOLEAN('v', "version", &version, "show version", NULL, 0, 0),
        OPT_END(),
    };
    argparse_init(&argparse, options, usages, 1);
    argc = argparse_parse(&argparse, argc, argv);

    if (version) {
        printf("%s %s\n", PROGRAM_NAME, PROGRAM_VERSION);
        printf("compile git commit:%s \n", GIT_COMMIT_ID);
        return 0;
    }

    if (argc < 1) {
        argparse_usage(&argparse);
        return -1;
    }

    /*create file: cpu_name*/
    system("cat /proc/cpuinfo | grep 'model name' | awk 'NR==1 {print $4}' > "FILE_NAME_1);

    /* Try to run command with args provided. */
    struct cmd_struct *cmd = NULL;
    for (int i = 0; i < ARRAY_SIZE(commands); i++) {
        if (strcmp(commands[i].cmd, argv[0]) == 0) {
            cmd = &commands[i];
        }
    }

    if (cmd) {
        return cmd->func(argc, argv);
    }
    return 0;
}
