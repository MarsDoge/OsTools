# //SPDX-License-Identifier: GPL-2.0
#!/usr/bin/env bash
set -euo pipefail

# 版本：2025-10-14 robust-v2
# 目标：支持多种输入（多个文件/目录/通配符），一次性解析到**一个**CSV。
# 关键修复：构造整行到变量再一次性写入，避免任何字段级重定向/换行边界问题。
# 用法：
#   ./parse_lcl_xxd_all.sh fwA.fd fwB.fd
#   ./parse_lcl_xxd_all.sh /path/to/dir_with_fd
#   ./parse_lcl_xxd_all.sh /path/*.fd
# 可选：OUT=out.csv 指定输出文件名（默认带时间戳）

command -v xxd >/dev/null || { echo "需要安装 xxd" >&2; exit 1; }
export LC_ALL=C

if (( $# == 0 )); then
  echo "用法: $0 <.fd 文件或目录或通配符...>" >&2
  exit 1
fi

# ---------------- 常量（按你的固件布局可在此调整） ----------------
BASE=0x88000           # 起始偏移
NODE_STRIDE=0x200      # 每个节点占用
LCL_STRIDE=0x80        # 每条 LCL 占用（0x80 字节 = 16 lanes * 8 bytes）
LANES=16               # 每个 LCL 的 lanes 数
LANE_BYTES=8           # 每 lane 字节数
LCL_BYTES=$(( LANES * LANE_BYTES ))   # 0x80

TOT_NODE_NUM=${TOT_NODE_NUM:-4}
START_NODE=${START_NODE:-0}
END_NODE=${END_NODE:-$((TOT_NODE_NUM-1))}

# ---------------- 收集输入（文件/目录/通配符），并 realpath 去重 ----------------
inputs=()
for arg in "$@"; do
  if [[ -d "$arg" ]]; then
    while IFS= read -r -d '' f; do inputs+=("$f"); done < <(find "$arg" -type f -name '*.fd' -print0 | sort -z)
  else
    inputs+=("$arg")
  fi
done

# realpath 去重
declare -A seen=()
declare -a fds=()
for f in "${inputs[@]}"; do
  [[ -f "$f" ]] || continue
  rp=$(readlink -f -- "$f" 2>/dev/null || realpath -- "$f" 2>/dev/null || printf '%s' "$f")
  [[ -n "${seen[$rp]:-}" ]] && continue
  seen["$rp"]=1
  fds+=("$rp")
done

(( ${#fds[@]} > 0 )) || { echo "未找到任何 .fd" >&2; exit 1; }

# ---------------- 输出 CSV 准备 ----------------
OUT=${OUT:-lcl_lane_parse_$(date +%Y%m%d_%H%M%S)_ALL.csv}
: >"$OUT"
# 打开输出 FD
exec {OUTFD}>>"$OUT"
# 表头（只写一次）
printf '%s
' 'File,Node,Lcl,Status,Lane,offset,preset,boost,vga,ee1,ee2,dfe1,dfe2,dfe3' >&$OUTFD

# ---------------- 工具函数 ----------------
read_lcl_hex() { # $1=file $2=addr_hex
  xxd -p -s 0x${2} -l $LCL_BYTES "$1" | tr -d '
'
}
hex_at() { # $1=hex $2=byte_index
  local pos=$(( $2 * 2 )); echo "${1:$pos:2}"
}

# ---------------- 主解析 ----------------
for file in "${fds[@]}"; do
  for ((node=$START_NODE; node<=$END_NODE; node++)); do
    for ((lcl=0; lcl<($NODE_STRIDE/$LCL_STRIDE); lcl++)); do
      lcl_addr=$(( BASE + NODE_STRIDE*node + LCL_STRIDE*lcl ))
      addr_hex=$(printf "%08x" "$lcl_addr")
      hex=$(read_lcl_hex "$file" "$addr_hex")

      # 文件不足：写一行标记
      if (( ${#hex} < LCL_BYTES*2 )); then
        printf '%s
' "$file,$node,$lcl,TRUNCATED,-,-,-,-,-,-,-,-,-" >&$OUTFD
        continue
      fi

      # 判定整条 LCL 不存在（前两字节 55 aa）
      b0=$(hex_at "$hex" 0); b1=$(hex_at "$hex" 1)
      if [[ "$b0" == "55" && "$b1" == "aa" ]]; then
        printf '%s
' "$file,$node,$lcl,ABSENT,-,-,-,-,-,-,-,-,-" >&$OUTFD
        continue
      fi

      # 逐 lane 解析（全部在变量里拼成一整行，再一次性输出）
      for ((lane=0; lane<LANES; lane++)); do
        base_idx=$(( lane * LANE_BYTES ))
        t0=$(hex_at "$hex" $((base_idx+0)))  # offset
        t1=$(hex_at "$hex" $((base_idx+1)))  # preset
        t2=$(hex_at "$hex" $((base_idx+2)))  # boost(high4) + vga(low4)
        t3=$(hex_at "$hex" $((base_idx+3)))  # ee1 low4
        t4=$(hex_at "$hex" $((base_idx+4)))  # ee2 low4
        t5=$(hex_at "$hex" $((base_idx+5)))  # dfe1 low4
        t6=$(hex_at "$hex" $((base_idx+6)))  # dfe2 low4
        t7=$(hex_at "$hex" $((base_idx+7)))  # dfe3 low4

        if [[ -z "$t7" ]]; then
          printf '%s
' "$file,$node,$lcl,TRUNCATED,$lane,-,-,-,-,-,-,-,-" >&$OUTFD
          continue
        fi

        # 直接用十进制位运算得到 0x* 文本，避免子 shell/printf 嵌套
        d_b2=$(( 16#${t2} ))
        boost=$(( (d_b2 >> 4) & 0xF ))
        vga=$(( d_b2 & 0xF ))
        ee1=$(( 16#${t3} & 0xF ))
        ee2=$(( 16#${t4} & 0xF ))
        dfe1=$(( 16#${t5} & 0xF ))
        dfe2=$(( 16#${t6} & 0xF ))
        dfe3=$(( 16#${t7} & 0xF ))

        line="$file,$node,$lcl,PRESENT,$lane,0x${t0},0x${t1},0x$(printf '%x' "$boost"),0x$(printf '%x' "$vga"),0x$(printf '%x' "$ee1"),0x$(printf '%x' "$ee2"),0x$(printf '%x' "$dfe1"),0x$(printf '%x' "$dfe2"),0x$(printf '%x' "$dfe3")"
        printf '%s
' "$line" >&$OUTFD
      done
    done
  done
done

# 关闭 FD
exec {OUTFD}>&-
# 清 CR 行尾，保持干净
sed -i 's/
$//' "$OUT" 2>/dev/null || true

# 最终提示（stderr，不污染 CSV）
echo "已生成: $OUT" >&2

