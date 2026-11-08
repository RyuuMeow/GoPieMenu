#pragma once
#include "update/UpdateManager.h"
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrl>
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

// The update check now talks to several URLs in a fixed order, so responses are
// keyed by URL instead of by request order. Every request is recorded so tests
// can assert exactly which endpoints were (and were not) contacted.
class Network : public QNetworkAccessManager {
public:
    QHash<QString, Response> responses;
    QList<QUrl> requests;
    void serve(const QUrl& url, const Response& response) { responses.insert(url.toString(), response); }
    void serve(const QUrl& url, const QByteArray& body, int status = 200,
               QNetworkReply::NetworkError error = QNetworkReply::NoError, int delayMs = 0) {
        serve(url, Response{body, status, error, delayMs});
    }
    bool requestedLatestEndpoint() const {
        for (const auto& url : requests)
            if (url.path().contains(QStringLiteral("/releases/latest"))) return true;
        return false;
    }
protected:
    QNetworkReply* createRequest(Operation, const QNetworkRequest& request, QIODevice*) override {
        requests.append(request.url());
        const auto found = responses.constFind(request.url().toString());
        const auto response = found == responses.constEnd()
            ? Response{{}, 500, QNetworkReply::UnknownNetworkError} : found.value();
        return new Reply(request, response, this);
    }
};

inline const QByteArray Installer = "This is test data, not an executable.";
inline const QString Notes = "### English\n<!-- gpm:lang en -->\n#### What's new\n- English update.\n<!-- gpm:endlang -->\n"
    "### 繁體中文\n<!-- gpm:lang zh-TW -->\n#### 新功能\n- 繁體更新內容。\n<!-- gpm:endlang -->\n"
    "### 简体中文\n<!-- gpm:lang zh-CN -->\n#### 新功能\n- 简体更新内容。\n<!-- gpm:endlang -->\n"
    "<!-- gpm:lang shared -->\n### Validation\nShared validation.\n<!-- gpm:endlang -->\n";

// Both platform flows are exercised on every host by overriding the update target.
inline QString WindowsAsset() { return QStringLiteral("GoPieMenu_Setup.exe"); }
inline QString MacAsset(const QString& version = "v2.1.0") {
    return QStringLiteral("GoPieMenu-") + QString(version).mid(1) + QStringLiteral(".dmg");
}
inline gpm::UpdateTarget WindowsTarget() {
    gpm::UpdateTarget target;
    target.platform = gpm::UpdatePlatform::Windows;
    return target;
}
inline gpm::UpdateTarget MacTarget() {
    gpm::UpdateTarget target;
    target.platform = gpm::UpdatePlatform::MacOS;
    return target;
}
inline gpm::UpdateTarget Target(gpm::UpdatePlatform platform, const QString& assetName) {
    gpm::UpdateTarget target;
    target.platform = platform;
    target.assetName = assetName;
    return target;
}

// The release channel and the URLs derived from its selected version.
inline QUrl ChannelUrl() {
    return QUrl(QStringLiteral("https://raw.githubusercontent.com/RyuuMeow/GoPieMenu/release-channel/platform-versions.json"));
}
inline QUrl TagUrl(const QString& version) {
    return QUrl(QStringLiteral("https://api.github.com/repos/RyuuMeow/GoPieMenu/releases/tags/v") + version);
}
inline QUrl DownloadUrl(const QString& version, const QString& asset) {
    return QUrl(QStringLiteral("https://github.com/RyuuMeow/GoPieMenu/releases/download/v") + version + "/" + asset);
}
inline QUrl ChecksumUrl(const QString& version) { return DownloadUrl(version, QStringLiteral("SHA256SUMS.txt")); }
inline QByteArray Channel(const QString& windows = "2.1.0", const QString& macos = "0.0.0") {
    return QJsonDocument(QJsonObject{{"windows", windows}, {"macos", macos}}).toJson();
}
inline void ServeChannel(Network& network, const QString& windows, const QString& macos) {
    network.serve(ChannelUrl(), Channel(windows, macos));
}
inline void ServeTaggedRelease(Network& network, const QString& version, const QByteArray& release) {
    network.serve(TagUrl(version), release);
}

inline QJsonObject Asset(const QString& name, qint64 size = Installer.size(), const QString& state = "uploaded") {
    return QJsonObject{{"name", name}, {"state", state}, {"size", size}};
}
inline QByteArray Release(const QString& version = "v2.1.0", const QString& notes = Notes,
                          const QString& assetName = WindowsAsset()) {
    return QJsonDocument(QJsonObject{{"tag_name", version}, {"draft", false}, {"prerelease", false}, {"body", notes},
        {"html_url", "https://untrusted.invalid/ignore"},
        {"assets", QJsonArray{Asset(assetName),
            QJsonObject{{"name", "SHA256SUMS.txt"}, {"state", "uploaded"}, {"size", 100}}}}}).toJson();
}
inline QByteArray MacRelease(const QString& version = "v2.1.0", const QString& notes = Notes) {
    return Release(version, notes, MacAsset(version));
}
inline QByteArray ReleaseWithAssets(const QString& version, const QJsonArray& assets, const QString& notes = Notes) {
    return QJsonDocument(QJsonObject{{"tag_name", version}, {"draft", false}, {"prerelease", false}, {"body", notes},
        {"html_url", "https://untrusted.invalid/ignore"}, {"assets", assets}}).toJson();
}
inline QByteArray ChecksumFor(const QString& assetName, const QByteArray& bytes = Installer) {
    return QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex() + "  " + assetName.toUtf8() + "\n";
}
inline QByteArray Checksum(const QByteArray& bytes = Installer) { return ChecksumFor(WindowsAsset(), bytes); }
// Serves the merged SHA256SUMS.txt plus the platform installer for one release.
inline void ServePlatformDownload(Network& network, const QString& version, const QString& asset,
                                  const QByteArray& installer = Installer) {
    network.serve(ChecksumUrl(version), ChecksumFor(asset, installer));
    network.serve(DownloadUrl(version, asset), installer);
}
// Seeds a complete Windows update (channel + tagged release + download) for tests
// that exercise the editor's update dialog rather than the update protocol.
inline void ServeStandardUpdate(Network& network, const QByteArray& releaseJson = Release(),
                                const QString& version = "2.1.0") {
    ServeChannel(network, version, "0.0.0");
    ServeTaggedRelease(network, version, releaseJson);
    ServePlatformDownload(network, version, WindowsAsset());
}
inline gpm::UpdateOptions Options(const QString& directory, const QString& language = "en",
                                  const gpm::UpdateTarget& target = WindowsTarget()) {
    gpm::UpdateOptions options;
    options.currentVersion = "2.0.0";
    options.language = language;
    options.settingsFile = directory + "/updates.ini";
    options.downloadDirectory = directory + "/downloads";
    options.target = target;
    options.channelUrl = ChannelUrl();
    options.launchInstaller = [](const QString&, QString*) { return true; };
    return options;
}
}
