#!/bin/bash

# =============================
# 配置参数
# =============================

# 总共测试次数
S3_REPEAT=3
REBOOT_REPEAT=3

# S3 睡眠时间（秒）
SLEEP_DURATION=30

# 挂起模式（S3 对应 mem）
SUSPEND_MODE=mem

# 日志文件路径
LOG_FILE="/var/log/s3_and_reboot_test.log"

# 临时标记文件用于记录 reboot 次数
REBOOT_FLAG_FILE="/tmp/reboot_test_counter"

# =============================
# 函数定义
# =============================

log() {
    echo "[$(date)] $1" | tee -a "$LOG_FILE"
}

s3_test() {
    log "==== 开始 S3 睡眠唤醒测试（共 $S3_REPEAT 次，每次睡眠 $SLEEP_DURATION 秒） ===="

    for ((i=1; i<=S3_REPEAT; i++)); do
        log "S3 测试 [$i/$S3_REPEAT]：准备进入睡眠模式..."
        rtcwake -m "$SUSPEND_MODE" -s "$SLEEP_DURATION"
        log "S3 测试 [$i/$S3_REPEAT]：系统已唤醒。"
        log "-------------------------------------------"
        sleep 5
    done

    log "==== S3 睡眠唤醒测试完成 ===="
}

reboot_test() {
    # 检查是否是第一次进入重启流程
    if [[ ! -f "$REBOOT_FLAG_FILE" ]]; then
        echo 1 > "$REBOOT_FLAG_FILE"
        log "==== 启动重启测试（共 $REBOOT_REPEAT 次） ===="
    else
        COUNT=$(cat "$REBOOT_FLAG_FILE")
        COUNT=$((COUNT + 1))
        echo "$COUNT" > "$REBOOT_FLAG_FILE"
    fi

    CURRENT_COUNT=$(cat "$REBOOT_FLAG_FILE")

    if [[ "$CURRENT_COUNT" -le "$REBOOT_REPEAT" ]]; then
        log "重启测试 [$CURRENT_COUNT/$REBOOT_REPEAT]：系统即将重启..."
        sleep 2
        reboot
    else
        log "==== 重启测试完成，共重启 $REBOOT_REPEAT 次 ===="
        rm -f "$REBOOT_FLAG_FILE"
    fi
}

# =============================
# 主控制逻辑
# =============================

# 确保以 root 身份运行
if [[ $EUID -ne 0 ]]; then
    echo "请以 root 用户身份运行此脚本。"
    exit 1
fi

# 参数说明
if [[ "$1" == "--s3" ]]; then
    s3_test
elif [[ "$1" == "--reboot" ]]; then
    reboot_test
elif [[ "$1" == "--all" ]]; then
    s3_test
    reboot_test
else
    echo "用法: $0 [--s3 | --reboot | --all]"
    echo "  --s3       只运行 S3 睡眠唤醒测试"
    echo "  --reboot   只运行 reboot 重启测试"
    echo "  --all      先运行 S3，再进行多次重启测试"
    exit 1
fi

