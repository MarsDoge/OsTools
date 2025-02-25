#!/bin/bash

# 执行 ECC 检测命令并将输出保存到变量
ecc_output=$(./OsTools ecc -c -n 4)

# 如果想同时在终端显示，也可以先显示再保存
# echo "$ecc_output" | tee ecc_output.log
# 这里直接放到变量中做后续解析
echo "$ecc_output"

echo "---------- ECC 检测结果分析 ----------"

# 从输出中筛选所有包含 "0x610 :" 的行
# 因为这行里包含 0x610 和 0x618（第二 8 字节）两个寄存器信息
echo "$ecc_output" | grep "0x610 :" | while read -r line
do
    # line 类似于：
    # 0x610 : 0x0000000000000000  0x0000000000000000
    #
    # 其中第3个字段是 0x0000000000000000 （对应 0x610）
    # 第4个字段是 0x0000000000000000 （对应 0x618）
    second_val=$(echo "$line" | awk '{print $4}')

    # 判断 second_val 是否为全 0
    if [ "$second_val" != "0x0000000000000000" ]; then
        echo "[!] ECC Error Detected!"
        echo "    出错行：$line"
        # 这里也可以做更具体的处理，如记录到日志、报警等
    else
        echo "[OK] No ECC error in line: $line"
    fi
done

echo "---------- 分析结束 ----------"

