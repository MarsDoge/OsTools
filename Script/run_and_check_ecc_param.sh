#!/bin/bash

#==============================================================================
#  run_and_check_ecc_param.sh
#  用法:
#     ./run_and_check_ecc_param.sh <SPEC脚本或可执行文件> [额外参数...]
#
#  功能:
#     1) 使用 stdbuf 禁用输出缓冲, 让SPEC脚本输出立刻可见
#     2) 把输出通过 tee 保存到 runsepc.log
#     3) 后台检测 ECC, 发现错误时备份日志并中止
#==============================================================================

if [ $# -lt 1 ]; then
  echo "用法: $0 <SPEC脚本名或可执行文件> [其余参数...]"
  exit 1
fi

# 第一个参数是要执行的脚本或程序
SPEC_SCRIPT=$1
shift  # 把后面参数(如果有)都留给SPEC_SCRIPT

# 如果用户只输入了脚本名, 而且该脚本确实在当前目录, 没有可执行路径, 就自动加上 "./"
if [[ "$SPEC_SCRIPT" != */* && -f "$SPEC_SCRIPT" ]]; then
    SPEC_SCRIPT="./$SPEC_SCRIPT"
fi

# 确认一下可执行权限
if [ ! -x "$SPEC_SCRIPT" ]; then
    echo "错误: $SPEC_SCRIPT 不存在或不可执行"
    exit 1
fi

#==============================================================================
# 1) 使用 stdbuf 禁用缓冲, 让 SPEC_SCRIPT 实时输出
#    同时将其输出重定向到 runsepc.log, 并在后台执行
#==============================================================================
echo "[INFO] Starting $SPEC_SCRIPT with args: $@ (No buffering)"
stdbuf -o0 -e0 "$SPEC_SCRIPT" "$@" | tee runsepc.log &
RUNSEPC_PID=$!

#==============================================================================
# 2) ECC 检测循环 (每隔 5 秒执行一次)
#==============================================================================
echo "[INFO] ECC 检测线程开始 ..."

while true; do
    # 如果 SPEC_SCRIPT 不在了, 说明它执行完毕(或已退出)
    if ! kill -0 $RUNSEPC_PID 2>/dev/null; then
        echo "[INFO] $SPEC_SCRIPT 已结束, 停止 ECC 检测."
        break
    fi

    # 调用 ECC 检测脚本(例如 check_ecc.sh)
    ./check_ecc.sh
    ECC_RET=$?

    # 如果检测到 ECC 错误
    if [ $ECC_RET -ne 0 ]; then
        echo "[ERROR] ECC Error detected! 停止测试并保存日志..."
        cp runsepc.log runsepc_ecc_$(date +%Y%m%d_%H%M%S).log
        kill -9 $RUNSEPC_PID
        break
    fi

    # 每隔 5 秒再查一次
    sleep 30
done

echo "[INFO] run_and_check_ecc_param.sh 脚本结束."

