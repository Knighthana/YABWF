---
Status: future
Authority: reference-only
Read-Tier: on-demand
Purpose: 记录已确认但当前轮次（L2/L3）不执行的代码改进项，供后续计划参考
version: knighthana@0.1.0
last_update: 2026-05-23
---

# 未来代码改进项

以下改进项已确认有价值但不在本轮执行范围，按优先级排列。

---

## 1. response.c — 抽象重复的 send_r_* 函数为模板函数

`response.c` 中存在多个 `send_r_*` 函数（`send_r_bad_request`、`send_r_error`、`send_r_moved_temp` 等），其核心逻辑相似（构造响应头 + 写入缓冲区）。建议抽象为一个模板函数以减少代码重复。

## 2. get.c — 拆分为 file_handler.c + dir_handler.c

`get.c` 当前同时处理静态文件服务和目录索引两种职责。建议拆分为：
- `file_handler.c`：静态文件响应逻辑
- `dir_handler.c`：目录索引与默认文档逻辑

## 3. alias.c — 路径拼接逻辑封装为统一安全函数

`alias.c` 中存在多处路径拼接操作（如虚拟路径与实际路径的组合），当前分散在各处。建议封装为一个统一的安全路径拼接函数，内置边界检查和规范化。

## 4. config.c — `strtol` 溢出检查

`config.c` 中使用 `strtol` 解析配置数值，但未对超出范围的值做溢出保护。建议增加范围校验逻辑。

## 5. 全局变量逐步收敛为配置结构体

当前全局变量分散声明于各 `.c` 文件和 `globals.h` 中，缺乏统一管理。建议逐步收敛为单一配置结构体，通过指针传递。

## 6. `simple_itoa` — 改为调用者提供 buffer（非 static）

`util.c` 中的 `simple_itoa` 使用 `static` 内部缓冲区，非线程安全且调用者无法控制缓冲区生命周期。建议改为调用者传入目标 buffer 和长度参数。
