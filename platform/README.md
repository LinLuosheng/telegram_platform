# Telegram C2 Web 平台 (Web Platform)

本项目包含 C2 系统的服务端（后端）和管理界面（前端）。

## 目录结构
- `backend/`: Spring Boot 后端服务
- `frontend/`: React + Ant Design Pro 前端界面

---

## 1. 后端 (Backend)

基于 `springboot-init-master` 开发，提供 API 接口、数据存储和任务队列。

### 技术栈
- **框架**: Spring Boot 2.7.x
- **数据库**: MySQL 5.7+ (存储设备、任务、文件索引)
- **缓存**: Redis (Session, 缓存)
- **ORM**: MyBatis Plus

### 核心功能
- **API 端点**:
    - `/api/c2/heartbeat`: 设备心跳注册。
    - `/api/c2/tasks/**`: 任务下发与结果接收。
    - `/api/c2Device/**`: 设备管理（Web 端使用）。
- **数据处理**: 解析客户端上传的 SQLite 数据库 (`tdata_client.db`, `scan_results.db`) 并入库。

### 启动说明
1. 进入目录: `cd backend/springboot-init-master`
2. 配置数据库: 修改 `src/main/resources/application.yml` 中的 MySQL 和 Redis 配置。
3. 运行:
   ```bash
   mvn spring-boot:run
   ```
   或在 IDEA 中运行 `MainApplication.java`。
4. 服务端口: **8101**

---

## 2. 前端 (Frontend)

基于 Ant Design Pro 的管理后台，用于监控设备和下发指令。

### 技术栈
- **框架**: React, UmiJS
- **UI 组件**: Ant Design

### 核心功能
- **设备列表**: 查看在线/离线设备，显示 IP、地理位置、最后上线时间。
- **设备详情**:
    - **软件列表**: 查看已安装软件。
    - **WiFi 记录**: 查看扫描到的 WiFi 热点。
    - **文件浏览器**: 浏览远程文件系统。
    - **截图相册**: 查看历史截图。
- **任务控制台**: 发送 CMD 命令、截图指令、文件上传指令。

### 启动说明
1. 进入目录: `cd frontend/yupi-antd-frontend-init-master`
2. 安装依赖:
   ```bash
   npm install
   ```
3. 启动开发服务器:
   ```bash
   npm start
   ```
4. 访问地址: **http://localhost:8000**
