---
Status: future
Authority: reference-only
Read-Tier: on-demand
Purpose: CVE 验证与修复记录，记录已分析的安全漏洞结论、当前代码状态及修复措施
version: knighthana@0.1.0
last_update: 2026-05-23
---

# CVE 验证与修复分析

本文件记录 YABWF 代码库中已知 CVE 的分析结果、修复措施和验证结论。

---

## CVE-2009-4496 — 日志控制字符注入

**状态：✅ 已修复（含补充修复 2026-05-23）**

**描述：** Boa 0.94.13 及之前版本在日志记录时未过滤控制字符（如 ASCII 0x00-0x1F 和 0x7F），攻击者可通过伪造 HTTP 请求头向日志注入换行符等控制字符，导致日志伪造或终端注入攻击。

**修复措施：**
- 在 `util.c` 新增 `sanitize_log_string()` 工具函数
- 控制字符范围：0x00-0x1F（不含 `\t` 0x09）和 0x7F（DEL）替换为 `'?'`
- 应用于 `log_access()` 中的 `logline`、`header_referer`、`header_user_agent`
- 应用于 `log_error_doc()` 中的 `header_host`、`logline`、`pathname`
- 补充修复：`log_access()` 和 `log_error_doc()` 中 `vhost_root` 模式下的 `req->host` 原未覆盖，已于本次（2026-05-23）补全

**验证：** 编译通过，零 warning。

---

## CVE-2019-9976 — POST 临时文件持久化

**状态：✅ 已确认安全（无需修复）**

**描述：** Boa 在处理 POST 请求时创建临时文件存储请求体，但未及时删除，导致敏感数据残留磁盘。

**审查结果：**
- `create_temporary_file()`（`util.c:504-543`）在 `want_unlink=1` 时调用 `mkstemp` + 立即 `unlink`
- `request.c:858` 调用 `create_temporary_file(1, NULL, 0)`，即 `want_unlink=1`
- 文件创建后立即删除目录条目，进程存活期间通过 fd 访问
- 进程退出后内核自动回收 fd，无磁盘残留

**结论：** 当前行为已满足"不持久化 POST 数据到磁盘文件"的安全要求。

---

## CVE-2000-0920 — `%2E` 目录穿越

**状态：✅ 已确认安全（无需修复）**

**描述：** 通过 URL 编码的 `%2E%2E%2F`（即 `../`）进行目录遍历攻击。

**审查调用链：**
1. `process_header_end`（`request.c:794`）→
2. `unescape_uri`（`util.c:359`）将 `%2E%2E%2F` 解码为 `../`，`%2E%2E` 解码为 `..`
3. `clean_pathname`（`util.c:47`）去除 `//`、`/./`、`/../` 序列
4. `translate_uri`（`alias.c:235`）将清理后的 URI 映射到文件系统路径

**分析：**
- `unescape_uri` 对 `%2E` → `.`、`%2F` → `/` 的解码正确
- `clean_pathname` 正确处理 `/../`（被 `/` 跟随的 `..`）
- `/..` 结尾（无尾部 `/`）虽不被处理，但 `translate_uri` 将其拼接到 `server_root` 后，不会产生文件系统穿越
- `clean_pathname` 对 `/%2e%2e%2f` → `/../` 正确收敛为 `/`

**结论：** 当前防御链完整，目录穿越已被阻断。

---

## CVE-2005-0864 — 负值 Content-Length 导致 DoS

**状态：✅ 已确认安全（无需修复）**

**描述：** 通过负值 Content-Length 头导致 memcpy 崩溃（DoS）。

**审查结果：**
- `read.c:184-192` 使用 `boa_atoi()` 解析 Content-Length
- `boa_atoi()` 返回 `-1` 表示非法输入或越界
- 检查 `content_length < 0` 时返回 `send_r_bad_request`
- 负值或非数值 Content-Length 均被拒绝

**结论：** 已有完善校验，无需额外修复。

---

## CVE-2016-9564 — MIME 类型堆 use-after-free

**状态：✅ 已确认安全（无需修复）**

**描述：** Boa 0.94.13 在处理畸形配置或请求时，MIME 类型哈希表操作可能存在 use-after-free。

**审查结果：**
- `add_mime_type()`（`hash.c:385`）调用 `hash_insert()` 存入 mime_hashtable
- `get_mime_type()`（`hash.c:413`）从哈希表中安全查找
- 配置解析在启动时一次性完成（`config.c`），不会在请求处理中动态释放

**结论：** 当前代码未发现 use-after-free 路径。

---

## 厂商定制代码 CVE（与上游无关）

以下 CVE 影响的是厂商定制版本的 Boa，与 YABWF 上游代码无关，仅归档记录：

| CVE | 年份 | 说明 | 与 YABWF 关系 |
|-----|------|------|--------------|
| CVE-2007-4915 | 2007 | Boa 0.94.14rc21 中 `boa.c` 的 `get` 函数存在缓冲区溢出 | 厂商定制代码，上游无此路径 |
| CVE-2019-7384 | 2019 | Boa 0.94.13 中 `read_body()` 的竞争条件 | 厂商定制代码，上游实现不同 |
| CVE-2021-35395 | 2021 | Boa 0.94.13 中报告的多重漏洞 | 厂商定制代码，影响非上游版本 |
| CVE-2023-7208 | 2023 | Boa 0.94.13 的文件描述符泄露 | 厂商定制代码，上游代码路径已验证无此问题 |

**结论：** 以上 CVE 均为特定厂商发行版中引入的定制代码漏洞，YABWF 上游（截止到本记录时的基线）无对应代码路径，无需修复。
