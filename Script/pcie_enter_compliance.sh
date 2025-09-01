#!/usr/bin/env bash
# Usage:
#   sudo ./pcie_enter_compliance.sh 0002:00:00.0 0x4
#   A=BDF (domain:bus:dev.func), B=0..0xF (only low 4 bits are used)
set -euo pipefail

if [[ $EUID -ne 0 ]]; then
  echo "Please run as root." >&2
  exit 1
fi

if [[ $# -ne 2 ]]; then
  echo "Usage: $0 <BDF(A)> <B(0..0xF)>" >&2
  echo " e.g. $0 0002:00:00.0 0x4" >&2
  exit 1
fi

BDF="$1"
# 只取 B 的低4位
if [[ "$2" =~ ^0x ]]; then
  B=$(( $2 & 0xF ))
else
  B=$(( $2 & 0xF ))
fi

have() { command -v "$1" >/dev/null 2>&1; }

# ---- 选择 devmem 命令：优先 busybox devmem，退回系统 devmem
DEV_MEM_CMD=""
#if have busybox && busybox --list 2>/dev/null | grep -q "^devmem$"; then
  DEV_MEM_CMD="busybox devmem"
#elif have devmem; then
#  DEV_MEM_CMD="devmem"
#else
#  echo "Neither 'busybox devmem' nor 'devmem' is available." >&2
#  exit 1
#fi

for cmd in lspci setpci busybox; do
  if ! have "$cmd"; then
    echo "Missing command: $cmd" >&2
    exit 1
  fi
done

# ---- 获取 BAR0 物理地址
get_bar0_phys() {
  local bdf="$1"

  # 固定英文输出，避免本地化影响解析
  local ls; ls=$(LC_ALL=C lspci -s "$bdf" -v 2>/dev/null || true)
  [[ -z "$ls" ]] && return 1

  # ① 优先匹配 “Region 0: Memory at <hex>”
  local hex
  hex=$(
    awk '
      BEGIN{IGNORECASE=1}
      match($0,/Region[[:space:]]+0:[[:space:]]+Memory[[:space:]]+at[[:space:]]+([0-9a-fA-F]+)/,m){print m[1]; exit}
    ' <<< "$ls"
  )
  if [[ -n "${hex:-}" ]]; then
    printf "%u" $((16#$hex))
    return 0
  fi

  # ② 兼容 Bridge/Root Port：匹配第一条 “Memory at <hex>”，排除 “behind bridge”
  hex=$(
    awk '
      BEGIN{IGNORECASE=1}
      /Memory[[:space:]]+behind[[:space:]]+bridge/ {next}
      match($0,/^[[:space:]]*Memory[[:space:]]+at[[:space:]]+([0-9a-fA-F]+)/,m){print m[1]; exit}
    ' <<< "$ls"
  )
  if [[ -n "${hex:-}" ]]; then
    printf "%u" $((16#$hex))
    return 0
  fi

  # ③ 还不行则回退：直接读配置空间 BAR0（0x10/0x14）
  local bar0_lo_hex bar0_lo is_io bar_type bar0_hi_hex bar0_hi phys
  bar0_lo_hex=$(setpci -s "$bdf" 0x10.L) || return 1
  bar0_lo=$((16#$bar0_lo_hex))
  is_io=$((bar0_lo & 0x1))
  bar_type=$(((bar0_lo >> 1) & 0x3))   # 00=32bit mem, 10=64bit mem

  if (( is_io == 1 )); then
    echo "I/O BAR0 not supported" >&2
    return 2
  fi

  if (( bar_type == 0x2 )); then
    bar0_hi_hex=$(setpci -s "$bdf" 0x14.L)
    bar0_hi=$((16#$bar0_hi_hex))
    phys=$(((bar0_hi << 32) | (bar0_lo & ~0xF)))
  else
    phys=$((bar0_lo & ~0xF))
  fi
  printf "%u" "$phys"
  return 0
}

PHYS_DEC=$(get_bar0_phys "$BDF") || {
  echo "Failed to locate BAR0 base for $BDF." >&2
  exit 1
}
PHYS=$(printf "0x%X" "$PHYS_DEC")
printf "Device: %s\nBAR0 phys: %s\n" "$BDF" "$PHYS"

# ---- 便捷封装（32-bit 访问）
mmio_read32() {
  local addr=$1
  # shellcheck disable=SC2086
  $DEV_MEM_CMD "$addr" 32
}
mmio_write32() {
  local addr=$1 val=$2
  # shellcheck disable=SC2086
  $DEV_MEM_CMD "$addr" 32 "$val" >/dev/null
}

# ---- Step 1: BAR0 + 0x0 的 bit3 清 0
MMIO_ADDR=$(( PHYS_DEC + 0x0 ))
VAL32=$(mmio_read32 "$MMIO_ADDR")
NEW32=$(( VAL32 & ~0x8 ))
printf "MMIO[0x%X]: 0x%08X -> 0x%08X (clear bit3)\n" "$MMIO_ADDR" "$VAL32" "$NEW32"
mmio_write32 "$MMIO_ADDR" "$NEW32"

# ---- Step 2: 配置空间 0xA0[3:0] 清 0
A0_HEX=$(setpci -s "$BDF" 0xA0.B)
A0=$((16#$A0_HEX))
A0_CLR=$((A0 & 0xF0))
printf "CFG[0xA0]: 0x%02X -> 0x%02X (clear [3:0])\n" "$A0" "$A0_CLR"
setpci -s "$BDF" 0xA0.B=$(printf "%02x" "$A0_CLR")

# ---- Step 3: 把 B 写入 0xA0[3:0]
A0_SETLOW=$(((A0_CLR & 0xF0) | (B & 0x0F)))
printf "CFG[0xA0][3:0] <- 0x%X => 0x%02X\n" "$B" "$A0_SETLOW"
setpci -s "$BDF" 0xA0.B=$(printf "%02x" "$A0_SETLOW")

# ---- Step 4: 置 0xA0 的 bit4 = 1
A0_AFTER_HEX=$(setpci -s "$BDF" 0xA0.B)
A0_AFTER=$((16#$A0_AFTER_HEX))
A0_BIT4=$((A0_AFTER | 0x10))
printf "CFG[0xA0]: 0x%02X -> 0x%02X (set bit4)\n" "$A0_AFTER" "$A0_BIT4"
setpci -s "$BDF" 0xA0.B=$(printf "%02x" "$A0_BIT4")

# ---- Step 5: BAR0 + 0x0 的 bit3 置 1
VAL32_2=$(mmio_read32 "$MMIO_ADDR")
NEW32_2=$(( VAL32_2 | 0x8 ))
printf "MMIO[0x%X]: 0x%08X -> 0x%08X (set bit3)\n" "$MMIO_ADDR" "$VAL32_2" "$NEW32_2"
mmio_write32 "$MMIO_ADDR" "$NEW32_2"

echo "Done."

