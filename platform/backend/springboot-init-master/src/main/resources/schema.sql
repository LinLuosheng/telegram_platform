
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
