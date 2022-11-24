#include "def.h"
#include "argparse.h"
#include "acpi.h"
#include "rtc.h"
#include "pci.h"
#include "spi.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

DevList *DevListInstance = NULL;

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
//    "    mps0           \n"
//    "    mps1           \n"
//    "    mps2           \n"
//    "    mps3           \n"
//    "    spd0           \n"
//    "    spd1           \n"
//    "    spd2           \n"
//    "    spd3           \n"
    ,NULL,
};

static struct cmd_struct commands[] = {
    {"acpi", cmd_acpi},
    {"conf", cmd_conf},
    {"rtc", cmd_rtc},
    {"spi", cmd_spi},
    {"pci", cmd_pci},
    {"gpio", cmd_gpio},
    {"ht", cmd_ht},
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
        return 0;
    }

    if (argc < 1) {
        argparse_usage(&argparse);
        return -1;
    }

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
