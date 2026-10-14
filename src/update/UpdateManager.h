#pragma once
#include "ReleaseInfo.h"
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QSaveFile>
#include <QSettings>
#include <QTimer>
#include <functional>
#include <memory>

namespace gpm {
struct UpdateOptions {
    QString currentVersion = QStringLiteral(APP_VERSION);
    QString language = QStringLiteral("system");
    QString settingsFile; // Empty uses the application's normal QSettings store.
    QString downloadDirectory; // Empty uses the application's cache directory.
    int requestTimeoutMs = 15000;
    int downloadTimeoutMs = 10 * 60 * 1000;
    // Tests inject a launcher; production uses the Windows installer shell verb.
    std::function<bool(const QString&, QString*)> launchInstaller;
};

class UpdateManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool checking READ checking NOTIFY changed)
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(bool canDownload READ canDownload NOTIFY changed)
    Q_PROPERTY(bool downloading READ downloading NOTIFY changed)
    Q_PROPERTY(bool ready READ ready NOTIFY changed)
    Q_PROPERTY(double progress READ progress NOTIFY changed)
    Q_PROPERTY(QString version READ version NOTIFY changed)
    Q_PROPERTY(QString notes READ notes NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY changed)
public:
    explicit UpdateManager(UpdateOptions options = {}, QNetworkAccessManager* network = nullptr, QObject* parent = nullptr);
    ~UpdateManager() override;
    bool checking() const { return Checking; }
    bool available() const { return Available; }
    bool canDownload() const { return Available && !Release.installerUrl.isEmpty(); }
    bool downloading() const { return Downloading; }
    bool ready() const { return Ready; }
    double progress() const { return Progress; }
    QString version() const { return Release.version; }
    QString notes() const { return Release.notes; }
    QString status() const { return Status; }
    QString error() const { return Error; }
    QString installerPath() const { return InstallerPath; }
    QUrl releasePage() const { return Release.pageUrl; }
    Q_INVOKABLE void check(bool manual = false);
    Q_INVOKABLE void download();
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE bool skipVersion();
    bool launchInstaller(); // Only called after the editor resolves unsaved changes.
signals:
    void changed();
    void updateAvailable();
    void checkFinished();
private:
    enum class Operation { Check, Checksum, Installer };
    void request(const QUrl& url, Operation operation);
    void consume();
    void finished();
    void fail(const QString& error);
    void stopRequest();
    UpdateOptions Options;
    QNetworkAccessManager* Network;
    std::unique_ptr<QSettings> Preferences;
    QPointer<QNetworkReply> Reply;
    QTimer Deadline;
    Operation CurrentOperation = Operation::Check;
    QByteArray Buffer, ExpectedChecksum;
    QCryptographicHash Hash{QCryptographicHash::Sha256};
    std::unique_ptr<QSaveFile> Download;
    ReleaseInfo Release;
    QString InstallerPath, Status, Error;
    qint64 Received = 0;
    double Progress = 0;
    bool Checking = false, Available = false, Downloading = false, Ready = false, Manual = false;
};
}
