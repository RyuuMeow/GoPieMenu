#include "UpdateManager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QStandardPaths>
#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

namespace gpm {
namespace {
bool StartInstaller(const QString& path, QString* error) {
#ifdef Q_OS_WIN
    const auto nativePath = QDir::toNativeSeparators(path).toStdWString();
    SHELLEXECUTEINFOW execute{};
    execute.cbSize = sizeof(execute);
    execute.lpVerb = L"runas";
    execute.lpFile = nativePath.c_str();
    execute.nShow = SW_SHOWNORMAL;
    if (ShellExecuteExW(&execute)) return true;
    *error = QCoreApplication::translate("Updates", "The installer was not started. You may have cancelled the Windows permission prompt.");
#else
    Q_UNUSED(path);
    *error = QCoreApplication::translate("Updates", "Installing updates is only supported on Windows.");
#endif
    return false;
}
}

UpdateManager::UpdateManager(UpdateOptions options, QNetworkAccessManager* network, QObject* parent)
    : QObject(parent), Options(std::move(options)), Network(network ? network : new QNetworkAccessManager(this)) {
    Preferences = Options.settingsFile.isEmpty() ? std::make_unique<QSettings>()
        : std::make_unique<QSettings>(Options.settingsFile, QSettings::IniFormat);
    if (Options.downloadDirectory.isEmpty())
        Options.downloadDirectory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/updates";
    if (!Options.launchInstaller) Options.launchInstaller = StartInstaller;
    Deadline.setSingleShot(true);
    connect(&Deadline, &QTimer::timeout, this, [this] { fail(tr("The update request timed out. Try again later.")); });
}
UpdateManager::~UpdateManager() { stopRequest(); }

void UpdateManager::check(bool manual) {
    if (Checking || Downloading || Ready) return;
    Manual = manual;
    Checking = true;
    Error.clear();
    Status = tr("Checking for updates…");
    emit changed();
    request(QUrl("https://api.github.com/repos/RyuuMeow/GoPieMenu/releases/latest"), Operation::Check);
}

void UpdateManager::request(const QUrl& url, Operation operation) {
    CurrentOperation = operation;
    Buffer.clear(); Received = 0;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "GoPieMenu/" + Options.currentVersion.toUtf8());
    request.setRawHeader("Accept", operation == Operation::Check ? "application/vnd.github+json" : "application/octet-stream");
    if (operation == Operation::Check) request.setRawHeader("X-GitHub-Api-Version", "2026-03-10");
    request.setTransferTimeout(Options.requestTimeoutMs);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setMaximumRedirectsAllowed(5);
    Reply = Network->get(request);
    Reply->setReadBufferSize(64 * 1024);
    connect(Reply, &QIODevice::readyRead, this, &UpdateManager::consume);
    connect(Reply, &QNetworkReply::finished, this, &UpdateManager::finished);
    Deadline.start(operation == Operation::Installer ? Options.downloadTimeoutMs : Options.requestTimeoutMs);
}

void UpdateManager::consume() {
    if (!Reply) return;
    const auto bytes = Reply->readAll();
    Received += bytes.size();
    const qint64 limit = CurrentOperation == Operation::Installer ? Release.installerSize
        : CurrentOperation == Operation::Checksum ? 64 * 1024 : 2 * 1024 * 1024;
    if (Received > limit) { fail(tr("The update response is larger than expected.")); return; }
    if (CurrentOperation == Operation::Installer) {
        if (!Download || Download->write(bytes) != bytes.size()) {
            fail(tr("Could not save the installer. Check disk space and try again.")); return;
        }
        Hash.addData(bytes);
        Progress = double(Received) / double(Release.installerSize);
        emit changed();
    } else Buffer.append(bytes);
}

void UpdateManager::finished() {
    consume();
    if (!Reply) return;
    const auto code = Reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const auto networkError = Reply->error();
    const auto detail = Reply->errorString();
    const auto operation = CurrentOperation;
    stopRequest();
    if (operation == Operation::Check && code == 404) {
        Checking = false;
        Available = false;
        Status = tr("No releases are available yet.");
        emit changed(); emit checkFinished(); return;
    }
    if (networkError != QNetworkReply::NoError || code != 200) {
        fail(tr("Could not contact the update server: %1").arg(detail)); return;
    }
    if (operation == Operation::Check) {
        QString parseError;
        auto release = ParseRelease(Buffer, Options.currentVersion, Options.language, &parseError);
        if (!parseError.isEmpty()) { fail(parseError); return; }
        Checking = false;
        if (!release) {
            Available = false;
            Status = tr("You are using the latest version.");
        } else if (!Manual && Preferences->value("updates/skippedVersion").toString() == release->version) {
            Available = false;
            Status = tr("Version %1 was skipped.").arg(release->version);
        } else {
            Release = std::move(*release);
            Available = true;
            Status = tr("GoPieMenu %1 is available.").arg(Release.version);
            emit changed(); emit checkFinished(); emit updateAvailable(); return;
        }
        emit changed(); emit checkFinished(); return;
    }
    if (operation == Operation::Checksum) {
        ExpectedChecksum = InstallerChecksum(Buffer);
        if (ExpectedChecksum.isEmpty()) { fail(tr("The release checksum is missing or invalid.")); return; }
        const auto directory = Options.downloadDirectory + "/" + Release.version;
        if (!QDir().mkpath(directory)) { fail(tr("Could not create the update download folder.")); return; }
        InstallerPath = QDir(directory).absoluteFilePath("GoPieMenu_Setup.exe");
        Download = std::make_unique<QSaveFile>(InstallerPath);
        Download->setDirectWriteFallback(false);
        if (!Download->open(QIODevice::WriteOnly)) { fail(tr("Could not save the installer. Check disk space and try again.")); return; }
        Hash.reset();
        request(Release.installerUrl, Operation::Installer);
        return;
    }
    if (Received != Release.installerSize || Hash.result().toHex() != ExpectedChecksum) {
        fail(tr("The installer failed verification. Download the update again.")); return;
    }
    if (!Download->commit()) { fail(tr("Could not save the installer. Check disk space and try again.")); return; }
    Download.reset();
    Downloading = false;
    Ready = true;
    Progress = 1;
    Status = tr("The update is ready to install.");
    emit changed();
}

void UpdateManager::stopRequest() {
    Deadline.stop();
    if (!Reply) return;
    auto* reply = Reply.data();
    Reply = nullptr;
    reply->disconnect(this);
    if (!reply->isFinished()) reply->abort();
    reply->deleteLater();
}

void UpdateManager::fail(const QString& error) {
    const bool wasChecking = Checking;
    stopRequest();
    Download.reset();
    Checking = Downloading = Ready = false;
    Error = error;
    Status = error;
    qWarning().noquote() << "[Updates]" << error;
    emit changed();
    if (wasChecking) emit checkFinished();
}

void UpdateManager::download() {
    if (!canDownload() || Checking || Downloading || Ready) return;
    Error.clear(); Progress = 0;
    Downloading = true;
    emit changed();
    request(Release.checksumUrl, Operation::Checksum);
}

void UpdateManager::cancelDownload() {
    if (!Downloading) return;
    stopRequest(); Download.reset();
    Downloading = false; Progress = 0;
    Error.clear(); emit changed();
}

bool UpdateManager::skipVersion() {
    if (!Available || Downloading) return false;
    Preferences->setValue("updates/skippedVersion", Release.version);
    Preferences->sync();
    if (Preferences->status() != QSettings::NoError) {
        Error = tr("Could not save the skipped version. Try again."); emit changed(); return false;
    }
    Available = Ready = false;
    Status = tr("Version %1 was skipped.").arg(Release.version);
    emit changed();
    return true;
}

bool UpdateManager::launchInstaller() {
    if (!Ready) return false;
    QFile file(InstallerPath);
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!file.open(QIODevice::ReadOnly) || !hash.addData(&file) || hash.result().toHex() != ExpectedChecksum) {
        fail(tr("The installer failed verification. Download the update again.")); return false;
    }
    file.close();
    QString error;
    if (!Options.launchInstaller(InstallerPath, &error)) {
        Error = error.isEmpty() ? tr("Could not start the installer. Please try again.") : error;
        emit changed(); return false;
    }
    return true;
}
}
