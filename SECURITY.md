# Security Policy

## 报告漏洞

若发现 YABWF 中的安全漏洞，请通过 GitHub Issue 提交，标题注明 `[SECURITY]`。
YABWF 面向嵌入式/内网场景，不适用于公开互联网部署。

## 已处理 CVE 列表

以下为已知 Boa 相关 CVE 在 YABWF 中的处理状态。按 CVE 编号排序。

| CVE | 状态 | 说明 |
|-----|------|------|
| CVE-2000-0920 | ✅ 基线已安全 | `clean_pathname` 阻断了 `%2E` 目录穿越 |
| CVE-2005-0864 | ✅ 基线已安全 | `boa_atoi` 拒绝负值 Content-Length |
| CVE-2007-4915 | ⬜ 不适用 | Intersil isl3893 厂商扩展代码，YABWF 无此路径 |
| CVE-2009-4496 | ✅ 已修复（0.0.2） | 日志控制字符过滤（`sanitize_log_string`） |
| CVE-2016-9564 | ✅ 基线已安全 | 未发现 use-after-free 路径 |
| CVE-2017-9833 | ⬜ 不适用 | **DISPUTED** — `/cgi-bin/wapopen` 为系统集成商代码，Boa 上游不含此程序 |
| CVE-2018-21028 | ✅ 已修复（0.0.2） | scandir.c malloc 未检查 + index_dir.c strdup 泄漏 + strutil.c strstr 优化 |
| CVE-2019-7384 | ⬜ 不适用 | Raisecom GPON 厂商定制 CGI handler，YABWF 无此路径 |
| CVE-2019-9976 | ✅ 基线已安全 | POST 临时文件使用 `mkstemp` + `unlink`，不持久化 |
| CVE-2021-33558 | ⬜ 不适用 | **DISPUTED** — `backup.html`/`log.html` 等文件非 Boa 组成部分，站点配置问题 |
| CVE-2021-35395 | ⬜ 不适用 | Realtek SDK 厂商定制 CGI handler，YABWF 无此路径 |
| CVE-2022-45956 | ✅ 已修复（0.0.2） | HEAD 方法绕过 Allow/Deny — 在 `process_header_end()` 中添加统一 `access_allow()` 检查 |
| CVE-2023-7208 | ⬜ 不适用 | Totolink 厂商定制 CGI handler，YABWF 无此路径 |
| CVE-2024-40088 | ⬜ 不适用 | Vilo 5 Mesh WiFi 厂商设备固件，非上游 Boa |
| CVE-2024-47916 | ✅ 基线已安全 | 路径穿越 CWE-22 — `unescape_uri`+`clean_pathname` 双编码/空字节均已阻断 |
| CVE-2025-7909 | ⬜ 不适用 | D-Link DIR-513 厂商定制 CGI handler（**EOL 设备，Unsupported When Assigned**） |
| CVE-2025-7910 | ⬜ 不适用 | D-Link DIR-513 厂商定制 CGI handler（**EOL 设备，Unsupported When Assigned**） |
| CVE-2025-8757 | ⬜ 不适用 | TRENDnet TV-IP110WN 嵌入式设备，需本地访问 |
| CVE-2026-1687 | ⬜ 不适用 | Tenda HG10 `/boaform/formSamba` 厂商定制 CGI handler |

**状态说明**：
- ✅ 已修复：该 CVE 在 YABWF 中已通过代码修改修复
- ✅ 基线已安全：该 CVE 描述的漏洞在 YABWF 基线代码中已被现有机制阻断
- ⬜ 不适用：该 CVE 影响的是特定厂商定制版 Boa，YABWF 上游代码中无对应路径。**保留此条目以防止重复评估**

详细分析见 `user_memo/future/CVE_ANALYSIS.md`。

## 安全设计要点

- **日志安全**：`sanitize_log_string()` 过滤控制字符（CVE-2009-4496 修复），缓冲区大小可通过 `LOG_SANITIZE_BUF_SIZE` 配置
- **CGI Location 头**：仅接受 `http://` 和 `https://` 开头的完整 URL，拒绝绝对路径
- **POST 临时文件**：使用 `mkstemp` + `unlink`，进程存活期间通过 fd 访问，退出后无磁盘残留
- **CGI Strip Prefix**（可选编译）：自动丢弃 CGI 程序在 HTTP 头之前输出的调试/日志内容
