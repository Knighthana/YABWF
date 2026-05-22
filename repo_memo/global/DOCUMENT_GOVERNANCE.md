---
Status: stable
Authority: authoritative
Read-Tier: always
Purpose: 定义 repo_memo 的治理规则、读取层级、例外流程与归档边界
version: knighthana@0.0.2
last_update: 2026-05-23 00:55:00
---

文档治理规则
===========

面向Agent

---

# 需求来源优先级
实现与测试的需求来源优先级固定为：
1. Plan 指令
2. `repo_memo/` 中的文档与 `repo_spec/` 中的 Schema
3. 代码注释
以上三项冲突时按优先级裁决。

# 文档 vs 代码
- 文档是指导性规范，代码跟随文档变更
- 代码可以随文档变更重构，但不反过来推导逻辑
- `user_memo/`不得直接作为实现依据

# 分层读取机制
- `Tier 0 / always-read`：建立最小正确上下文的核心文档；agent 每次开始实现前必须读取
- `Tier 1 / task-scoped`：按模块或任务读取的权威设计文档；仅在任务涉及对应子系统时读取
- `user_memo/archive/`：历史设计、阶段决策、同步记录；默认不作为当前实现依据，仅在追溯时按需读取
- `user_memo/future/`：已确认但当前轮次不执行的未来方案；默认不作为当前实现依据

## Tier 0（每次必读）
- `README.md`
- `repo_memo/global/DOCUMENT_GOVERNANCE.md`
- `repo_memo/global/DOCUMENT_METADATA.md`
- `repo_memo/global/PATTERNS_ENGINEERING.md`
- `repo_spec/README.md`

## Tier 1（按任务读取）

# 冲突解决
- `repo_memo/` 与 `user_memo/` 冲突 → 以 `repo_memo/` 为准
- `repo_memo/` 与 `repo_spec/` 冲突 → Plan 裁决（二者同级权威）
- `repo_memo/` 内部文档间冲突 → Plan 裁决
- 未满足需求来源优先级时，任务标记为 blocked

# Plan 授权例外
- 仅当 Plan 明确写出例外范围时，允许临时引用 `user_memo/`
- 例外任务必须写明：授权来源、有效期、影响字段、回收动作
- 例外结束后在 `user_memo/archive/MEMORY_SYNC_INDEX.md` 追加回收记录

# 执行门禁
- 实现任务必须引用 `repo_memo/` 中的文档路径；缺失路径视为无效
- 涉及字段命名变更时，先更新 `TERMS_TERMINOLOGY.md` 与相关设计文档，再改代码
- 新建文档遵循 `DOCUMENT_METADATA.md`；至少显式写出 `Purpose`
- 归档判定以“是否仍为当前权威规范”为准，不以“是否已完成实现”单独判定

# 角色与现场信息
- `repo_logs/` 没有明确指令情况下**禁止阅读**，仅用于历史追溯，不参与默认读取与契约裁决
- `work_memo/` 仅用于当前工作现场状态记录
- `repo_spec/` 中的 Schema 与 `repo_memo/` 中的文档具有同等权威性
- Plan 需要排障时可读取 `work_memo/decisions.md`，但若与 `repo_memo/` 冲突，仍以 `repo_memo/` 为准

# AI Agent 使用约束
- `user_memo/` 目录对 AI agent **禁止**用于推导实现逻辑、Schema 约束或字段定义
- Agent 任务必须以 `repo_memo/` 和 `repo_spec/` 为唯一权威来源
- `user_memo/` 仅由人类管理者在明确指令下被动更新，agent 读取该目录内容时须忽略其规范含义

# 归档与未来方案边界
- `user_memo/archive/` 只接收历史设计、阶段决策、迁移记录；其有效规范必须已被主目录文档吸收
- `user_memo/future/` 只接收未来方案，不得混入历史归档
- 若某设计文档仍定义当前系统行为，即使对应功能已完成，也必须保留在主目录并按任务读取
