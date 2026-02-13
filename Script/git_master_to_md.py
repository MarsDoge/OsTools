# Author: Dongyan Qian

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import re
import subprocess
from pathlib import Path
from typing import List, Tuple, Dict, Optional

TRAILER_RE = re.compile(r"^\s*([A-Za-z][A-Za-z0-9-]*):\s*(.+)\s*$")

def run(cmd: List[str], cwd: Path) -> str:
    return subprocess.check_output(cmd, cwd=str(cwd), text=True, stderr=subprocess.STDOUT)

def split_desc_and_trailers(body: str) -> Tuple[str, List[Tuple[str, str]]]:
    lines = body.splitlines()

    # strip trailing blank lines
    i = len(lines) - 1
    while i >= 0 and lines[i].strip() == "":
        i -= 1

    trailers_rev: List[Tuple[str, str]] = []
    while i >= 0:
        m = TRAILER_RE.match(lines[i])
        if not m:
            break
        key = m.group(1).strip().lower()
        val = m.group(2).strip()
        trailers_rev.append((key, val))
        i -= 1

    trailers = list(reversed(trailers_rev))

    desc_lines = lines[: i + 1]
    while desc_lines and desc_lines[-1].strip() == "":
        desc_lines.pop()

    desc = "\n".join(desc_lines).strip()
    return desc, trailers

def normalize_description(desc: str, max_lines: int = 10, max_chars: int = 1000) -> str:
    if not desc:
        return ""
    # first paragraph only
    parts = re.split(r"\n\s*\n", desc, maxsplit=1)
    first = parts[0].strip()
    lines = first.splitlines()
    if len(lines) > max_lines:
        lines = lines[:max_lines] + ["…"]
    s = "\n".join(lines)
    if len(s) > max_chars:
        s = s[: max_chars - 1] + "…"
    return s

def group_trailers(trailers: List[Tuple[str, str]]) -> Dict[str, List[str]]:
    d: Dict[str, List[str]] = {}
    for k, v in trailers:
        d.setdefault(k, []).append(v)
    return d

def maybe_change_url(gerrit_base: Optional[str], change_id: str) -> Optional[str]:
    if not gerrit_base or not change_id:
        return None
    base = gerrit_base.rstrip("/")
    return f"{base}/q/{change_id}"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", default=".", help="path to git repo")
    ap.add_argument("--branch", default="master")
    ap.add_argument("--out", default=None, help="output markdown file")
    ap.add_argument("--reverse", action="store_true", help="oldest -> newest (git --reverse)")
    ap.add_argument("--gerrit-base", default=None, help="optional: http://xxxxx:xxxx")

    ap.add_argument("--max-desc-lines", type=int, default=10)
    ap.add_argument("--max-desc-chars", type=int, default=1000)
    ap.add_argument("--limit", type=int, default=0, help="limit commits (0 = no limit)")

    args = ap.parse_args()

    repo = Path(args.repo).resolve()
    if not (repo / ".git").exists():
        raise SystemExit(f"Not a git repo: {repo}")

    # optional fetch
    try:
        run(["git", "fetch", "--all", "--prune"], cwd=repo)
    except Exception:
        pass

    RS = "\x1e"
    FS = "\x1f"
    # fields: full hash, short hash, subject, author name/email, body
    pretty = f"%H{FS}%h{FS}%s{FS}%an{FS}%ae{FS}%b{RS}"

    cmd = ["git", "log", args.branch, f"--pretty=format:{pretty}"]
    if args.reverse:
        cmd.append("--reverse")
    if args.limit and args.limit > 0:
        cmd.extend(["-n", str(args.limit)])

    raw = run(cmd, cwd=repo)
    records = [r for r in raw.split(RS) if r.strip()]

    md: List[str] = []
    title = f"## ✅ 已合并提交（{args.branch}，按 git log 顺序）— xxxxx\n"
    md.append(title)

    if not records:
        md.append("_未查询到提交。_\n")
    else:
        for rec in records:
            parts = rec.split(FS)
            if len(parts) < 6:
                continue

            full_hash = parts[0].strip()
            short = parts[1].strip()
            subject = parts[2].strip()
            author_name = parts[3].strip()
            author_email = parts[4].strip()
            body = parts[5]

            desc, trailers = split_desc_and_trailers(body)
            desc_short = normalize_description(desc, args.max_desc_lines, args.max_desc_chars)
            tmap = group_trailers(trailers)

            md.append(f"---")
            md.append(f"- `{short}` {subject}")
            md.append(f"  - 作者：{author_name} <{author_email}>")

            if desc_short:
                md.append("  - 描述：")
                for line in desc_short.splitlines():
                    md.append(f"    - {line}")

            # trailers we care about
            for key, label in [
                ("change-id", "Change-Id"),
                ("reviewed-by", "Reviewed-by"),
                ("tested-by", "Tested-by"),
                ("acked-by", "Acked-by"),
                ("signed-off-by", "Signed-off-by"),
            ]:
                vals = tmap.get(key, [])
                if vals:
                    md.append(f"  - {label}：")
                    for v in vals:
                        if key == "change-id":
                            url = maybe_change_url(args.gerrit_base, v)
                            md.append(f"    - [`{v}`]({url})" if url else f"    - `{v}`")
                        else:
                            md.append(f"    - {v}")

            md.append("")

    content = "\n".join(md)

    if not args.out:
        args.out = "merged_commits.md"
    Path(args.out).write_text(content, encoding="utf-8")
    print(args.out)

if __name__ == "__main__":
    main()

