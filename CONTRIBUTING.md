# 贡献指南

## Commit 规范

使用以下格式：

```text
<type>(<scope>): <简短说明>

<为什么改，以及修改后的行为>

验证：
- <实际执行的检查及结果>

关联：#<issue，可选>
```

- 类型：`feat`（功能）、`fix`（修复）、`refactor`（重构）、`build`（构建）、`ci`（持续集成）、`docs`（文档）、`test`（测试）。
- 范围使用受影响模块，如 `spi`、`loongarch`、`smbios`。
- 标题建议不超过 72 字符。说明可使用中文或英文，正文按修改复杂度填写，不保留空字段。
- 一个 commit 解决一个明确问题。功能修改、构建修复和无关清理分别提交。
- 验证记录必须准确；语法检查、完整编译、运行测试和硬件实测应分别说明。
- 关联 Issue 为可选项，不为填写模板而创建 Issue。

示例：

```text
feat(spi): make write-protection fallback configurable

Replace ENABLE_WRITE_PROTECTION guards with the
--write-protection command-line option, disabled by default.

Validation:
- C syntax check passed.
- Hardware testing not performed.
```

在仓库根目录执行以下命令，为当前克隆启用模板：

```bash
git config --local commit.template commit-template.txt
```

模板不会随克隆自动启用。使用不带 `-m` 的 `git commit` 打开编辑器填写；模板中的注释行在正常交互式提交时会被移除。

## PR 规范

PR 标题同样使用 `<type>(<scope>): <简短说明>`。描述按 PR 模板填写：

1. 问题与改动：说明问题和最终行为，必要时附使用示例。
2. 验证结果：列出实际执行的检查及结果，说明未验证的范围。
3. 关联 Issue：可选，无关联时删除。

硬件相关修改注明测试平台、工具链、是否真机实测及覆盖范围。仅通过编译不能视为硬件验证通过。构建或 CI 失败应说明原因，不能标记为验证通过。

提交前检查差异，排除无关改动和临时产物。初期通过模板和代码审查落实规范，不强制校验历史 commit；后续如增加自动校验，优先校验 PR 标题。
