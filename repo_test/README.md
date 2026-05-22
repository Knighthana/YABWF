---
Status: active
Authority: authoritative
Read-Tier: task-scoped
Purpose: 定义 YABWF 测试策略总览、目录结构、工具选择与执行规范
version: knighthana@0.1.0
last_update: 2026-05-23
---

# YABWF 测试策略

测试策略总览文档

---

## 测试分层

引用 `repo_memo/global/PATTERNS_ENGINEERING.md` 测试分层表：

| 层级 | 目录 | 工具 | 覆盖范围 |
|------|------|------|---------|
| 单元测试 | `test/unit/` | 独立编译的 C 测试程序（利用 `STANDALONE_TEST` 模式） | 纯函数：`unescape_uri` `clean_pathname` `range_parse` `boa_atoi` 等 |
| 集成测试 | `test/integration/` | Shell 脚本 + `curl` + 项目自带 CGI 示例 | HTTP 端到端：GET/POST/CGI/目录索引 |
| 安全测试 | `test/security/` | PoC 脚本 | CVE 复现与回归验证 |

## 目录结构

```
test/
├── unit/          # 单元测试 — 独立 C 程序，测试纯函数逻辑
├── integration/   # 集成测试 — Shell 脚本 + curl，测试 HTTP 端到端行为
└── security/      # 安全测试 — PoC 脚本，CVE 复现与回归
repo_test/
└── README.md      # 本文件 — 测试策略总览
```

- 测试代码存放于 `test/` 目录下，按三层结构分目录
- 测试策略文档存放于 `repo_test/` 目录下

## 工具选择

### 单元测试
- 独立编译的 C 测试程序，链接被测模块的目标文件
- 利用 `STANDALONE_TEST` 宏隔离测试入口（如 `escape.c` 已内置 `#ifdef TEST` 模式）
- 断言使用标准 `assert.h`，输出格式自定

### 集成测试
- Shell 脚本驱动，使用 `curl` 发送 HTTP 请求并验证响应
- 依赖项目自带的 CGI 示例（位于 `extras/` 或 `examples/`）
- 启动 `boa` 实例后执行测试用例，测试完成后清理

### 安全测试
- PoC（Proof of Concept）脚本，每个 CVE 对应一个独立测试用例
- 验证修复有效性，防止回归

## 执行规范

- 所有测试用例在提交前必须通过
- 新增功能须同时添加对应层级的测试用例
- 安全测试用例在引入新 CVE 修复时添加

## CVE 测试覆盖

| CVE | 测试文件 | 层级 | 验证内容 |
|-----|---------|------|---------|
| CVE-2009-4496 | `test/unit/test_sanitize_log_string.c` `test/security/test_poc_sanitize_log_string.sh` | 单元+安全 | 控制字符过滤正确性 |
| CVE-2000-0920 | — | 审查 | `clean_pathname` 防御链（已确认） |
| CVE-2022-45956 | `test/integration/test_cve_2022_45956.sh` | 集成 | HEAD 方法尊重 Allow/Deny 规则 |
| CVE-2018-21028 | `test/unit/test_strstr_fallback.c` | 单元 | `strstr` fallback 正确性 |
| CGI Location | `test/unit/test_cgi_header_location.c` `test/security/test_poc_cgi_header_location.sh` | 单元+安全 | Location 头安全拒绝 |
| CGI Strip | `test/unit/test_cgi_strip_prefix.c` `test/integration/test_cgi_strip_prefix.sh` | 单元+集成 | 前缀剥离功能正确性 |
| Cross-compile | `test/integration/test_verify_cross.sh` | 集成 | 交叉编译验证 |
