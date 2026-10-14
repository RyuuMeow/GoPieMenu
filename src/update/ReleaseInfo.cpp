#include "ReleaseInfo.h"
#include "core/TranslationManager.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QRegularExpression>
#include <QVersionNumber>

namespace gpm {
namespace {
std::optional<QVersionNumber> StableVersion(const QString& text) {
    static const QRegularExpression pattern(R"(^v?((?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*))$)");
    const auto match = pattern.match(text);
    if (!match.hasMatch()) return {};
    qsizetype suffix = 0;
    const auto version = QVersionNumber::fromString(match.captured(1), &suffix);
    if (suffix != match.capturedLength(1) || version.segmentCount() != 3) return {};
    return version;
}
}

QString LocalizedReleaseNotes(const QString& body, const QString& language) {
    // Older releases had an English-only body without language markers.
    if (!body.contains("<!-- gpm:lang")) return body.trimmed();
    static const QRegularExpression blocks(
        R"(^[ \t]*<!-- gpm:lang (en|zh-TW|zh-CN|shared) -->[ \t]*\r?\n(.*?)^[ \t]*<!-- gpm:endlang -->[ \t]*\r?$)",
        QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    QMap<QString, QString> sections;
    auto matches = blocks.globalMatch(body);
    while (matches.hasNext()) {
        const auto match = matches.next();
        const auto content = match.captured(2).trimmed();
        // Don't display another language if a marker was accidentally left unclosed.
        if (content.contains("<!-- gpm:lang") || sections.contains(match.captured(1))) return {};
        sections.insert(match.captured(1), content);
    }
    const auto locale = ResolveLanguage(language).replace('_', '-');
    auto notes = sections.value(locale);
    if (notes.isEmpty()) notes = sections.value("en");
    if (notes.isEmpty()) return {};
    if (const auto shared = sections.value("shared"); !shared.isEmpty()) notes += "\n\n" + shared;
    return notes;
}

std::optional<ReleaseInfo> ParseRelease(const QByteArray& json, const QString& currentVersion,
                                       const QString& language, QString* error) {
    if (error) error->clear();
    const auto document = QJsonDocument::fromJson(json);
    const auto current = StableVersion(currentVersion);
    const auto object = document.object();
    const auto tag = object.value("tag_name").toString();
    const auto version = StableVersion(tag);
    if (!document.isObject() || !object.value("draft").isBool() || !object.value("prerelease").isBool()) {
        if (error) *error = QCoreApplication::translate("Updates", "The release response is invalid.");
        return {};
    }
    if (object.value("draft").toBool() || object.value("prerelease").toBool()) return {};
    if (!current || !version) {
        if (error) *error = QCoreApplication::translate("Updates", "The release version is invalid.");
        return {};
    }
    if (QVersionNumber::compare(*version, *current) <= 0) return {};
    ReleaseInfo result;
    result.tag = tag;
    result.version = version->toString();
    result.notes = LocalizedReleaseNotes(object.value("body").toString(), language);
    // Derive URLs from our repository and validated tag, not arbitrary API fields.
    const QString repo = "https://github.com/RyuuMeow/GoPieMenu/releases/";
    result.pageUrl = QUrl(repo + "tag/" + tag);
    bool installer = false, checksum = false;
    for (const auto& value : object.value("assets").toArray()) {
        const auto asset = value.toObject();
        if (asset.value("state").toString() != "uploaded") continue;
        const auto size = asset.value("size").toInteger();
        if (asset.value("name").toString() == "GoPieMenu_Setup.exe" && size > 0 && size <= 512 * 1024 * 1024) {
            installer = true;
            result.installerSize = size;
        }
        if (asset.value("name").toString() == "SHA256SUMS.txt" && size > 0 && size <= 64 * 1024) checksum = true;
    }
    if (installer && checksum) {
        result.installerUrl = QUrl(repo + "download/" + tag + "/GoPieMenu_Setup.exe");
        result.checksumUrl = QUrl(repo + "download/" + tag + "/SHA256SUMS.txt");
    }
    return result;
}

QByteArray InstallerChecksum(const QByteArray& checksums) {
    static const QRegularExpression line(R"(^([a-fA-F0-9]{64})[ \t]+\*?GoPieMenu_Setup\.exe[ \t]*$)");
    QByteArray checksum;
    for (const auto& raw : checksums.split('\n')) {
        const auto match = line.match(QString::fromUtf8(raw).trimmed());
        if (!match.hasMatch()) continue;
        if (!checksum.isEmpty()) return {}; // Ambiguous manifest.
        checksum = match.captured(1).toLatin1().toLower();
    }
    return checksum;
}
}
