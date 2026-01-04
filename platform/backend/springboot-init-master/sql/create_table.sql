
-- C2 Tasks Table
create table if not exists c2_task
(
    id           bigint auto_increment comment 'id' primary key,
    taskId       varchar(64)                        not null comment 'Task ID',
    command      varchar(256)                       not null comment 'Command',
    params       text                               null comment 'Parameters',
    status       varchar(32) default 'pending'      not null comment 'Status',
    result       longtext                           null comment 'Result',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    updateTime   datetime     default CURRENT_TIMESTAMP not null on update CURRENT_TIMESTAMP comment 'Update Time',
    isDelete     tinyint      default 0                 not null comment 'Is Deleted',
    index idx_taskId (taskId)
) comment 'C2 Tasks' collate = utf8mb4_unicode_ci;

-- Collected Data Table (Optional, for storing uploaded DBs or logs)
create table if not exists collected_data
(
    id           bigint auto_increment comment 'id' primary key,
    dataId       varchar(64)                        not null comment 'Data ID',
    dataType     varchar(32)                        not null comment 'Data Type',
    content      longtext                           null comment 'Content (Base64 or Text)',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment 'Create Time',
    isDelete     tinyint      default 0                 not null comment 'Is Deleted'
) comment 'Collected Data' collate = utf8mb4_unicode_ci;
