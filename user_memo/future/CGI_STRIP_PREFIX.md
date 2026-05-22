---
Status: draft
Authority: reference-only
Read-Tier: on-demand
Purpose: CGI 输出前缀剥离功能设计方案——自动丢弃 CGI 程序在有效 HTTP 头之前输出的调试/日志内容
version: knighthana@0.1.0
last_update: 2026-05-23
---

CGI 输出前缀剥离功能设计
=======================

针对 CGI 程序向 stdout 输出调试信息（printf、warning 等）导致 HTTP 响应异常的问题，
在服务器侧提供可选的自动剥离功能。

---

# 问题场景

CGI 程序在输出 HTTP 头之前向 stdout 写入非 HTTP 内容：
```
DEBUG: connecting to database...
DEBUG: query returned 3 rows
Content-Type: text/html

<html>...
```

标准 CGI 要求第一行即为 HTTP 头。上述输出会导致客户端收到畸形响应。

# 功能概述

启用后，服务器在处理 CGI 输出时：
1. 从 CGI 输出流中定位第一个有效 HTTP 头行
2. 丢弃该行之前的所有内容
3. 丢弃的内容写入 CGI 日志（`CgiLog` 配置项），标注 `[CGI STRIP]` 前缀
4. 从头行开始正常解析 HTTP 响应

# 配置项

```
# boa.conf
CGIStripPrefix On    # 启用 CGI 输出前缀剥离（默认 Off）
```

# 有效头行的识别规则

满足以下任一条件的行视为"第一个有效 HTTP 头行"：

| 优先级 | 匹配规则 | 示例 |
|--------|---------|------|
| 1 | `Status: ` 开头（大小写不敏感） | `Status: 200 OK` |
| 2 | `Location: ` 开头（大小写不敏感） | `Location: http://...` |
| 3 | `Content-Type: ` 开头（大小写不敏感） | `Content-Type: text/html` |
| 4 | 匹配 `^[A-Za-z][A-Za-z0-9-]*: ` 格式的任意行 | `X-Custom: value` `Set-Cookie: ...` |

不满足以上任一条件的行视为"前缀内容"，被丢弃。

# 特殊边界情况

| 场景 | 行为 |
|------|------|
| CGI 输出中**一直未出现**有效头行，直至 `\n\n` | 视为 CGI 错误，返回 502 Bad Gateway；完整 CGI 输出记录到错误日志 |
| CGI 输出**仅含前缀内容**（无任何有效头），CGI 进程退出 | 返回 502 Bad Gateway；前缀内容记录到 CGI 日志 |
| CGI 输出第一行即为空行（`\n`） | 空行属于前缀内容，被丢弃 |
| CGI 输出中包含看起来像 HTTP 头的调试信息（如日志中的 `Content-Length: 0`） | 以**第一个**匹配行为准，之前的全部丢弃。此行为可能导致误识别——调试信息中若恰好包含匹配格式的文本，会被当作头开始。这是功能固有的权衡 |
| NPH CGI（`nph-` 前缀的脚本） | **不受此功能影响**，NPH 输出直接透传给客户端，不做任何解析 |

# 实现位置

修改 `src/cgi_header.c` 的 `process_cgi_header()` 函数。

在现有 `strstr(buf, "\n\n")` 查找头结束标记之前，插入前缀扫描逻辑：

```
1. cgi_strip_enabled 检查（全局配置标志）
2. 若启用，从头扫描 buf，逐行匹配有效头规则
3. 定位第一个有效头行位置 start_pos
4. 若 start_pos > buf，将 buf 到 start_pos 的内容写入 CGI 日志
5. 将 start_pos 设置为新的 header_line，继续原有流程
```

# 内存影响

在 `request` 结构体中新增 `cgi_prefix_stripped` 标志位和 `cgi_prefix_len` 计数（各 4 字节，<10 字节增量）。

# 安全考量

- 丢弃的内容**不包含**在 HTTP 响应中，不存在响应注入风险
- 丢弃的内容写入日志前经过 `sanitize_log_string()` 过滤
- 误识别风险（调试信息被当作头）：可通过日志排查，且属于 fail-open（仍有头输出）而非 fail-closed（无响应）

# 后续扩展

- 增加 `CGIStripMaxPrefix` 配置项，限制最大丢弃字节数（默认 4096），防止 CGI 输出大量垃圾数据撑满日志
- 正则规则支持（当前逐行前缀匹配已覆盖绝大多数场景，正则在此场景下边际收益低）
