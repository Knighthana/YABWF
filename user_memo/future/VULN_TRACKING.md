---
Status: active
Authority: authoritative
Read-Tier: on-demand
Purpose: 待评估安全漏洞追踪表——已评估完毕，记录每项判定
version: knighthana@0.2.0
last_update: 2026-05-23
---

漏洞评估结果
===========

以下为 12 项漏洞的判定结果。已处理的移入 `SECURITY.md`，需代码审查的移入 `STATES.md`。

---

## ⬜ 不适用（厂商定制代码，YABWF 无对应路径）

| 编号 | 日期 | 来源 | 判定依据 |
|------|------|------|---------|
| CVE-2017-9833 | 2017-06-24 | NVD | **DISPUTED**。`/cgi-bin/wapopen` + FILECAMERA 变量是系统集成商代码，Boa 上游不含 wapopen 程序。NVD 标注为 disputed |
| CVE-2024-40088 | 2024-10-21 | NVD | Vilo 5 Mesh WiFi 厂商设备的 Boa webserver 目录穿越。影响 Vilo 固件而非上游 Boa |
| CVE-2025-8757 | 2025-08-09 | VulDB | TRENDnet TV-IP110WN 嵌入式 Boa 的 `/server/boa.conf` 最小权限违规。需要本地访问且复杂度高 |
| CVE-2025-7910 | 2025-07-20 | VulDB | D-Link DIR-513 的 `/goform/formSetWanNonLogin` 中 sprintf 栈溢出。**Unsupported When Assigned**，已停止维护的 EOL 设备 |

---

## 🔄 与已有 CVE 重复（合并处理）

| 标识 | 重复于 | 说明 |
|------|--------|------|
| EDB-42290 | CVE-2017-9833 | 同一 exploit：`/cgi-bin/wapopen` 路径穿越。Exploit-DB 42290 即 CVE-2017-9833 的 PoC |
| WA-20221128001 | CVE-2022-45956 | 澳大利亚 WA SOC 公告（2022-11-28）与 CVE-2022-45956 同日，描述同为 HEAD 方法绕过认证 |

---

## 🔴 需要代码审查（YABWF 上游 Boa 相关）

以下 3 项确认为上游 Boa 漏洞，需要审查 YABWF 代码确认是否存在：

| 编号 | 日期 | CVSS | 描述 | 审查重点 |
|------|------|------|------|---------|
| CVE-2018-21028 | 2019-10-11 | 7.5 HIGH | Boa ≤0.94.14rc21 缺少 `free()` 调用导致内存泄漏。GitHub PR 有 patch | 审查 `request.c` `free_request()` 和 `hash.c` 各 `hash_clear()`/`dump_*()` 函数是否完整释放内存 |
| CVE-2022-45956 | 2022-12-12 | 5.3 MEDIUM | Boa 0.94.13-0.94.14 的 HEAD 方法绕过 Basic Authorization。PacketStorm 有 exploit | 审查 `get.c` 中 HEAD 方法处理路径，确认是否跳过 access 检查 |
| CVE-2024-47916 | 2024-11-14 | 7.5 HIGH | Boa web server CWE-22 路径穿越。以色列国家网络局发布，细节未公开 | 审查 `unescape_uri`→`clean_pathname`→`translate_uri` 全链路。与 CVE-2000-0920 同类，需确认是否有新绕过 |

---

## ❓ 待获取详情

| 编号 | 来源 | 备注 |
|------|------|------|
| CVE-2025-7909 | cve.org | 未获取详情 |
| CVE-2026-1687 | cve.org | 未获取详情 |
| VulDB-38805 | VulDB | 未获取详情 |

---

**后续动作**：3 项 🔴 标记的 CVE 进入下轮审查队列（`STATES.md` T-003）。4 项 ⬜ + 2 项 🔄 移入 `SECURITY.md`。
