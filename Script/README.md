# 📌 git_master_to_md.py 使用说明

---

## 🧩 工具简介

`git_master_to_md.py` 用于：

- 读取指定 Git 仓库某个分支的提交记录
- 自动解析 commit 描述与 trailers（如 Change-Id / Reviewed-by 等）
- 生成结构化 Markdown 报告
- 可选生成 Gerrit Change 链接

适用于：

- Patch 汇总
- Gerrit 提交统计
- CI 自动生成 Release Notes
- 内部版本归档

---

## 🚀 基本使用方法

```bash
python3.8 git_master_to_md.py --repo <git_repo_path> --branch <branch_name>


🧾 参数说明
| 参数                 | 说明                              |
| ------------------ | ------------------------------- |
| `--repo`           | Git 仓库路径（默认当前目录）                |
| `--branch`         | 要读取的分支（默认 master）               |
| `--out`            | 输出 Markdown 文件名                 |
| `--reverse`        | 旧 → 新 顺序输出                      |
| `--limit`          | 限制提交数量                          |
| `--gerrit-base`    | Gerrit 服务器地址（自动生成 Change-Id 链接） |
| `--max-desc-lines` | 最大保留描述行数                        |
| `--max-desc-chars` | 最大保留描述字符数                       |

