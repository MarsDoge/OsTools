#!/bin/bash
# 文件名: check_ecc.sh

ecc_output=$(./OsTools ecc -c -n 4)
echo "$ecc_output"

# 判断是否存在 "0x618" 非 0 的记录
cnt=$(echo "$ecc_output" \
      | grep "0x610 :" \
      | grep -v "0x0000000000000000$" \
      | wc -l)

if [ "$cnt" -gt 0 ]; then
    echo "[ERROR] ECC Error Detected!"
    exit 1
else
    echo "[OK] No ECC error."
    exit 0
fi

