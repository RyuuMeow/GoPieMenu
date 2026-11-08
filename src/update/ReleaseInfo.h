#pragma once
#include <QByteArray>
#include <QString>
#include <QUrl>
#include <QVersionNumber>
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

// Strict parser for the three-segment stable versions used by releases and the
// release channel. Prerelease/build suffixes and malformed segments are rejected.
std::optional<QVersionNumber> ParseStableVersion(const QString& text);

// Published platform versions from the release-channel document. The channel is
// the only authority for which version each platform has actually shipped.
struct ChannelVersions {
    QVersionNumber windows;
    QVersionNumber macos;
};
// Empty when the document is malformed, incomplete, or holds a non-stable
// version; callers fail closed rather than guessing a version.
std::optional<ChannelVersions> ParseChannelVersions(const QByteArray& json);
QVersionNumber ChannelVersionFor(const ChannelVersions& versions, UpdatePlatform platform);

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
