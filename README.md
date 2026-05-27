# TopProcess Plugin for TrafficMonitor

[中文版](README_CN.md)

A TrafficMonitor plugin that displays the process with the highest CPU and memory usage in the Windows taskbar.

## Features

- **Top CPU Process** — Shows the process name and CPU usage percentage of the most CPU-intensive application
- **Top Memory Process** — Shows the process name and memory usage percentage of the most memory-intensive application
- Multi-process applications (Chrome, VS Code, etc.) are automatically merged by process name
- System processes (Idle, System, Memory Compression) are excluded

## Display Format

```
chrome:15.2%    chrome:8.6%
```

No label prefix — designed to sit alongside TrafficMonitor's native CPU/Memory displays.

## Build

### Requirements

- MSYS2 MinGW64 (g++ with C++17 support)
- Windows 10/11

### Compile

```bash
cd TopProcessPlugin
bash build_mingw64.sh
```

This produces `TopProcessPlugin.dll`.

### Alternative: Visual Studio

Open `TopProcessPlugin.vcxproj` with Visual Studio 2022, select `Release | x64`, and build.

## Installation

1. Copy `TopProcessPlugin.dll` to the `plugins` folder under your TrafficMonitor installation directory
2. Restart TrafficMonitor
3. Right-click the taskbar widget → Display Items → Enable "CPU占用最高进程" and "内存占用最高进程"

## How It Works

### CPU Usage

- Uses `GetProcessTimes` to calculate per-process CPU delta over sampling intervals
- Merges all instances of the same process name (e.g., 20 chrome.exe → one "chrome" entry)
- Divides by number of logical processors for accurate percentage

### Memory Usage

- Uses `GetProcessMemoryInfo` → `WorkingSetSize` for each process
- Merges by process name (handles multi-process apps like Chrome, Feishu, etc.)
- **Percentage formula**: `Process WorkingSetSize / Physical Memory In Use × 100%`
- The denominator is `TotalPhysicalMemory - AvailablePhysicalMemory` (from `GlobalMemoryStatusEx`)

For a detailed analysis of Windows memory metrics and why this approach was chosen, see [Memory Research](MEMORY_RESEARCH.md).

## File Structure

```
TopProcessPlugin/
├── PluginInterface.h          - TrafficMonitor plugin interface
├── TopProcessPlugin.h/cpp     - Main plugin class (ITMPlugin)
├── TopCpuProcessItem.h/cpp    - CPU top process display item (IPluginItem)
├── TopMemoryProcessItem.h/cpp - Memory top process display item (IPluginItem)
├── ProcessInfo.h              - Process enumeration and metrics
├── pch.h/cpp                  - Precompiled header
├── framework.h                - Framework header
├── build_mingw64.sh           - MinGW build script
├── MEMORY_RESEARCH.md         - Memory measurement research document
└── README.md                  - This file
```

## Notes

- CPU usage requires two sampling cycles to be accurate; first reading may show 0%
- Process names longer than 15 characters are truncated with "..."
- System Idle (PID 0), System (PID 4), and Memory Compression are filtered out
- Some system services (svchost, csrss, etc.) may not be accessible due to permission restrictions; this has negligible impact on the top-process ranking

## License

MIT License
