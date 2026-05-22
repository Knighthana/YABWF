---
Status: active
Authority: authoritative
Read-Tier: always
Purpose: 记录当前生效的架构决策，供 agent 在执行任务时参考
version: knighthana@0.1.0
last_update: 2026-05-23
---

# Active Decisions

## D-001: SPEC 驱动开发
项目采用 SPEC 驱动开发模式，所有实现任务前必须先确认对应模块的 SPEC 文档存在。

## D-002: CVE 修复优先级
CVE 修复按优先级排列：日志/信息泄露 → 输入验证/缓冲区溢出。

## D-003: 厂商专属 CVE
厂商专属 CVE 不进行代码修复，仅归档记录。

## D-004: 测试策略
采用 STANDALONE_TEST 单测 + 黑盒 HTTP 集成测试的双层测试策略。

## D-005: 交叉编译验证三级矩阵
交叉编译验证分三级：编译通过 → QEMU 烟气测试 → 物理硬件测试。

## D-006: 编译系统
编译系统暂不迁移 CMake，仅更新 config.sub/config.guess 以保持对新架构的支持。
