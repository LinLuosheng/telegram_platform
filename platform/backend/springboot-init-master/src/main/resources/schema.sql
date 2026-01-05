
-- C2 Device
create table if not exists c2_device
(
    id           bigint auto_increment primary key,
    ip           varchar(64)                        null,
    hostName     varchar(128)                       null,
    os           varchar(128)                       null,
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
    deviceId     bigint                             null,
    command      varchar(128)                       not null,
    params       text                               null,
    status       varchar(32)                        default 'pending' not null,
    result       longtext                           null,
    createTime   datetime     default CURRENT_TIMESTAMP not null,
    updateTime   datetime     default CURRENT_TIMESTAMP not null,
    isDelete     tinyint      default 0             not null
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
