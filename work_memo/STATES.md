# Work State

## Current Phase

Infrastructure (Step 1-2) — 文档体系 + 交叉编译支持建立中

## Active Decisions

- D-001: 项目采用 SPEC 驱动开发
- D-002: CVE 修复优先级：日志/信息泄露 → 输入验证/缓冲区
- D-003: 厂商专属 CVE 不进行代码修复，仅归档
- D-004: 测试策略：STANDALONE_TEST 单测 + 黑盒 HTTP 集成测试
- D-005: 交叉编译验证三级矩阵：编译通过 → QEMU 烟气测试 → 物理硬件
- D-006: 编译系统暂不迁移 CMake，仅更新 config.sub/config.guess

## Pending Tasks

- Step 1: 文档基础设施
- Step 2: 交叉编译支持更新
- Step 3: 非 CVE Bug 修复（cgi_header.c + hash.c）
- Step 4: CVE 验证与修复队列
