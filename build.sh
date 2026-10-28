#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
set -e

make -C LoongArch clean
make -C LoongArch
cp LoongArch/OsTools ./
