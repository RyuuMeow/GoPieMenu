#pragma once
#include "update/UpdateManager.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <cstring>

namespace update_test {
struct Response {
    QByteArray body;
    int status = 200;
    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    int delayMs = 0; // Negative simulates a server that never responds.
};

class Reply : public QNetworkReply {
public:
    Reply(const QNetworkRequest& request, Response response, QObject* parent) : QNetworkReply(parent), Data(response.body) {
        setRequest(request); setUrl(request.url());
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, response.status);
        open(QIODevice::ReadOnly | QIODevice::Unbuffered);
        if (response.delayMs < 0) return;
        QTimer::singleShot(response.delayMs, this, [this, response] {
            if (isFinished()) return;
            if (response.error != NoError) setError(response.error, "Simulated network failure");
            emit readyRead();
            if (isFinished()) return;
            setFinished(true); emit finished();
        });
    }
    void abort() override {
        if (isFinished()) return;
        setError(OperationCanceledError, "Cancelled"); setFinished(true); emit finished();
    }
    qint64 bytesAvailable() const override { return Data.size() - Offset + QNetworkReply::bytesAvailable(); }
protected:
    qint64 readData(char* destination, qint64 maxSize) override {
        const auto count = std::min(maxSize, qint64(Data.size()) - Offset);
        if (count <= 0) return -1;
        std::memcpy(destination, Data.constData() + Offset, size_t(count)); Offset += count; return count;
    }
private:
    QByteArray Data;
    qint64 Offset = 0;
};

class Network : public QNetworkAccessManager {
public:
    QList<Response> responses;
    QList<QNetworkRequest> requests;
protected:
    QNetworkReply* createRequest(Operation, const QNetworkRequest& request, QIODevice*) override {
        requests.append(request);
        const auto response = responses.isEmpty()
            ? Response{{}, 500, QNetworkReply::UnknownNetworkError} : responses.takeFirst();
        return new Reply(request, response, this);
    }
};

inline const QByteArray Installer = "This is test data, not an executable.";
inline const QString Notes = "### English\n<!-- gpm:lang en -->\n#### What's new\n- English update.\n<!-- gpm:endlang -->\n"
    "### 繁體中文\n<!-- gpm:lang zh-TW -->\n#### 新功能\n- 繁體更新內容。\n<!-- gpm:endlang -->\n"
    "### 简体中文\n<!-- gpm:lang zh-CN -->\n#### 新功能\n- 简体更新内容。\n<!-- gpm:endlang -->\n"
    "<!-- gpm:lang shared -->\n### Validation\nShared validation.\n<!-- gpm:endlang -->\n";
inline QByteArray Release(const QString& version = "v2.1.0", const QString& notes = Notes) {
    return QJsonDocument(QJsonObject{{"tag_name", version}, {"draft", false}, {"prerelease", false}, {"body", notes},
        {"html_url", "https://untrusted.invalid/ignore"},
        {"assets", QJsonArray{
            QJsonObject{{"name", "GoPieMenu_Setup.exe"}, {"state", "uploaded"}, {"size", Installer.size()}},
            QJsonObject{{"name", "SHA256SUMS.txt"}, {"state", "uploaded"}, {"size", 100}}}}}).toJson();
}
inline QByteArray Checksum(const QByteArray& bytes = Installer) {
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex() + "  GoPieMenu_Setup.exe\n";
}
inline gpm::UpdateOptions Options(const QString& directory, const QString& language = "en") {
    gpm::UpdateOptions options;
    options.currentVersion = "2.0.0";
    options.language = language;
    options.settingsFile = directory + "/updates.ini";
    options.downloadDirectory = directory + "/downloads";
    options.launchInstaller = [](const QString&, QString*) { return true; };
    return options;
}
}
