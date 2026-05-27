# TopProcess Plugin for TrafficMonitor

[English Version](README.md)

一个 TrafficMonitor 插件，在 Windows 任务栏显示 CPU 和内存占用最高的进程。

## 功能

- **CPU 占用最高进程** — 显示 CPU 使用率最高的应用名称和百分比
- **内存占用最高进程** — 显示内存占用最高的应用名称和百分比
- 多进程应用（Chrome、VS Code 等）自动按进程名合并
- 系统进程（Idle、System、Memory Compression）自动排除

## 显示效果

```
chrome:15.2%    chrome:8.6%
```

无标签前缀，设计为与 TrafficMonitor 原生 CPU/内存显示并排使用。

## 编译

### 环境要求

- MSYS2 MinGW64（支持 C++17 的 g++）
- Windows 10/11

### 编译步骤

```bash
cd TopProcessPlugin
bash build_mingw64.sh
```

生成 `TopProcessPlugin.dll`。

### 备选：Visual Studio

用 Visual Studio 2022 打开 `TopProcessPlugin.vcxproj`，选择 `Release | x64` 配置，生成即可。

## 安装使用

1. 将 `TopProcessPlugin.dll` 复制到 TrafficMonitor 安装目录下的 `plugins` 文件夹
2. 重启 TrafficMonitor
3. 右键任务栏图标 → 显示项目 → 勾选"CPU占用最高进程"和"内存占用最高进程"

## 工作原理

### CPU 使用率

- 使用 `GetProcessTimes` 计算每个进程在采样间隔内的 CPU 时间增量
- 按进程名合并所有同名实例（如 20 个 chrome.exe → 一个 "chrome" 条目）
- 除以逻辑处理器数量得到准确百分比

### 内存使用率

- 使用 `GetProcessMemoryInfo` → `WorkingSetSize` 获取每个进程的物理内存占用
- 按进程名合并（处理 Chrome、飞书等多进程应用）
- **百分比公式**：`进程 WorkingSetSize ÷ 物理内存已使用量 × 100%`
- 分母为 `总物理内存 - 可用物理内存`（来自 `GlobalMemoryStatusEx`）

关于 Windows 内存指标的详细分析和方案选择过程，请参阅 [内存测算方案文档](MEMORY_RESEARCH.md)。

## 文件结构

```
TopProcessPlugin/
├── PluginInterface.h          - TrafficMonitor 插件接口定义
├── TopProcessPlugin.h/cpp     - 主插件类 (ITMPlugin)
├── TopCpuProcessItem.h/cpp    - CPU 最高进程显示项 (IPluginItem)
├── TopMemoryProcessItem.h/cpp - 内存最高进程显示项 (IPluginItem)
├── ProcessInfo.h              - 进程枚举与指标获取
├── pch.h/cpp                  - 预编译头
├── framework.h                - 框架头文件
├── build_mingw64.sh           - MinGW 编译脚本
├── MEMORY_RESEARCH.md         - 内存测算方案文档
└── README.md                  - 英文说明
```

## 注意事项

- CPU 使用率需要两个采样周期才能准确，首次运行可能显示 0%
- 进程名超过 15 个字符会截断并显示 "..."
- 系统空闲进程 (PID 0)、System (PID 4) 和 Memory Compression 会被过滤
- 部分系统服务（svchost、csrss 等）因权限限制无法访问，对排名结果影响可忽略

## 许可证

MIT License
