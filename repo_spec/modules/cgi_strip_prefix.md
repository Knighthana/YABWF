---
Status: draft
Authority: reference-only
Read-Tier: task-scoped
Purpose: CGI 输出前缀剥离功能设计方案——自动丢弃 CGI 程序在有效 HTTP 头之前输出的调试/日志内容
version: knighthana@0.2.0
last_update: 2026-05-23
---

CGI 输出前缀剥离功能设计
=======================

针对 CGI 程序向 stdout 输出调试信息（printf、warning 等）导致 HTTP 响应异常的问题，
在服务器侧提供可选的自动剥离功能。

---

# 问题场景

**标准场景**（有换行）：
```
DEBUG: connecting to database...
DEBUG: query returned 3 rows
Content-Type: text/html

<html>...
```

**恶化场景**（无换行——小公司烂代码常见）：
```
DEBUG: connecting to DB...Status: 200 OK\nContent-Type: text/html\n\n<html>...
```
垃圾文本直接拼接在头行前面，无 `\n` 分隔，行检测彻底失效。

标准 CGI 要求第一行即为 HTTP 头。上述输出会导致客户端收到畸形响应。

# 功能概述

启用后，服务器在处理 CGI 输出时：
1. 定位 HTTP 头-体分隔符 `\n\n`（锚点）
2. 从锚点向前扫描，使用 **token 白名单 + 行格式** 双重匹配确定头块起点
3. 丢弃起点之前的所有内容
4. 丢弃的内容写入 CGI 日志（`CgiLog` 配置项），标注 `[CGI STRIP]`
5. 从起点开始正常解析 HTTP 响应

# 配置项

```
# boa.conf
CGIStripPrefix On    # 启用 CGI 输出前缀剥离（默认 Off）
```

# 匹配算法

## 第一阶段：定位锚点

在 CGI 输出 buffer 中查找 `\n\n`（或 `\r\n\r\n`）。若未找到，返回 502 Bad Gateway。

## 第二阶段：确定头块起点

从锚点向前逐行扫描：

```
1. 设 pos = 锚点位置（\n\n 的第一个 \n）
2. 循环：
   a. 找到上一行的起始位置 line_start（上一个 \n 之后，或 buffer 起始）
   b. 若 line_start 指向的行以白名单头名开头 → 继续向前（pos = line_start - 1）
   c. 若不匹配白名单 → 在该行内搜索最后一个白名单 token，若找到则头块起点 = token 位置
   d. 若行内也无 token → 头块起点 = 下一行起始（当前行及之前为垃圾）
   e. 若到达 buffer 起始且全部匹配 → 无垃圾，不剥离
```

## token 白名单

匹配条件：大小写不敏感。白名单内头名后必须跟随 `:` 和一个空格或 `\r`/`\n`。

| 优先级 | token | 说明 |
|--------|-------|------|
| 特殊 | `Status:` | CGI 特殊头 |
| 特殊 | `Location:` | CGI 特殊头 |
| 高频 | `Content-Type:` | 必定存在 |
| 高频 | `Set-Cookie:` | 常见 |
| 高频 | `Content-Length:` | 常见 |
| 通用 | `Cache-Control:` `Connection:` `WWW-Authenticate:` `Expires:` `Pragma:` `Content-Encoding:` `Content-Language:` `Content-Disposition:` `Last-Modified:` `ETag:` `Vary:` `Allow:` | 标准 HTTP 头 |
| 扩展 | `^[A-Za-z][A-Za-z0-9-]*: ` | 兜底（匹配 X-* 等自定义头） |

## 示例

**场景 A：标准垃圾行**
```
DEBUG: connecting...\nContent-Type: text/html\n\n<html>
```
- 锚点 = `\n\n`
- 向前第 1 行：`Content-Type: text/html` → 白名单匹配 ✓
- 向前第 2 行：`DEBUG: connecting...` → 白名单不匹配，行内无 token
- → 头上限 = `Content-Type:` 的 `C` 位置
- → 剥离 `DEBUG: connecting...\n`

**场景 B：无换行垃圾**
```
DEBUG: connecting...Status: 200 OK\nContent-Type: text/html\n\n<html>
```
- 锚点 = `\n\n`
- 向前第 1 行：`Content-Type: text/html` → 白名单匹配 ✓
- 向前第 2 行：`DEBUG: connecting...Status: 200 OK` → 行首不匹配，但行内搜索到 `Status:`
- → 头上限 = `Status:` 的 `S` 位置
- → 剥离 `DEBUG: connecting...`

# 特殊边界情况

| 场景 | 行为 |
|------|------|
| CGI 输出中无 `\n\n` | 返回 502 Bad Gateway |
| 所有行都匹配白名单（纯标准 CGI） | 不剥离，零开销 |
| 垃圾文本中包含恰好等于白名单 token 的字符串 | 会误触发——这是功能固有权衡。此时 CGI 输出本身已不规范，误触发后的行为不会比不开此功能更差 |
| NPH CGI（`nph-` 前缀的脚本） | 不受此功能影响，直接透传 |
| 空行出现在头块中 | 空行不匹配白名单，会终止向前扫描，属于正确行为 |

# 实现位置

修改 `src/cgi_header.c` 的 `process_cgi_header()` 函数。

新增辅助函数 `strip_cgi_prefix(char *buf, size_t buf_len)`：
- 返回头块起点指针，若无需剥离则返回 `buf`
- 剥离内容写入日志
- 全局开关 `cgi_strip_prefix`（配置项）

# 内存影响

- 新增全局标志 `cgi_strip_prefix`（1 byte）
- `strip_cgi_prefix()` 使用栈上局部变量，无额外堆分配

# 安全考量

- 剥离内容不进入 HTTP 响应，无注入风险
- 剥离内容写入日志前经 `sanitize_log_string()` 过滤
- token 白名单基于 HTTP 规范，不接受任意字符串

# 后续扩展

- `CGIStripMaxPrefix`：限制最大剥离字节数（默认 4096），防止 CGI 输出巨量垃圾撑满日志
