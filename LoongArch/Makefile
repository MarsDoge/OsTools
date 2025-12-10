# SPDX-License-Identifier: GPL-2.0
#COMPILE_PATH =  /opt/LoongArch_Toolchains/loongarch64-linux-gnu-2021-06-19-vector/bin/loongarch64-linux-gnu-gcc
#COMPILE_PATH =  /opt/LoongArch_Toolchains/loongarch64-linux-gnu-2021-06-19-vector/bin/loongarch64-linux-gnu-gcc
COMPILE_PATH =  gcc

BUILD_DIR := build
ENC_C_FILES := $(wildcard *.c.enc)
DECODED_C_FILES := $(patsubst %.c.enc,$(BUILD_DIR)/%.c,$(ENC_C_FILES))
SRCS := $(sort $(wildcard *.c) $(DECODED_C_FILES))
OBJS := $(SRCS:.c=.o)

GIT_COMMIT_ID := $(shell git rev-parse HEAD)

ARCH := $(shell uname -m)
ifeq ($(ARCH),loongarch64)
CFLAGS = -DPLATFORM_LA64 -std=gnu11
else ifeq ($(ARCH),mips64)
CFLAGS = -DPLATFORM_MIPS64 -std=gnu11
else
    $(warning Unsupported architecture: $(ARCH))
endif
CFLAGS += -DGIT_COMMIT_ID="\"$(GIT_COMMIT_ID)\""

LIBC_STATIC = $(shell echo "int main(){}" | $(COMPILE_PATH) -x c - --static -o /dev/null > /dev/null 2>&1 || echo "1")
ifeq ($(LIBC_STATIC),)
	LD_STATIC := -static
else
	LD_STATIC :=
endif

all: OsTools
##.PHONY all SEnd
OsTools: $(OBJS)
	$(COMPILE_PATH) $(LD_STATIC) $^ -o $@
	chmod 777 $@

%.o: %.c
	$(COMPILE_PATH) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.c: %.c.enc | $(BUILD_DIR)
	../script/codec.sh decode $< $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean :
	rm -f $(OBJS) OsTools
	rm -rf $(BUILD_DIR)
