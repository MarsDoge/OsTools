#!/usr/bin/env bash
# UEFI 板卡快速进入脚本（从当前目录读取 .ini 配置）
# 作者: MarsDoge
# 功能:
#   - 在当前目录读取 uefi.ini
#   - 每行格式:  路径 :: 注释
#   - 提供交互菜单选择后自动 cd 进入

INI_FILE="./uefidir.ini"

# ====== 检查配置文件 ======
if [ ! -f "$INI_FILE" ]; then
  echo "⚠️ 未找到配置文件: $INI_FILE"
  echo "请在当前目录创建一个，例如:"
  echo ""
  echo "$HOME/work/uefi-loongarch :: 3A6000 EVB 主板"
  echo "$HOME/github/LsRefCodePkg_3C6000 :: 3C6000 服务器平台"
  echo ""
  exit 1
fi

# ====== 读取配置 ======
mapfile -t ITEMS < <(grep -v '^[[:space:]]*#' "$INI_FILE" | grep -v '^[[:space:]]*$')

if [ ${#ITEMS[@]} -eq 0 ]; then
  echo "⚠️ 配置文件为空: $INI_FILE"
  exit 1
fi

# ====== 交互菜单 ======
if command -v fzf >/dev/null 2>&1; then
  SELECTED=$(printf '%s\n' "${ITEMS[@]}" | \
    awk -F '::' '{printf "%-60s | %s\n", $1, $2}' | \
    fzf --prompt="选择板卡目录 > " --height=40% --with-nth=1,2)
  DIR=$(echo "$SELECTED" | awk -F '|' '{print $1}' | sed 's/[[:space:]]*$//')
else
  echo "未检测到 fzf，使用简单菜单："
  select entry in "${ITEMS[@]}"; do
    SELECTED="$entry"
    break
  done
  DIR=$(echo "$SELECTED" | awk -F '::' '{print $1}' | sed 's/[[:space:]]*$//')
fi

# ====== 进入目录 ======
if [ -n "$DIR" ] && [ -d "$DIR" ]; then
  cd "$DIR" || exit
  echo "✅ 已进入目录: $DIR"
else
  echo "⚠️ 目录不存在: $DIR"
fi

