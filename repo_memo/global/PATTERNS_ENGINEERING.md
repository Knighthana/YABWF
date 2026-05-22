---
Status: stable
Authority: authoritative
Read-Tier: always
Purpose: 记录 YABWF 项目的工程模式、设计约束与开发规范，适用于所有 agent 任务执行
version: knighthana@1.0.0
last_update: 2026-05-23
---

YABWF 工程模式
=============

记录每次工程实践必须遵守的步骤与约束。

---

# 核心设计原则

## 低资源占用优先

YABWF 面向低性能嵌入式设备（ARM/Linux, uclibc），运行时内存占用是最高优先级设计约束。

- 所有连接共享同一进程空间，不采用 per-request fork/thread 模型
- 固定大小的栈分配缓冲区优先于堆分配（`BUFFER_SIZE=4KB` per connection）
- 请求结构体 `request` 通过对象池复用，避免频繁 `malloc`/`free`
- 文件服务优先使用 `mmap` 零拷贝，辅以 `sendfile` 系统调用



### 内存 Profiling 要求

任何涉及缓冲区大小、结构体成员增减、新内存分配的变更，必须在测试阶段输出内存占用报告，报告至少包含：

1. 单连接稳态内存增量（空闲连接占用的 struct + buffer 空间）
2. 峰值并发下的总 RSS 增量（含 mmap 缓存和 CGI 子进程）
3. 与变更前基准值的对比

测试工具可使用 `/proc/self/status` 采样或 `valgrind --tool=massif`。报告存放于 `test/` 目录，与代码变更同一 PR 提交。

## 单进程事件驱动

项目采用单进程 `select`/`poll` 事件循环，所有 I/O 操作通过非阻塞 socket + 每连接状态机驱动。

- 主循环位于 `select.c` 或 `poll.c`，编译时二选一
- 信号处理遵循异步安全模式：信号 handler 仅设置 flag，主循环中处理
- CGI 执行通过 `fork`+`exec` 创建子进程，父进程通过管道收集输出



## 内存管理策略：对象池

对象池是 YABWF 的核心内存管理模式。其设计目标是在内存占用可预测且恒定的前提下，消除运行时堆碎片与分配延迟。

### 池的设计原则

- 池的大小在编译期或启动期确定（通过 `#define` 或配置文件），启动后不可增长
- 池满时明确拒绝新增（返回 `NULL` 或回退到降级路径），不触发隐式 `malloc`
- 池中对象结构体大小固定，成员中的动态内存（如 `char *` 字段）仍需单独管理，但结构体本身不产生碎片

### 当前已使用的池

| 池名称 | 数据结构 | 容量 | 位置 |
|--------|---------|------|------|
| request 池 | `request_free` 链表 | 无硬上限（由 `max_connections` 间接限制） | `queue.c` `request.c` |
| Range 池 | `range_pool` 链表 | 按需增长 | `range.c` |
| mmap 缓存 | `mmap_list[256]` 固定数组 + 开放寻址 | `MMAP_LIST_SIZE=256` | `mmap_cache.c` |

### 池的优势与适用条件

池在嵌入式场景下提供三个关键保证：

1. **内存占用可预测** — 启动后内存曲线平坦，不会因长时间运行发生堆膨胀
2. **分配延迟恒定** — O(1) 链表操作，不触发 brk/mmap 系统调用
3. **零碎片** — 固定大小对象，无外部碎片累积

适用条件：对象大小固定，且峰值并发数可以通过配置或编译常量上限化。

### 新增池的判断标准

当以下条件同时满足时，应引入对象池：

1. 某类结构体在请求生命周期内被频繁分配和释放
2. 该结构体的大小固定（或变化范围极小）
3. 峰值并发数可以估算出合理上限

新增池时，须在对应模块的 SPEC 文档中记录：池容量、容量计算依据、池满时的降级行为。

---

# 编译与工具链

## 编译系统

使用 GNU Autoconf（`configure.in` → `configure`）+ Make。入口脚本 `construct.sh` 封装交叉编译流程。

产物资源控制通过编译选项实现：`-Os`（体积优化）、`-s`（strip 符号表）。构建工具本身的资源占用不影响产物，仅在开发环境使用。

`config.sub` 和 `config.guess` 来源于 GNU config 项目，须定期同步上游以保持对新架构（aarch64、riscv64 等）的支持。

## 语法服务器

项目根目录 `generator_clangd.sh` 负责生成 clangd 所需的编译数据库。

生成方式：通过 `bear -- make` 捕获编译命令，产出 `compile_commands.json`。clangd 以此文件获取 include path、宏定义与编译选项，提供准确的诊断、跳转与补全。

在未完成 configure 的环境（即无 `config.h` 时），脚本须生成包含 `config.h.in` 占位的 fallback 配置，确保 clangd 不因缺失 `config.h` 而报错。



---

# 层级划分

YABWF 是 C 单体项目。层间通信通过头文件中声明的函数签名和结构体定义实现。

| 层级 | 名称 | 包含模块 | 职责 |
|------|------|---------|------|
| L0 | 文档层 | `repo_memo/` `repo_spec/` `repo_test/` | 设计规范、Schema、测试策略 |
| L1 | 平台适配层 | `compat.h` `config.h.in` `defines.h` `ip.c` `signals.c` `configure.in` `config.sub` `config.guess` | OS/平台差异抽象、编译期配置 |
| L2 | 核心服务层 | `request.c` `response.c` `get.c` `alias.c` `cgi.c` `cgi_header.c` `read.c` `pipe.c` `range.c` | HTTP 协议处理、资源映射、CGI 调度 |
| L3 | 基础设施层 | `hash.c` `queue.c` `mmap_cache.c` `buffer.c` `log.c` `escape.c` `util.c` `sublog.c` `timestamp.c` | 通用数据结构、日志、编码工具 |
| L4 | 入口与控制层 | `boa.c` `config.c` `select.c` `poll.c` `access.c` | 启动、配置加载、事件循环 |

层间约束：上层可调用下层，下层不得依赖上层。跨层接口的权威定义位于各模块对应的 `repo_spec/` SPEC 文档和源码头文件中。



---

# 开发前置检查

agent 在开始任何实现任务前，必须完成以下检查：

1. 读取 Tier 0 文档（见 `DOCUMENT_GOVERNANCE.md`）
2. 确认 `config.h` 已生成（即项目已通过 `./configure`），否则 clangd 诊断不可靠
3. 确认任务涉及的模块在 `repo_spec/` 中有对应 SPEC 文档；若没有，需先创建 SPEC 再实现
4. 涉及内存分配变更时，确认已规划 profiling 方案



---

# 测试分层

| 层级 | 目录 | 工具 | 覆盖范围 |
|------|------|------|---------|
| 单元测试 | `test/unit/` | 独立编译的 C 测试程序（利用 `STANDALONE_TEST` 模式） | 纯函数：`unescape_uri` `clean_pathname` `range_parse` `boa_atoi` 等 |
| 集成测试 | `test/integration/` | Shell 脚本 + `curl` + 项目自带 CGI 示例 | HTTP 端到端：GET/POST/CGI/目录索引 |
| 安全测试 | `test/security/` | PoC 脚本 | CVE 复现与回归验证 |

测试策略文档存放于 `repo_test/`，测试代码存放于 `test/`。



---

# 编码约定

## 返回值约定

YABWF 中所有函数遵循统一的返回值语义。按函数类别分述如下：

### 请求处理函数（状态机驱动）

适用于 `read_header` `read_body` `write_body` `process_get` `read_from_pipe` `write_from_pipe` `io_shuffle` 等。

| 返回值 | 含义 | 调用方动作 |
|--------|------|-----------|
| `1` | 函数完成一轮非阻塞处理，仍有后续工作 | 保持在 ready 队列，下次循环继续 |
| `0` | 请求处理完成或发生不可恢复错误 | 调用 `free_request()`，关闭连接 |
| `-1` | I/O 操作会阻塞（EAGAIN/EWOULDBLOCK） | 移至 blocked 队列，等待 fd 就绪 |

### I/O 缓冲写入函数

适用于 `req_write` `req_write_escape_http` `req_write_escape_html` `req_flush`。

| 返回值 | 含义 |
|--------|------|
| `>= 0` | 成功，值为当前 `buffer_end`（已缓冲字节数） |
| `-1` | 缓冲区不足或 flush 时发生 EAGAIN（阻塞） |
| `-2` | flush 时发生不可恢复错误（连接断开），请求状态置为 DEAD |

### 初始化与一次性操作函数

适用于 `init_get` `init_cgi` `process_header_end` `process_logline` `process_option_line`。

| 返回值 | 含义 |
|--------|------|
| `1` | 初始化成功，请求继续处理 |
| `0` | 初始化失败，错误已发送给客户端，连接关闭 |

### CGI 头解析函数

适用于 `process_cgi_header`。

| 返回值 | 含义 |
|--------|------|
| `1` | CGI 头解析成功，继续处理 |
| `0` | 错误（非 CGI/1.1 兼容、HEAD 请求完成）或 Location 重定向已完成 |

### 布尔判定函数

适用于 `modified_since` `unescape_uri` `check_host` `range_parse` `ranges_fixup`。

| 返回值 | 含义 |
|--------|------|
| `1` | 真（条件满足、解析成功） |
| `0` | 假（条件不满足、解析失败） |

### 指针返回函数

适用于 `new_request` `find_mmap` `find_alias` `hash_find` `get_mime_type` `get_home_dir` `strdup` `malloc`。

| 返回值 | 含义 |
|--------|------|
| 非 NULL 指针 | 成功 |
| `NULL` | 失败（内存不足、未找到、或参数无效） |

### 文件描述符返回函数

适用于 `create_temporary_file` `open_gen_fd`。

| 返回值 | 含义 |
|--------|------|
| `> 0` | 有效的文件描述符 |
| `0` | 失败 |

### 数值转换函数

适用于 `boa_atoi` `month2int`。

| 返回值 | 含义 |
|--------|------|
| `>= 0` | 有效转换结果 |
| `-1` | 输入不合法或越界 |

### 新增函数的返回值选择

新增函数时，按以下优先级选择返回风格：

1. 若函数属于请求状态机 → 使用三值 `1 / 0 / -1` 约定
2. 若函数是纯判定 → 使用 `1 / 0`
3. 若函数分配资源 → 使用指针 / fd 返回，`NULL` / `0` 表示失败
4. 若函数需要传递多种错误原因 → 使用负数错误码，并在模块 SPEC 文档中定义错误码表

## 错误处理

- 使用 `DIE()` 宏处理致命错误（输出日志 + `exit`）
- 使用 `WARN()` 处理非致命错误
- `malloc`/`strdup` 返回值必须检查，失败时通过 `boa_perror()` 报告并向客户端发送 500
- `errno` 在调用 `log_error_doc()` 等会修改 `errno` 的函数前必须保存

## 全局变量

- 全局变量集中声明于 `globals.h`
- 定义（分配存储）于各自的 `.c` 文件中
- 命名使用小写下划线风格，模块内聚的变量加模块前缀

## 信号处理

- 信号处理函数仅设置全局 flag（`sighup_flag` `sigchld_flag` `sigterm_flag`）
- 实际逻辑在事件循环中通过 `xxx_run()` 函数执行
- `SIGBUS` 例外：在 mmap 区域写入时通过 `setjmp`/`longjmp` 即时捕获

## 缓冲区边界

- 所有字符串拼接操作前必须验证目标缓冲区剩余空间
- `MAX_HEADER_LENGTH`（当前 1024）、`BUFFER_SIZE`（4096）、`CLIENT_STREAM_SIZE`（8192）的调整必须在 SPEC 文档中明确理由，并通过内存 profiling
