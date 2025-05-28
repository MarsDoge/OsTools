#!/bin/bash
# 依赖：lspci, grep, awk, busybox (需带 devmem 命令)
# 建议以 root 权限运行

echo ">>> Dumping PCIe bridge MEM BARs and reading values with busybox devmem"
echo

# 仅遍历 class code = 06-04 (PCI-to-PCI Bridge) 的设备
for dev in $(lspci -D -d ::0604 | awk '{print $1}'); do
    echo "=== Device: $dev ==="

    # 获取设备信息
    desc=$(lspci -D -s "$dev")
    echo "Description: $desc"

    # 获取 BAR 的 MEM 地址段
    lspci -v -s "$dev" | while IFS= read -r line; do
        if [[ $line == *"Memory at"* ]]; then
            bar=$(echo "$line" | grep -oP 'Memory at \K[0-9a-fA-F]+')
            size=$(echo "$line" | grep -oP '\[\Ksize=[^\]]+')

            echo "MEM BAR: 0x$bar ($size)"

            # 读取感兴趣的寄存器（可根据需求调整）
            for offset in 0xb8 0xbc 0xc4; do
                addr=$(printf "0x%x" $((0x$bar + offset)))
                value=$(busybox devmem "$addr" 32)
                echo "  [${addr}] = ${value}"
            done
        fi
    done

    echo
done

