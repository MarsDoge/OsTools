#!/bin/bash
# 文件名: run_and_check_ecc.sh

# 1) 启动 runsepc.sh 并记录输出到 runsepc.log
echo "[INFO] Starting runsepc.sh ..."
./runsepc.sh | tee runsepc.log &
RUNSEPC_PID=$!

# 2) 循环检测 ECC
echo "[INFO] ECC 检测线程开始 ..."

while true; do
    # 如果 runsepc.sh 不存在了，说明测试结束
    if ! kill -0 ${RUNSEPC_PID} 2>/dev/null; then
        echo "[INFO] runsepc.sh 测试脚本结束。停止 ECC 检测。"
        break
    fi

    # 调用 check_ecc.sh
    ./check_ecc.sh
    ECC_RET=$?

    # 如果返回码不为 0, 表示 ECC 出错
    if [ $ECC_RET -ne 0 ]; then
        echo "[ERROR] ECC Error detected. 停止 SPEC 测试并保存日志 ..."
        # 保存当前日志到带时间戳的文件
        cp runsepc.log runsepc_ecc_$(date +%Y%m%d_%H%M%S).log
        # 杀掉 SPEC 测试进程
        kill -9 ${RUNSEPC_PID}
        break
    fi

    # 每隔 5 秒检测一次
    sleep 5
done

echo "[INFO] run_and_check_ecc.sh 结束。"

