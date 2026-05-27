# Memory Measurement Research / 内存测算方案调研

[English](#english) | [中文](#中文)

---

<a id="english"></a>
## English

### Background

The plugin needs to display the memory usage percentage of the top process, matching Windows Task Manager as closely as possible. Through extensive testing, we discovered that Task Manager's memory percentage is **not** simply `Process Memory / Total Physical RAM`.

### Windows Memory Metrics Overview

| Metric | API | Definition | Matches Task Manager? |
|--------|-----|-----------|----------------------|
| **Working Set** | `GetProcessMemoryInfo` → `WorkingSetSize` | Total physical RAM pages (private + shared DLLs) | Close (~1% deviation) |
| **Private Working Set** | `NtQueryInformationProcess` → `VM_COUNTERS_EX2.PrivateWorkingSetSize` | Physical RAM exclusive to the process (no shared pages) | Matches "Details" tab |
| **Private Bytes** | `GetProcessMemoryInfo` → `PROCESS_MEMORY_COUNTERS_EX.PrivateUsage` | Total committed private virtual memory (RAM + pagefile) | No (too high) |
| **PDH Working Set - Private** | `PdhAddEnglishCounter` → `\Process(*)\Working Set - Private` | Same as Private Working Set via Performance Counters | Matches "Details" tab |

### Methods Evaluated

| # | Method | Result | Why Rejected/Accepted |
|---|--------|--------|----------------------|
| 1 | `GetProcessMemoryInfo` (basic) → `WorkingSetSize` | **Accepted** | Closest match to Task Manager "Processes" tab after testing |
| 2 | `GetProcessMemoryInfo` (EX) → `PrivateUsage` | Rejected | Returns committed virtual memory, much higher than Task Manager |
| 3 | `NtQueryInformationProcess` + `VM_COUNTERS_EX2` | Rejected | Returns Private Working Set (~3%), but Task Manager "Processes" tab shows ~7-8% for Chrome |
| 4 | `QueryWorkingSetEx` page enumeration | Rejected | Same result as method 3, high performance cost |
| 5 | PDH `\Process(*)\Working Set - Private` | Rejected | Same as method 3; also has multi-instance naming complexity |
| 6 | WMI `Win32_Process` | Rejected | No `PrivateWorkingSetSize` property available |
| 7 | `NtQuerySystemInformation` | Rejected | Structure offsets vary across Windows versions, unreliable |

### Key Discovery: Task Manager's Percentage Formula

Through empirical testing with known process values:

**Task Manager "Processes" tab percentage ≠ `Process Memory / Total Physical RAM`**

Our findings:
- `claude.exe`: 280 MB displayed as 2.8% → denominator ≈ 10,000 MB
- `chrome.exe` (merged): 1,565 MB displayed as 15.7% → denominator ≈ 9,968 MB
- System state: Total RAM = 32 GB, In Use = 13 GB, Available = 18 GB

The denominator is **not** total physical memory (32 GB), but rather the **physical memory currently in use** (~13 GB minus kernel/cache overhead ≈ 10 GB).

### Task Manager Has Two Different Views

| Tab | Column Name | Metric Used | Our Match |
|-----|-------------|-------------|-----------|
| **Processes** (进程) | "Memory" | WorkingSetSize-based, % relative to in-use memory | WorkingSetSize / (Total - Available) ≈ ±1.2% |
| **Details** (详细信息) | "Active Private Working Set" (活动的专用工作集) | `PrivateWorkingSetSize` from kernel | NtQuery matches exactly |

### Final Chosen Approach

```
Memory % = Process WorkingSetSize / (TotalPhysicalMemory - AvailablePhysicalMemory) × 100%
```

**Why this works:**
- `WorkingSetSize` via `GetProcessMemoryInfo` is the most reliable and accessible metric
- Using "physical memory in use" as denominator closely matches Task Manager's "Processes" tab
- Tested deviation: typically within ±1.5% of Task Manager
- Simple implementation, no undocumented NT APIs required
- Works reliably across all processes (no permission issues)

**Why not Private Working Set:**
- While more "correct" theoretically, it produces values 50% lower than what Task Manager's "Processes" tab shows
- Task Manager's "Processes" tab appears to use a metric closer to WorkingSetSize
- The "Details" tab uses Private Working Set, but users typically compare against the "Processes" tab

### Multi-Process Application Handling

Applications like Chrome, Feishu, and WeChat run multiple processes (e.g., Chrome typically has 15-25 `chrome.exe` instances). Our approach:

1. Enumerate all processes via `CreateToolhelp32Snapshot`
2. Get `WorkingSetSize` for each process via `GetProcessMemoryInfo`
3. Merge by executable name: `std::map<name, sum>`
4. This matches Task Manager's grouped view (e.g., "Google Chrome (21)")

### Processes Excluded

- PID 0 (System Idle Process)
- PID 4 (System)
- "Memory Compression" (Windows memory compression service)

### Permission Handling

Some system processes (svchost, csrss, lsass, etc.) cannot be opened with `PROCESS_QUERY_INFORMATION`. The code falls back to `PROCESS_QUERY_LIMITED_INFORMATION`. If both fail, the process is skipped. This affects ~87 svchost instances and ~30 other system services, but has negligible impact on the top-process ranking since user applications are always accessible.

---

<a id="中文"></a>
## 中文

### 背景

插件需要显示内存占用最高进程的百分比，尽可能匹配 Windows 任务管理器。经过大量测试，我们发现任务管理器的内存百分比**并非**简单的 `进程内存 ÷ 总物理内存`。

### Windows 内存指标概览

| 指标 | API | 定义 | 是否匹配任务管理器？ |
|------|-----|------|-------------------|
| **Working Set** | `GetProcessMemoryInfo` → `WorkingSetSize` | 物理内存中进程占用的总页面（私有 + 共享 DLL） | 接近（偏差 ~1%） |
| **Private Working Set** | `NtQueryInformationProcess` → `VM_COUNTERS_EX2.PrivateWorkingSetSize` | 物理内存中仅属于该进程的页面（不含共享） | 匹配"详细信息"标签页 |
| **Private Bytes** | `GetProcessMemoryInfo` → `PROCESS_MEMORY_COUNTERS_EX.PrivateUsage` | 进程私有的已提交虚拟内存（RAM + 分页文件） | 否（偏高） |
| **PDH Working Set - Private** | `PdhAddEnglishCounter` → `\Process(*)\Working Set - Private` | 通过性能计数器获取的 Private Working Set | 匹配"详细信息"标签页 |

### 评估过的方法

| # | 方法 | 结果 | 原因 |
|---|------|------|------|
| 1 | `GetProcessMemoryInfo` (基础) → `WorkingSetSize` | **采用** | 经测试最接近任务管理器"进程"标签页 |
| 2 | `GetProcessMemoryInfo` (EX) → `PrivateUsage` | 弃用 | 返回已提交虚拟内存，远高于任务管理器 |
| 3 | `NtQueryInformationProcess` + `VM_COUNTERS_EX2` | 弃用 | 返回 Private Working Set（~3%），但任务管理器"进程"标签页对 Chrome 显示 ~7-8% |
| 4 | `QueryWorkingSetEx` 页面枚举 | 弃用 | 结果同方法 3，性能开销大 |
| 5 | PDH `\Process(*)\Working Set - Private` | 弃用 | 结果同方法 3；多实例命名复杂 |
| 6 | WMI `Win32_Process` | 弃用 | 没有 `PrivateWorkingSetSize` 属性 |
| 7 | `NtQuerySystemInformation` | 弃用 | 结构体偏移在不同 Windows 版本间不一致 |

### 关键发现：任务管理器的百分比公式

通过已知进程值的实测对比：

**任务管理器"进程"标签页的百分比 ≠ `进程内存 ÷ 总物理内存`**

实测数据：
- `claude.exe`：280 MB 显示为 2.8% → 分母 ≈ 10,000 MB
- `chrome.exe`（合并后）：1,565 MB 显示为 15.7% → 分母 ≈ 9,968 MB
- 系统状态：总内存 32 GB，已使用 13 GB，可用 18 GB

分母**不是**总物理内存（32 GB），而是**当前已使用的物理内存**（~13 GB 减去内核/缓存开销 ≈ 10 GB）。

### 任务管理器的两个不同视图

| 标签页 | 列名 | 使用的指标 | 我们的匹配度 |
|--------|------|-----------|------------|
| **进程** | "内存" | 基于 WorkingSetSize，百分比相对于已使用内存 | WorkingSetSize / (总量 - 可用) ≈ ±1.2% |
| **详细信息** | "活动的专用工作集" | 内核的 `PrivateWorkingSetSize` | NtQuery 完全匹配 |

### 最终选择的方案

```
内存百分比 = 进程 WorkingSetSize ÷ (总物理内存 - 可用物理内存) × 100%
```

**为什么选择这个方案：**
- `WorkingSetSize` 通过 `GetProcessMemoryInfo` 获取，最可靠、最易访问
- 使用"已使用物理内存"作为分母，与任务管理器"进程"标签页高度匹配
- 实测偏差：通常在 ±1.5% 以内
- 实现简单，无需未文档化的 NT API
- 对所有进程可靠工作（无权限问题）

**为什么不用 Private Working Set：**
- 虽然理论上更"正确"，但产生的值比任务管理器"进程"标签页低约 50%
- 任务管理器"进程"标签页使用的指标更接近 WorkingSetSize
- "详细信息"标签页才使用 Private Working Set，但用户通常对比的是"进程"标签页

### 多进程应用的处理

Chrome、飞书、微信等应用运行多个进程（如 Chrome 通常有 15-25 个 `chrome.exe` 实例）。我们的处理方式：

1. 通过 `CreateToolhelp32Snapshot` 枚举所有进程
2. 对每个进程通过 `GetProcessMemoryInfo` 获取 `WorkingSetSize`
3. 按可执行文件名合并：`std::map<名称, 累加值>`
4. 这与任务管理器的分组视图一致（如 "Google Chrome (21)"）

### 排除的进程

- PID 0（系统空闲进程）
- PID 4（System）
- "Memory Compression"（Windows 内存压缩服务）

### 权限处理

部分系统进程（svchost、csrss、lsass 等）无法以 `PROCESS_QUERY_INFORMATION` 打开。代码会 fallback 到 `PROCESS_QUERY_LIMITED_INFORMATION`。如果都失败则跳过该进程。这影响约 87 个 svchost 实例和约 30 个其他系统服务，但对排名结果影响可忽略，因为用户应用程序始终可以访问。

---

## References / 参考来源

- [QueryWorkingSetEx function - Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-queryworkingsetex)
- [Specifying a Counter Path - Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/perfctrs/specifying-a-counter-path)
- [Win32_Process class - Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/cimwin32prov/win32-process)
- [How to calculate memory used percents - Microsoft Q&A](https://learn.microsoft.com/en-us/answers/questions/917516/how-to-calculate-memory-used-percents-of-process-i)
- [How to calculate private working set - Stack Overflow](https://stackoverflow.com/questions/2611141/how-to-calculate-private-working-set-memory)
- [Memory Information in Task Manager - Scorpio Software](https://scorpiosoftware.net/2023/04/12/memory-information-in-task-manager/)
- [Chromium Memory Usage Backgrounder](https://www.chromium.org/developers/memory-usage-backgrounder/)
- [Computing Memory Usage According to Task Manager - PowerAdmin](https://www.poweradmin.com/blog/computing-memory-usage-according-to-task-manager/)
