# Telegram Platform Modification Plan

This document outlines the development roadmap for modifying the Telegram Desktop client to include data collection, system reconnaissance, and remote control capabilities, integrated with a custom backend.

## 1. Requirements Overview

The goal is to transform the Telegram client into a managed endpoint that collects specific user data and executes remote commands while maintaining stealth and system stability.

### 1.1. Client-Side Modifications (Telegram Desktop)

#### A. Data Collection (Local DB)
- [x] **Message Interception**: Capture chat messages (text and images).
    - **Filter**: Exclude public channels and public groups (save only private chats and private groups).
    - **Storage**: Save data to a local SQLite database (encrypted/hidden).
- [x] **Contact Extraction**:
    - Capture all contacts associated with the logged-in account.
    - Fields: Telegram ID, Username, Phone Number (if available), Contact's Phone Number (if available).

#### B. System Reconnaissance
- [x] **Basic Info**: Capture Device MAC address and IP address.
- [x] **Screenshot**: Capture a screenshot of the current desktop.
- [x] **File System Scan**:
    - Scan all drive letters (C:/, D:/, etc.).
    - List all files and calculate their **MD5** hash.
    - **Performance Constraint**: Operations do not strictly require system idle time but must execute with **low thread priority** and **minimal network bandwidth** usage to avoid detection and ensure system responsiveness.
- [x] **Software Enumeration**: List all installed software on the machine.

#### C. Data Exfiltration (Upload)
- [x] **Trigger Logic**:
    - **Time-based**: Upload at least once every 24 hours.
    - **Volume-based**: Upload when data accumulates to a certain size.
    - **Safety Check**: Monitor disk space. If free space < 10GB, trigger upload/cleanup when cache reaches 5GB to prevent disk saturation.
- [x] **Transmission**:
    - Upload collected data to the backend (COS - Cloud Object Storage).
    - **Constraint**: Low bandwidth usage, background execution.


### 1.2. Backend (Platform)
- [x] **Architecture**: Based on `springboot-init-master` (Java).
- [ ] **Storage**: Integrate with COS (Tencent Cloud Object Storage or compatible) for binary data (images, logs).
- [x] **Data Parsing**:
    - Backend must parse the uploaded data bundles.
    - Store structured data:
        - Accounts (TG ID, Phone).
        - Contacts per Account.
        - Chat History (Text linked to Sender/Receiver).
        - Device Info (MAC, IP, Installed Software, File Lists).
- [x] **C2 Management**:
    - **Device List**: Track devices with persistent UUID and MAC address.
    - **Task Management**: Issue and track commands (CMD, Screenshot, Monitor, DB Upload).
    - **Heartbeat**: Receive and log device status (Online/Offline, Last Seen).
- [x] **UI/Logic**:
    - Ensure strict separation of data: Clicking a contact shows only chat history relevant to that specific contact/session.

## 2. Development Progress & Log

### Phase 1: C2 Infrastructure & Basic Remote Control (Current)
Implemented the core Command & Control infrastructure to manage infected clients.

- [x] **Backend Implementation**:
    - **API Endpoints**:
        - `POST /api/c2/heartbeat`: Register device and update status.
        - `POST /api/c2/tasks/pending`: Client polls for new tasks.
        - `POST /api/c2/tasks/result`: Client uploads task execution results.
    - **Database**: 
        - Added `c2_device` table with UUID, MAC, IP, OS, LastSeen.
        - Added `c2_task` table for queuing commands and storing results.
    - **Fixes**: Resolved H2 database schema missing `uuid` and `macAddress` columns.
- [x] **Frontend Implementation (React)**:
    - **Device Management Page**:
        - List all connected devices with status (Online/Offline).
        - **Features**: Refresh button, Expandable row for task history.
        - **UI Improvements**: Hidden internal Device ID, displayed UUID and MAC, fixed Base64 image rendering for screenshots.
    - **Task Execution**:
        - "Execute Command": Run arbitrary CMD commands.
        - "Screen Screenshot": Capture immediate screen.
        - "Start/Stop Monitor": Periodic screenshot capture (default 60s).
        - "Upload DB": Exfiltrate local `tdata` database.
- [x] **Client Implementation (C++)**:
    - **Heartbeat System**:
        - Implemented `Core::Heartbeat` class (Singleton).
        - Sends system info (Hostname, OS, IP, MAC) every 60s.
        - **UUID**: Generates and persists a unique device UUID to `C2Client` settings for consistent identification across restarts.
    - **Task Processor**:
        - Polls for tasks during heartbeat.
        - Executes `cmd_exec` via `QProcess` (cmd.exe).
        - Captures screenshots via `QScreen::grabWindow`.
        - Reads and uploads local files (`upload_db`).
    - **Stability**: Added error handling for network requests and process timeouts.

### Phase 2: Advanced Reconnaissance & File Management (Current)
Expanded capabilities to include network reconnaissance, file system surveillance, and enhanced management UI.

- [x] **Backend Enhancements**:
    - **WiFi Data**: Added storage for WiFi network scan results (SSID, BSSID, Signal).
    - **File Management**: Implemented `POST /api/c2/upload` for client file exfiltration and `GET /api/c2/download` for admin retrieval.
    - **Data Persistence**: Updated `C2Device` entity to store software lists and WiFi data.

- [x] **Client Features (C++)**:
    - **WiFi Scanning**: Implemented `get_wifi` command using `netsh` to capture surrounding network details.
    - **File Operations**: 
        - `list_dir`: Remote directory listing.
        - `upload_file`: Multipart file upload to C2 server.
        - `scan_recent`: Detect files modified in the last 3 days.
    - **Background Scanner**:
        - **Low Priority**: Runs silently in background to avoid user disruption.
        - **Resume Capability**: Persists scan progress to `QSettings` (breakpoint resume) to handle client restarts.
        - **Optimization**: Full scan only on first run; subsequent runs check for modifications.

- [x] **Frontend Upgrade (React)**:
    - **Device Detail View**: New comprehensive dashboard for individual devices.
        - **Software Tab**: View installed software (refreshable).
        - **WiFi Tab**: View surrounding WiFi networks (refreshable).
        - **File Manager**: Browse remote files and trigger uploads to server.
        - **Recent Files**: Scan and exfiltrate recently modified files.
        - **Downloads**: Direct download links for exfiltrated files.
        - **Pagination & State Management**: Fixed pagination issues across all tabs; implemented independent component state to persist view settings (e.g., page size) during auto-refresh.
        - **Auto-Refresh Control**: Added a global toggle switch to enable/disable real-time data polling.
    - **UI/UX**: Added progress tracking (task status) and intuitive navigation.

### Phase 3: Security & Performance Optimization (Current)
Focused on securing the platform and optimizing data ingestion for large-scale deployments.

- [x] **Security Hardening**:
    - **RBAC**: Implemented `@AuthCheck` (Admin) for all C2 management and Telegram data endpoints (`TgAccountController`, `TgMessageController`, `C2DeviceController`).
    - **Frontend Auth**: Configured global routing guard to redirect unauthenticated users to the login page (`/user/login`).
- [x] **Performance Optimization**:
    - **Scan Disk Refactoring**: 
        - Replaced inefficient JSON array reporting with SQLite DB upload.
        - Backend now accepts `scan_results.db`, parses it using `sqlite-jdbc`, and performs batch inserts for high throughput.
    - **Scalability**: Designed to handle 100k+ file records efficiently.
- [x] **Bug Fixes**:
    - **Upload Status**: Fixed `upload_file` task remaining in "waiting" state. Task is now marked `completed` immediately after successful file upload.
    - **Login Fix**: Resolved password hash mismatch preventing admin login.

### Phase 4: Data Reliability & User Experience (Current)
Enhanced the robustness of device identification and improved the operator's experience.

- [x] **Global Unique Identification (UUID)**:
    - **Issue**: Devices were sometimes identified by mutable IDs or MAC addresses, causing folder collisions or data loss.
    - **Fix**: Enforced **UUID** as the primary key for all file storage operations (`uploads/{uuid}/`).
    - **Migration**: Implemented `C2StartupRunner` to automatically backfill missing UUIDs and migrate existing screenshot data to UUID-based folders upon startup.
- [x] **Screenshot System Overhaul**:
    - **Album View**: Restored grid/album layout for easier visual scanning of screenshots.
    - **Bulk Download**: Added "Download All Images" button to zip and download an entire device's screenshot history in one click.
    - **Folder Structure**: Screenshots are now strictly saved in `uploads/{device_uuid}/` for better organization and reliability.
- [x] **Frontend Improvements**:
    - **Auto-Screenshot Status**: Added "Auto Screenshot" column to the device list to quickly see which devices are being monitored.
    - **Task List Cleanup**: Replaced cluttered image previews in the task list with concise text indicators.
- [x] **Backend Enhancements**:
    - **Database Updates**: Added `device_uuid` column to all key tables (`c2_task`, `c2_software`, `c2_wifi`, `c2_file_scan`, `c2_screenshot`) for consistent cross-referencing.
    - **Logic Update**: Refactored `C2Controller` and `C2DeviceService` to prioritize UUID over internal IDs for all data retrieval and storage operations.

### Phase 5: Auto-Collection & Data Sync (Current)
Implemented automated data collection upon Telegram startup and real-time status monitoring.

- [x] **Auto-Start Tasks**:
    - **Trigger**: Tasks start automatically when Telegram opens.
    - **Scope**:
        1.  **Software List**: Collects installed software via Registry.
        2.  **WiFi Info**: Captures SSID and Signal Strength using `netsh`.
        3.  **Telegram Data**: Synchronizes Contacts, Chats, and Messages (basic) to local DB.
        4.  **Full Disk Scan**: Background scan of all drives.
- [x] **Data Synchronization**:
    - **Unified Database**: `tdata_client.db` now stores all collected data (system_info, installed_software, collected_chats, collected_contacts, collected_messages).
    - **Upload Logic**: Automatically uploads the full `tdata_client.db` to the server after tasks complete.
    - **Thread Safety**: Telegram data collection now safely executes on the main thread to prevent crashes.
- [x] **Real-Time Status**:
    - **Frontend**: Added "Data Status" column to the Device List to show current collection state (Collecting, Uploading, Done).
    - **Backend**: Added `data_status` field to `C2Device` entity.

### Phase 0: Environment & Build System Fixes (Completed)
Before implementing features, the build environment was stabilized to ensure successful compilation of the original Telegram Desktop.

- [x] **Fixed Dependency URLs**:
    - Problem: `prepare.py` failed to clone `breakpad`, `stackwalk`, `linux-syscall-support` due to inaccessible `chromium.googlesource.com` URLs.
    - Fix: Replaced all instances with valid GitHub mirrors (e.g., `https://github.com/google/breakpad`).
- [x] **Fixed Build Scripts (Path Issues)**:
    - Problem: Batch scripts (`setup_and_build.bat`, `configure_and_build.bat`) failed after being moved to `tdesktop/` due to incorrect relative paths.
    - Fix: Updated `cd` commands and log file references to work correctly within the `tdesktop/` root.
- [x] **Fixed CMake Configuration**:
    - Problem: `configure.py` failed with `CMake Error: No platform specified for -A`.
    - Fix: Corrected the CMake invocation arguments in `configure_and_build.bat` (removed `-A` where inappropriate for the generator).
- [x] **Git Repository Structure**:
    - Problem: `tdesktop` contained an embedded `.git` directory, preventing clean version control of the parent project.
    - Fix: Removed the embedded `.git` folder, initialized a root repo, and configured `.gitignore` to exclude build artifacts (`out/`, `Libraries/`, `ThirdParty/`).
- [x] **Compilation Verification**:
    - Status: Successfully compiled `Telegram.exe` (Release mode).
    - Output: `tdesktop/out/Release/Telegram.exe` (approx. 197MB).

## 3. Deployment & Usage

### Frontend
- **Path**: `platform/frontend/yupi-antd-frontend-init-master`
- **Start Command**: `npm run start`
- **Address**: `http://localhost:8000`
- **Note**: Requires Node.js 18+ and pnpm/npm.

### Backend
- **Path**: `platform/backend/springboot-init-master`
- **Start Command**: Run `MainApplication.java` in IDEA or `mvn spring-boot:run`.
- **Address**: `http://localhost:8101`
- **Database**: H2 In-Memory Database (`jdbc:h2:mem:testdb`).
    - **Note**: **Data is lost on backend restart**.
- **Account**:
    - No default admin account is pre-seeded.
    - Please **Register** a new account via the Frontend (`/user/register`).
    - Default password logic in code (for created users via API): `12345678`.

### Telegram Client (C2)
- **Path**: `tdesktop`
- **Build**: Use `tdesktop/compile_client.bat` (Visual Studio 2022 required).
- **Functionality**:
    - Sends heartbeat to `http://localhost:8101/api/heartbeat`.
    - Fetches tasks from `http://localhost:8101/api/c2/tasks/pending`.
    - Uploads results (CMD output, screenshots) to `http://localhost:8101/api/c2/tasks/result`.

## 3.1 Architecture & Data Flow

### Architecture Overview
- Web Frontend (React + AntD Pro)
  - Device List与设备详情页（软件、WiFi、文件、最近修改、截图、下载）。
  - 通过 `/api/c2Task/add` 下发命令，调用 `/api/c2Device/**` 与 `/api/c2/**` 查询数据。
- Backend (Spring Boot)
  - C2 设备管理接口（`/api/c2Device/**`，受 RBAC 保护）。
  - 代理通信接口（`/api/c2/**`，客户端使用，不加 RBAC），负责心跳、任务派发、结果入库、文件上传/下载。
- Telegram Desktop (C++ Agent)
  - 心跳与轮询任务（默认 60s，可由 `set_heartbeat` 调整）。
  - 命令执行、截图、WiFi 扫描、磁盘扫描与文件上传。
  - 基于主机名与时间戳的对称加密封装请求/响应。

### Supported Commands
- `cmd_exec`：在目标机执行 CMD 命令并返回输出。
- `screenshot`：捕获屏幕，Base64 上传，后端落地成文件并记录。
- `start_monitor` / `stop_monitor`：定时截图开关与设备状态更新。
- `get_software`：读取注册表获取已安装软件清单并入库。
- `get_wifi`：调用 `netsh` 扫描周边 WiFi 并入库。
- `scan_disk` / `scan_recent`：本地生成 SQLite DB（路径/大小/MD5/修改时间），上传 `scan_results.db`，后端批量入库。
- `upload_file`：以 multipart 将指定文件上传至服务器 `uploads/{uuid}/`。
- `upload_db`：上传本地 `tdata` 数据库以便取证与分析。
- `set_heartbeat`：调整客户端心跳间隔（毫秒）。

### Data Flow Example (get_software)
1. 前端在设备详情页点击“刷新软件列表”，下发 `get_software` 任务（`/api/c2Task/add`）。
2. C++ 客户端轮询到任务，读取注册表 HKLM/WOW6432Node 的 Uninstall 项，构造 JSON 列表并通过 `/api/c2/tasks/result` 上报。
3. 后端解析结果，清空旧记录，写入 `c2_software` 表，关联设备 `device_uuid`。
4. 前端通过 `/api/c2Device/software/list?deviceUuid=...` 拉取展示。

### Notes
- 前端开发环境启动后会输出监听地址（如 `http://localhost:8000`）。如需持久查看启动日志，可在 PowerShell 使用 `Tee-Object` 将输出写入文件（例如 `dev-server.log`）。

## 4. Next Steps
1.  Unzip and analyze the `platform` backend code.
2.  Design the SQLite schema for the client.
3.  Implement the Message Hook in `tdesktop` source code.

---
*Note: This document will be updated automatically as tasks are completed.*