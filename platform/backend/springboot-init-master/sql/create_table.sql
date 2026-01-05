
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

-- User Table (t_user)
create table if not exists t_user
(
    id           bigint auto_increment comment 'id' primary key,
    userAccount  varchar(256)                           not null comment '账号',
    userPassword varchar(512)                           not null comment '密码',
    unionId      varchar(256)                           null comment '微信开放平台id',
    mpOpenId     varchar(256)                           null comment '公众号openId',
    userName     varchar(256)                           null comment '用户昵称',
    userAvatar   varchar(1024)                          null comment '用户头像',
    userProfile  varchar(512)                           null comment '用户简介',
    userRole     varchar(256) default 'user'            not null comment '用户角色：user/admin/ban',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment '创建时间',
    updateTime   datetime     default CURRENT_TIMESTAMP not null on update CURRENT_TIMESTAMP comment '更新时间',
    isDelete     tinyint      default 0                 not null comment '是否删除',
    index idx_unionId (unionId)
) comment '用户' collate = utf8mb4_unicode_ci;

-- Post Table
create table if not exists post
(
    id           bigint auto_increment comment 'id' primary key,
    title        varchar(512)                           null comment '标题',
    content      text                                   null comment '内容',
    tags         varchar(1024)                          null comment '标签列表（json 数组）',
    thumbNum     int          default 0                 not null comment '点赞数',
    favourNum    int          default 0                 not null comment '收藏数',
    userId       bigint                                 not null comment '创建用户 id',
    createTime   datetime     default CURRENT_TIMESTAMP not null comment '创建时间',
    updateTime   datetime     default CURRENT_TIMESTAMP not null on update CURRENT_TIMESTAMP comment '更新时间',
    isDelete     tinyint      default 0                 not null comment '是否删除',
    index idx_userId (userId)
) comment '帖子' collate = utf8mb4_unicode_ci;
