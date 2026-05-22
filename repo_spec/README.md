---
Status: active
Authority: authoritative
Read-Tier: always
Purpose: SPEC 文档索引与命名约定，作为 Tier 0 必读文档指导 agent 查阅各模块 SPEC
version: knighthana@0.1.0
last_update: 2026-05-23
---

# repo_spec 文档规范

SPEC 索引与模块设计文档约定

---

## 文件命名约定

- 每个源文件对应一个 SPEC 文档，路径为 `repo_spec/modules/{模块名}.md`
- 模块名取源文件名去掉 `.c` 后缀，例如 `src/alias.c` → `repo_spec/modules/alias.md`
- 条件编译模块（如 `select.c` / `poll.c`）共享同一 SPEC 文档 `repo_spec/modules/event_loop.md`
- 头文件（`.h`）不单独创建 SPEC，其内容归属于对应的实现模块 SPEC

## SPEC 文件最小模板

每个 SPEC 文档必须包含以下章节：

```md
---
Status: draft | active | stable
Authority: authoritative | reference-only
Read-Tier: task-scoped
Purpose: 说明本文档约束哪个模块及约束范围
version: <version>
last_update: <date>
---

# 模块名

模块职责
--------
一句话说明该模块的功能边界和核心职责。

接口签名
--------
列出模块对外暴露的函数签名（含参数和返回值类型）。

返回值语义
--------
说明每个函数返回值的含义，参照 PATTERNS_ENGINEERING.md 中的返回值约定。

错误处理策略
--------
说明模块内错误处理方式（返回码、日志、DIE 等），以及调用方的处理责任。

安全边界
--------
说明模块涉及的安全敏感操作（输入验证、缓冲区限制、权限检查等）及防御策略。

内存占用
--------
说明模块使用的静态/动态内存、对象池或缓冲区大小，以及峰值内存估算。
```

## 当前已知模块清单

以下为 `src/Makefile.in` 中 `SOURCES` 列表对应的模块（含条件编译模块）：

| 模块 | 源文件 | SPEC 路径 | 层级 |
|------|--------|-----------|------|
| alias | `src/alias.c` | `repo_spec/modules/alias.md` | L2 |
| boa | `src/boa.c` | `repo_spec/modules/boa.md` | L4 |
| buffer | `src/buffer.c` | `repo_spec/modules/buffer.md` | L3 |
| cgi | `src/cgi.c` | `repo_spec/modules/cgi.md` | L2 |
| cgi_header | `src/cgi_header.c` | `repo_spec/modules/cgi_header.md` | L2 |
| config | `src/config.c` | `repo_spec/modules/config.md` | L4 |
| escape | `src/escape.c` | `repo_spec/modules/escape.md` | L3 |
| get | `src/get.c` | `repo_spec/modules/get.md` | L2 |
| hash | `src/hash.c` | `repo_spec/modules/hash.md` | L3 |
| ip | `src/ip.c` | `repo_spec/modules/ip.md` | L1 |
| log | `src/log.c` | `repo_spec/modules/log.md` | L3 |
| mmap_cache | `src/mmap_cache.c` | `repo_spec/modules/mmap_cache.md` | L3 |
| pipe | `src/pipe.c` | `repo_spec/modules/pipe.md` | L2 |
| queue | `src/queue.c` | `repo_spec/modules/queue.md` | L3 |
| range | `src/range.c` | `repo_spec/modules/range.md` | L2 |
| read | `src/read.c` | `repo_spec/modules/read.md` | L2 |
| request | `src/request.c` | `repo_spec/modules/request.md` | L2 |
| response | `src/response.c` | `repo_spec/modules/response.md` | L2 |
| signals | `src/signals.c` | `repo_spec/modules/signals.md` | L1 |
| util | `src/util.c` | `repo_spec/modules/util.md` | L3 |
| sublog | `src/sublog.c` | `repo_spec/modules/sublog.md` | L3 |
| select | `src/select.c` | `repo_spec/modules/event_loop.md` | L4 |
| poll | `src/poll.c` | `repo_spec/modules/event_loop.md` | L4 |
| access | `src/access.c` | `repo_spec/modules/access.md` | L4 |
| index_dir | `src/index_dir.c` | `repo_spec/modules/index_dir.md` | 工具 |
| cgi_strip_prefix | `src/cgi_header.c`（子功能） | `repo_spec/modules/cgi_strip_prefix.md` | L2 |

层级定义参见 `repo_memo/global/PATTERNS_ENGINEERING.md` 层级划分表。

SPEC 文档尚为空白（未创建）时，agent 可在首次涉及对应模块时按上述模板创建。
