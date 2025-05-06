#!/usr/bin/env bash
#
# 用法：./scan.sh 192.168.1 1 254
#      参数依次为：网段前三段、起始主机号、结束主机号
#

if [ $# -ne 3 ]; then
  echo "Usage: $0 <网段前三段，如 192.168.1> <start> <end>"
  exit 1
fi

network=$1
start=$2
end=$3

for i in $(seq "$start" "$end"); do
  ip="${network}.${i}"
  # -c1 发一个包；-W1 等待 1 秒超时
  ping -c1 -W1 "$ip" &>/dev/null
  if [ $? -eq 0 ]; then
    echo "[+] $ip is up"
  else
    echo "[-] $ip is down"
  fi
done

