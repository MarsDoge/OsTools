#!/usr/bin/env python3
import os, sys, struct

MAGIC = 0x51554D2D5041434B  # "QM-PACK"

def main():
    if len(sys.argv) != 5:
        print(f"Usage: {sys.argv[0]} <stub> <qemu-ARCH-static> <target-prog> <out>")
        sys.exit(2)

    stub, qemu, prog, out = sys.argv[1:]

    for p in (stub, qemu, prog):
        if not os.path.isfile(p):
            print(f"error: not a file: {p}")
            sys.exit(1)

    len_q = os.path.getsize(qemu)
    len_p = os.path.getsize(prog)

    flags = 0  # bit0=1 表示压缩，这里先置 0（原始）

    # 复制 stub → out
    with open(stub, 'rb') as fs, open(out, 'wb') as fo:
        fo.write(fs.read())

    # 依次追加 qemu/prog
    with open(out, 'ab') as fo, open(qemu, 'rb') as fq, open(prog, 'rb') as fp:
        # 直接流式拷贝，避免内存暴涨
        for f in (fq, fp):
            while True:
                b = f.read(1024 * 1024)
                if not b: break
                fo.write(b)

        # 末尾写 trailer：<QQQQ>
        fo.write(struct.pack('<QQQQ', MAGIC, len_q, len_p, flags))

    # 置可执行
    mode = os.stat(out).st_mode
    os.chmod(out, mode | 0o111)

    # 粗略校验
    final = os.path.getsize(out)
    expect = os.path.getsize(stub) + len_q + len_p + 32
    if final != expect:
        print("warning: size mismatch (unexpected), got", final, "expect", expect)

    print("OK ->", out)

if __name__ == '__main__':
    main()

