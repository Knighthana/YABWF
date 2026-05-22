---
Status: active
Authority: authoritative
Read-Tier: task-scoped
Purpose: 定义 YABWF 交叉编译验证三级兼容性矩阵，指导跨平台构建与测试流程
version: knighthana@0.1.0
last_update: 2026-05-23
---

# 交叉编译兼容性验证

三级兼容性验证矩阵

---

## 验证矩阵

| 级别 | 名称 | 覆盖范围 | 执行频率 | 环境要求 |
|------|------|---------|---------|---------|
| Level 1 | 编译验证 | `{ glibc, musl, uclibc-ng } × { x86_64, aarch64, armv7l }` | 每 commit | 交叉编译工具链 |
| Level 2 | QEMU 烟气测试 | 同 Level 1 | PR 或夜间 | QEMU user-mode |
| Level 3 | 物理硬件测试 | 目标平台实际硬件 | 发布前 | 物理设备 |

## 当前阶段

当前仅落地 **Level 1**（编译验证）。Level 2 和 Level 3 纳入后续路线图。

## Level 1 编译验证命令示例

以下为各 libc + arch 组合的典型编译命令：

### x86_64 + glibc（原生编译）
```sh
./configure --build=x86_64-linux-gnu --host=x86_64-linux-gnu
make -j$(nproc)
```

### x86_64 + musl
```sh
CC=musl-gcc ./configure --build=x86_64-linux-gnu --host=x86_64-linux-gnu
make -j$(nproc)
```

### aarch64 + glibc
```sh
HOST=aarch64-linux-gnu CC=aarch64-linux-gnu-gcc ./configure --build=x86_64-linux-gnu --host=aarch64-linux-gnu
make -j$(nproc)
```

### aarch64 + musl
```sh
HOST=aarch64-linux-musl CC=aarch64-linux-musl-gcc ./configure --build=x86_64-linux-gnu --host=aarch64-linux-musl
make -j$(nproc)
```

### armv7l + glibc
```sh
HOST=arm-linux-gnueabihf CC=arm-linux-gnueabihf-gcc ./configure --build=x86_64-linux-gnu --host=arm-linux-gnueabihf
make -j$(nproc)
```

### armv7l + musl
```sh
HOST=arm-linux-musleabihf CC=arm-linux-musleabihf-gcc ./configure --build=x86_64-linux-gnu --host=arm-linux-musleabihf
make -j$(nproc)
```

### aarch64 + uclibc-ng
```sh
HOST=aarch64-linux-uclibc CC=aarch64-linux-uclibc-gcc ./configure --build=x86_64-linux-gnu --host=aarch64-linux-uclibc
make -j$(nproc)
```

### armv7l + uclibc-ng
```sh
HOST=arm-linux-uclibceabihf CC=arm-linux-uclibceabihf-gcc ./configure --build=x86_64-linux-gnu --host=arm-linux-uclibceabihf
make -j$(nproc)
```

## 使用 `construct.sh` 简化验证

```sh
# x86_64 原生编译
./construct.sh build

# aarch64 交叉编译验证
HOST=aarch64-linux-gnu CC=aarch64-linux-gnu-gcc ./construct.sh verify-cross

# armv7l 交叉编译验证
HOST=arm-linux-gnueabihf CC=arm-linux-gnueabihf-gcc ./construct.sh verify-cross
```

## 验证通过标准

- `configure` 成功退出（返回码 0）
- `make` 成功完成（返回码 0），零 warning
- 产物 `src/boa` 和 `src/boa_indexer` 生成成功
