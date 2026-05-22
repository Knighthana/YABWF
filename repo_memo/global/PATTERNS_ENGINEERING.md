---
Status: stable
Authority: authoritative
Read-Tier: always
Purpose: 记录跨任务复用的工程模式、校验顺序与开发环境约束
version: knighthana@0.0.1
last_update: 2026-05-23 01:11:00
---

工程模式
=======

记录了每次工程实践之前必须遵守的步骤

---

# 开发环境约束
- **Python / pip**：所有 Python 操作（安装依赖、运行测试、启动服务）必须在项目根目录的 `.venv/` 虚拟环境中执行。禁止向系统 Python 或其他虚拟环境安装包。
- **Node / npm**：所有 Node 操作必须通过本机安装的 `fnm` 管理的 Node 版本执行。版本固定见项目根目录 `.node-version`，进入目录后 `fnm` 自动切换。禁止绕过 `fnm` 直接调用系统 node/npm。

# 分层开发
开发前必须明确自己要创建和修改的部分位于哪一层，层之间只能通过约定好的schema进行通信，严禁为了某一层的功能变更，在没有明确对Schema提出修改请求的前提下，擅自修改其他层的代码；

跨层 Schema 的权威定义位于 `repo_spec/` 目录。涉及跨层接口变更时，必须同步更新对应的 Schema 文件并递增版本号。

L0: 文档与 SPEC 层（`repo_memo/` + `repo_spec/` + `repo_test/`）
L1: 平台适配层（compat.h, config.h.in, ip.c, signals.c, configure.in）
L2: 核心业务层（request.c, response.c, get.c, alias.c, cgi.c, read.c, pipe.c, buffer.c, range.c）
L3: 基础设施层（hash.c, queue.c, mmap_cache.c, log.c, escape.c, util.c, sublog.c）
L4: 入口与控制层（boa.c, config.c, select.c/poll.c, timestamp.c）
