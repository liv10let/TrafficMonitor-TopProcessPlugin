# TopProcess Plugin for TrafficMonitor

这是一个 TrafficMonitor 插件，用于在任务栏显示 CPU 和内存占用最高的进程。

## 功能

- **CPU 占用最高的进程** - 显示当前 CPU 使用率最高的进程名称和占用百分比
- **内存占用最高的进程** - 显示当前内存使用量最高的进程名称和占用大小

## 编译要求

- Visual Studio 2022 (v143 工具集)
- Windows 10 SDK
- MFC 动态链接库

## 编译步骤

1. 用 Visual Studio 2022 打开 `TopProcessPlugin.vcxproj`
2. 选择 `Release | x64` 配置（推荐）
3. 生成解决方案 (Ctrl+Shift+B)
4. 编译后的 DLL 会输出到 `Bin\x64\Release\plugins\` 目录

## 安装使用

1. 编译插件得到 `TopProcessPlugin.dll`
2. 将 DLL 复制到 TrafficMonitor 主程序目录下的 `plugins` 文件夹
3. 启动 TrafficMonitor
4. 右键任务栏图标 → 其他功能 → 插件管理，确认插件已加载
5. 右键任务栏图标 → 显示项目，勾选 "Top CPU Process" 和 "Top Memory Process"

## 显示效果

在任务栏中会显示类似以下内容：

```
CPU:chrome:15.2%  MEM:chrome:1.2GB
```

## 注意事项

- CPU 使用率的计算需要两个采样周期才能准确，第一次运行时可能显示为 0%
- 进程名称超过 15 个字符会被截断并显示 "..."
- 系统空闲进程 (PID 0) 和 System 进程 (PID 4) 会被自动过滤

## 文件结构

```
TopProcessPlugin/
├── PluginInterface.h       - TrafficMonitor 插件接口定义
├── TopProcessPlugin.h/cpp  - 主插件类 (ITMPlugin)
├── TopCpuProcessItem.h/cpp - CPU 最高进程显示项 (IPluginItem)
├── TopMemoryProcessItem.h/cpp - 内存最高进程显示项 (IPluginItem)
├── ProcessInfo.h           - 进程信息获取工具类
├── pch.h/cpp              - 预编译头
├── framework.h            - 框架头文件
├── TopProcessPlugin.vcxproj - VS 项目文件
└── README.md              - 本文件
```

## 许可证

MIT License
