#!/bin/bash

PERCENT=10
DURATION=60
CORES=$(nproc)

if [ ! -z "$1" ]; then
    PERCENT=$1
fi

if [ ! -z "$2" ]; then
    DURATION=$2
fi

if [ ! -z "$3" ]; then
    CORES=$3
fi

BUSY_TIME=$(echo "$PERCENT * 0.01" | bc -l)
IDLE_TIME=$(echo "1 - $BUSY_TIME" | bc -l)

# 实际计算函数（小规模，方便频繁调用）
real_work() {
    while true; do
        echo "$RANDOM * $RANDOM" | bc > /dev/null
    done
}

echo "🚀 正在运行：每核 $PERCENT% 真实负载，持续 $DURATION 秒，使用 $CORES 核"

for i in $(seq 1 $CORES); do
    (
        end=$((SECONDS + DURATION))
        while [ $SECONDS -lt $end ]; do
            # 用 timeout 限制计算时间
            timeout "$BUSY_TIME"s bash -c 'while true; do echo "$RANDOM * $RANDOM" | bc > /dev/null; done'
            sleep $IDLE_TIME
        done
    ) &
done

wait
echo "✅ 负载完成"

