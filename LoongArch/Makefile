
COMPILE_FILE_C = main.c gpio.c rtc.c acpi.c conf.c pci.c util.c spi.c mps.c i2c.c spd.c ht.c argparse.c
COMPILE_FILE_O = main.o gpio.o rtc.o acpi.o conf.o pci.o util.o spi.o mps.o i2c.o spd.o ht.o argparse.o

#COMPILE_PATH =  /opt/LoongArch_Toolchains/loongarch64-linux-gnu-2021-06-19-vector/bin/loongarch64-linux-gnu-gcc
#COMPILE_PATH =  /opt/LoongArch_Toolchains/loongarch64-linux-gnu-2021-06-19-vector/bin/loongarch64-linux-gnu-gcc
COMPILE_PATH =  gcc

all: OsTools
##.PHONY all SEnd
OsTools : OsTools.o
	$(COMPILE_PATH) -static $(COMPILE_FILE_O) -o OsTools
	chmod 777 OsTools
OsTools.o :
	#$(COMPILE_PATH) -g -c $(COMPILE_FILE_C) 
	$(COMPILE_PATH) -c $(COMPILE_FILE_C) 

clean :
	rm *.o OsTools
