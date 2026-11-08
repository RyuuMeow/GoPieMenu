#pragma once
#include <QByteArray>
#include <QString>
#include <QUrl>
#include <optional>

namespace gpm {
enum class UpdatePlatform { Windows, MacOS };

// The build's update platform. Tests never rely on the host default; they set
// UpdateTarget explicitly so both platform flows run everywhere.
UpdatePlatform CurrentUpdatePlatform();

// Describes the release asset this build installs. An empty assetName derives
// the name from the platform and release version (Windows Setup.exe, macOS DMG).
struct UpdateTarget {
    UpdatePlatform platform = CurrentUpdatePlatform();
    QString assetName;
};

struct ReleaseInfo {
    QString version;
    QString tag;
    QString notes;
    QUrl pageUrl;
    QUrl installerUrl;
    QUrl checksumUrl;
    QString installerName; // Validated asset file name used to build the URLs.
    qint64 installerSize = 0;
};

// Empty means there is no newer stable version, or the response is invalid.
std::optional<ReleaseInfo> ParseRelease(const QByteArray& json, const QString& currentVersion,
                                       const QString& language, const UpdateTarget& target = {},
                                       QString* error = nullptr);
QString LocalizedReleaseNotes(const QString& body, const QString& language);
// Selects the checksum line for assetName from a manifest that also lists the
// other platform's installer. Empty means the entry is missing or ambiguous.
QByteArray InstallerChecksum(const QByteArray& checksums, const QString& assetName);
}
