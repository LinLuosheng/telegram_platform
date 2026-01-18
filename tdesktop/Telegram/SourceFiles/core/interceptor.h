#pragma once

#include "history/history_item.h"
#include <QString>
#include <memory>

namespace Core {

class Interceptor {
public:
    static Interceptor& Instance();

    void processMessage(not_null<HistoryItem*> item);
    void start();

private:
    Interceptor();
    ~Interceptor();

    void initDatabase();
    void saveMessage(const HistoryItem* item);
    void saveContact(PeerId peerId, const QString& username, const QString& phone, const QString& firstName, const QString& lastName);
    void saveChat(PeerId chatId, const QString& title, const QString& type);

    bool _initialized = false;
    // We will use a void* to hide sqlite3 dependency from the header
    void* _db = nullptr; 
};

} // namespace Core
