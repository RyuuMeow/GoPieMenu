#pragma once
#include <QByteArray>
#include <QString>
#include <QUrl>
#include <optional>

namespace gpm {
struct ReleaseInfo {
    QString version;
    QString tag;
    QString notes;
    QUrl pageUrl;
    QUrl installerUrl;
    QUrl checksumUrl;
    qint64 installerSize = 0;
};

// Empty means there is no newer stable version, or the response is invalid.
std::optional<ReleaseInfo> ParseRelease(const QByteArray& json, const QString& currentVersion,
                                       const QString& language, QString* error = nullptr);
QString LocalizedReleaseNotes(const QString& body, const QString& language);
QByteArray InstallerChecksum(const QByteArray& checksums);
}
