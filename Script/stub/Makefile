# SPDX-License-Identifier: GPL-2.0
COMPILE_FILE_C = stub.c $(DECODED_C_FILES)
COMPILE_FILE_O = stub.o $(DECODED_O_FILES)

#COMPILE_PATH =  /opt/LoongArch_Toolchains/loongarch64-linux-gnu-2021-06-19-vector/bin/loongarch64-linux-gnu-gcc
#COMPILE_PATH =  /opt/LoongArch_Toolchains/loongarch64-linux-gnu-2021-06-19-vector/bin/loongarch64-linux-gnu-gcc
COMPILE_PATH =  gcc

BUILD_DIR := build
ENC_C_FILES := $(wildcard *.c.enc)
DECODED_C_FILES := $(patsubst %.c.enc,$(BUILD_DIR)/%.c,$(ENC_C_FILES))
DECODED_O_FILES := $(patsubst %.c,%.o,$(DECODED_C_FILES))

GIT_COMMIT_ID := $(shell git rev-parse HEAD)

ARCH := $(shell uname -m)
ifeq ($(ARCH),loongarch64)
CFLAGS = -DPLATFORM_LA64
else ifeq ($(ARCH),mips64)
CFLAGS = -DPLATFORM_MIPS64
else
    $(warning Unsupported architecture: $(ARCH))
endif

all: OsTools_stub
##.PHONY all SEnd
OsTools_stub : OsTools_stub.o
	$(COMPILE_PATH) -O2 -s -static $(COMPILE_FILE_O) -o OsTools_stub
        ##./pack.py ./OsTools_stub ./qemu-x86_64 ./hello.x86_64 ./hello.one
OsTools_stub.o : $(COMPILE_FILE_C)
	#$(COMPILE_PATH) -g -c $(COMPILE_FILE_C)
	$(COMPILE_PATH) $(CFLAGS) -c $(COMPILE_FILE_C) -DGIT_COMMIT_ID="\"$(GIT_COMMIT_ID)\""

$(BUILD_DIR)/%.c: %.c.enc | $(BUILD_DIR)
	../script/codec.sh decode $< $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean :
	rm -f *.o OsTools_stub
	rm -rf $(BUILD_DIR)
