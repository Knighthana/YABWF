---
Status: stable
Authority: authoritative
Read-Tier: task-scoped
Purpose: 记录 YABWF 所有编译期配置选项，避免开发者从 configure.in/defines.h 中翻找
version: knighthana@0.1.0
last_update: 2026-05-23
---

# YABWF 编译选项

## configure 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `--enable-log-color` | yes | ANSI 颜色码（仅在 isatty 时生效） |
| `--enable-access-control` | no | Allow/Deny 访问控制 |
| `--disable-gunzip` | (gunzip检测) | 禁用 gunzip 压缩支持 |
| `--disable-sendfile` | (Linux检测) | 禁用 sendfile 系统调用 |
| `--disable-debug` | no | 禁用调试日志 |
| `--disable-verbose` | no | 禁用详细调试日志 |
| `--enable-profiling` | no | 编译 profiling 代码（-pg） |
| `--with-dmalloc` | no | 链接 Dmalloc 内存调试器 |
| `--with-efence` | no | 链接 Electric Fence 内存调试器 |

## defines.h 可修改常量

| 宏 | 默认值 | 说明 |
|----|--------|------|
| `SERVER_ROOT` | `/etc/boa` | 配置文件搜索路径 |
| `DEFAULT_CONFIG_FILE` | `boa.conf` | 配置文件名 |
| `MIME_TYPES_DEFAULT` | `/etc/mime.types` | MIME 类型文件路径 |
| `SOCKETBUF_SIZE` | 32768 | Socket 缓冲区大小 |
| `BUFFER_SIZE` | 4096 | 每连接输出缓冲区 |
| `CLIENT_STREAM_SIZE` | 8192 | 客户端输入流缓冲区 |
| `MAX_HEADER_LENGTH` | 1024 | HTTP 头最大长度 |
| `REQUEST_TIMEOUT` | 60 | 请求超时（秒） |
| `MMAP_LIST_SIZE` | 256 | mmap 缓存条目数 |
| `MAX_FILE_MMAP` | 102400 | mmap 文件最大字节数（0=始终mmap） |
