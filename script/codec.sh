#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "Usage: $0 encode <input> <output> | decode <input> <output>" >&2
    exit 1
}

if [ "$#" -ne 3 ]; then
    usage
fi

if ! command -v base64 >/dev/null 2>&1; then
    echo "Error: required command 'base64' not found" >&2
    exit 1
fi

mode="$1"
input="$2"
output="$3"

case "$mode" in
    encode)
        base64 "$input" > "$output"
        ;;
    decode)
        base64 -d "$input" > "$output"
        ;;
    *)
        usage
        ;;
esac
