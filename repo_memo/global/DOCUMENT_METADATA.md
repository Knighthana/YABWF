---
Status: active
Authority: authoritative
Read-Tier: always
Purpose: 规定 repo_memo 文档的元信息字段、最小模板与新建文档的标注要求
version: knighthana@0.0.1
last_update: 2026-05-23 01:11:00
---

文档元信息说明
=============

面向Agent

---

# 目标
本说明用于管理 `repo_memo/` 及`user_memo/`子目录中的文档元信息，帮助 agent 在不限制开发期按需创建文档的前提下，明确文档用途、读取级别与后续归档方式。

# 基本原则
- **按需创建**：开发阶段允许为当前任务临时或长期创建新文档，不要求预先审批，不限制自由创建。
- **创建即标注用途**：新文档创建时，必须明确写出其用途与预期角色，避免后续归档时无法判断其地位。
- **先标用途，再看去留**：文档是否保留在主目录、移入 `archive/` 或 `future/`，取决于其用途与状态，而不是单纯取决于“是否完成”。
- **元信息服务治理，不阻塞开发**：元信息的目标是让后续整理更容易，而不是让创建文档变得繁琐。

# 适用范围
- `repo_memo/` 主目录中的规范文档
- `user_memo/archive/` 中的历史归档文档
- `user_memo/future/` 中的未来方案文档

# 最小元信息集
所有文档（包括存量文档）均须在开头显式写出以下信息。

- `Status`：`draft` / `active` / `stable` / `archived` / `future`
- `Authority`：`authoritative` / `reference-only`
- `Read-Tier`：`always` / `task-scoped` / `on-demand`
- `Purpose`：一句话说明本文档解决什么问题、服务什么任务
- `Supersedes`：可选；若替代了旧文档或旧方案，在此注明
- `version`: 用于标注兼容性的版本信息，当引用文档的时候所必需带上的版本号码
- `last_update`: 参考修改时间的信息，凡是修改都必须更新此时间和版本号码

# 最小模板
```md
---
Status: active
Authority: authoritative
Read-Tier: task-scoped
Purpose: 说明本文档用于约束哪个模块或阶段
Supersedes: 可选，填写被替代文档
version: 用于标注兼容性的版本信息，当引用文档的时候所必需带上的版本号码
last_update: 参考修改时间的信息，凡是修改都必须更新此时间和版本号码
---

文档标题
=======

副标题/附带说明/其他重要信息

---

# 章节一
```

# 目录归属判断
- 放在 `repo_memo/` 主目录：当前仍然定义系统行为或工程约束的权威文档
- 放在 `user_memo/archive/`：历史决策、迁移记录、阶段性实施文档；其有效约束已被主目录文档吸收
- 放在 `user_memo/future/`：已确认但当前轮次不执行的未来方案

# Agent 行为要求
- 创建新文档时，优先判断它是“当前规范”、“历史记录”还是“未来方案”
- 若无法立即判断长期归属，允许先在主目录创建，但必须明确写出 `Purpose`
- 若文档仅为阶段性记录或排障过程，应在阶段结束后评估是否转入 `user_memo/archive/`
- 不得因元信息制度而拒绝创建对当前任务有帮助的文档
