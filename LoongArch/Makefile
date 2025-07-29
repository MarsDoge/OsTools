# SPDX-License-Identifier: GPL-2.0
COMPILE_FILE_C = main.c gpio.c rtc.c acpi.c conf.c pci.c util.c spi.c mps.c i2c.c spd.c ht.c argparse.c process.c smbios.c temp.c avs.c fastboot.c \
		 lcl.c check_ecc.c check_chip_model.c
COMPILE_FILE_O = main.o gpio.o rtc.o acpi.o conf.o pci.o util.o spi.o mps.o i2c.o spd.o ht.o argparse.o process.o smbios.o temp.o avs.o fastboot.o \
		 lcl.o check_ecc.o check_chip_model.c

#COMPILE_PATH =  /opt/LoongArch_Toolchains/loongarch64-linux-gnu-2021-06-19-vector/bin/loongarch64-linux-gnu-gcc
#COMPILE_PATH =  /opt/LoongArch_Toolchains/loongarch64-linux-gnu-2021-06-19-vector/bin/loongarch64-linux-gnu-gcc
COMPILE_PATH =  gcc

GIT_COMMIT_ID := $(shell git rev-parse HEAD)

ARCH := $(shell uname -m)
ifeq ($(ARCH),loongarch64)
CFLAGS = -DPLATFORM_LA64
else ifeq ($(ARCH),mips64)
CFLAGS = -DPLATFORM_MIPS64
else
    $(warning Unsupported architecture: $(ARCH))
endif

all: OsTools
##.PHONY all SEnd
OsTools : OsTools.o
	$(COMPILE_PATH) -static $(COMPILE_FILE_O) -o OsTools
	chmod 777 OsTools
OsTools.o :
	#$(COMPILE_PATH) -g -c $(COMPILE_FILE_C)
	$(COMPILE_PATH) $(CFLAGS) -c $(COMPILE_FILE_C) -DGIT_COMMIT_ID="\"$(GIT_COMMIT_ID)\""

clean :
	rm *.o OsTools
