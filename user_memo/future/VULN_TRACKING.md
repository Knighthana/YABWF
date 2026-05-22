---
Status: future
Authority: reference-only
Read-Tier: on-demand
Purpose: 待评估安全漏洞追踪表——记录尚未分析的 CVE 及其他来源的安全问题，供后续轮次逐一评估
version: knighthana@0.1.0
last_update: 2026-05-23
---

待评估漏洞追踪
=============

本文件记录尚未评估的已知漏洞。每个漏洞经评估后：
- 若与 YABWF 相关且可修复 → 移至 README CVE 表 + `CVE_ANALYSIS.md`
- 若基线已安全 → 移至 README CVE 表（标注"基线已安全"）
- 若不适用 → 移至 README CVE 表（标注"不适用"）

---

## CVE 来源

| 编号 | 日期 | 来源 | 状态 | 摘要 |
|------|------|------|------|------|
| CVE-2017-9833 | 2017-06-24 | NVD | 待评估 | Boa 0.94.14rc21 中 /cgi-bin/ 路径穿越 |
| CVE-2018-21028 | 2019-10-10 | NVD | 待评估 | Boa 0.94.14rc21 未正确处理连续请求导致信息泄露 |
| CVE-2022-45956 | 2022-11-28 | NVD | 待评估 | Boa Web Server 0.94.14rc21 目录穿越 |
| CVE-2024-40088 | 2024 | NVD | 待评估 | Boa 相关漏洞 |
| CVE-2024-47916 | 2024 | NVD | 待评估 | Boa 相关漏洞 |
| CVE-2025-7909 | 2025 | cve.org | 待评估 | Boa 相关漏洞 |
| CVE-2025-7910 | 2025 | NVD | 待评估 | Boa 相关漏洞 |
| CVE-2025-8757 | 2025 | NVD | 待评估 | Boa 相关漏洞 |
| CVE-2026-1687 | 2026 | cve.org | 待评估 | Boa 相关漏洞 |

## 非 CVE 来源

| 标识 | 日期 | 来源 | 状态 | 摘要 |
|------|------|------|------|------|
| WA-20221128001 | 2022-11-28 | WA SOC Advisory | 待评估 | Boa Web Server 漏洞公告 |
| EDB-42290 | 2017 | Exploit-DB | 待评估 | Boa HTTP 服务器 exploit |
| VulDB-38805 | 未知 | VulDB | 待评估 | Boa 相关漏洞 |
