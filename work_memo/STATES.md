# Work State

## Current Phase

CVE 修复与功能开发 —— 基础设施就绪，进入迭代开发

## Active Decisions

- D-001: 项目采用 SPEC 驱动开发
- D-002: CVE 修复优先级：日志/信息泄露 → 输入验证/缓冲区
- D-003: 厂商专属 CVE 不进行代码修复，仅归档
- D-004: 测试策略：STANDALONE_TEST 单测 + 黑盒 HTTP 集成测试
- D-005: 交叉编译验证三级矩阵：编译通过 → QEMU 烟气测试 → 物理硬件
- D-006: 编译系统暂不迁移 CMake，仅更新 config.sub/config.guess
- D-007: boa→yabwf 硬分叉重命名推迟，当前仅统一日志前缀

## Pending Tasks（后续轮次评估）

- T-001: CGI → FastCGI（去掉 fork 模型，减少进程创建开销）
- T-002: epoll 编译期可选模型（替代 select，高并发场景）
- T-003: 新漏洞审查 — ✅ 已清零。CVE-2018-21028 已修复，CVE-2024-47916 基线已安全 |
