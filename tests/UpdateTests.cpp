#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include "UpdateTestSupport.h"

using namespace gpm;
using namespace update_test;

class UpdateTests : public QObject {
    Q_OBJECT
private slots:
    void comparesVersionsNumericallyAndRejectsPrereleases() {
        QString error;
        QVERIFY(ParseRelease(Release("v2.0.10"), "2.0.9", "en", &error));
        QVERIFY(error.isEmpty());
        QVERIFY(!ParseRelease(Release("v2.0.9"), "2.0.10", "en"));
        QVERIFY(!ParseRelease(Release("v2.0.0"), "2.0.0", "en"));
        QVERIFY(!ParseRelease(Release("v1.9.9"), "2.0.0", "en"));
        QVERIFY(!ParseRelease(Release("v3.0.0-beta.1"), "2.0.0", "en"));
        QVERIFY(!ParseRelease(Release("v2.1.0/../../file"), "2.0.0", "en"));
        QVERIFY(!ParseRelease("not JSON", "2.0.0", "en", &error)); QVERIFY(!error.isEmpty());
        auto json = QJsonDocument::fromJson(Release()).object();
        json["draft"] = true; QVERIFY(!ParseRelease(QJsonDocument(json).toJson(), "2.0.0", "en"));
        json["draft"] = false; json["prerelease"] = true;
        QVERIFY(!ParseRelease(QJsonDocument(json).toJson(), "2.0.0", "en"));
        const auto release = ParseRelease(Release(), "2.0.0", "en"); QVERIFY(release);
        QCOMPARE(release->pageUrl, QUrl("https://github.com/RyuuMeow/GoPieMenu/releases/tag/v2.1.0"));
        QCOMPARE(release->installerUrl, QUrl("https://github.com/RyuuMeow/GoPieMenu/releases/download/v2.1.0/GoPieMenu_Setup.exe"));
    }
    void selectsLocalizedNotesAndSharedValidation() {
        const auto traditional = LocalizedReleaseNotes(Notes, "zh-HK");
        QVERIFY(traditional.contains("繁體更新內容"));
        QVERIFY(traditional.contains("Shared validation"));
        QVERIFY(!traditional.contains("English update"));
        QVERIFY(!traditional.contains("简体更新内容"));
        QVERIFY(!traditional.contains("gpm:lang"));
        QVERIFY(LocalizedReleaseNotes(Notes, "zh-SG").contains("简体更新内容"));
        QVERIFY(LocalizedReleaseNotes(Notes, "fr-FR").contains("English update"));
        const QString english = "<!-- gpm:lang en -->\nFallback\n<!-- gpm:endlang -->";
        QCOMPARE(LocalizedReleaseNotes(english, "zh-TW"), "Fallback");
        QCOMPARE(LocalizedReleaseNotes("Legacy English notes", "zh-TW"), "Legacy English notes");
        QVERIFY(LocalizedReleaseNotes("<!-- gpm:lang en -->\nBroken", "en").isEmpty());
        QVERIFY(LocalizedReleaseNotes(english + "\n" + english, "en").isEmpty());
        auto crlf = Notes; crlf.replace("\n", "\r\n");
        QCOMPARE(LocalizedReleaseNotes(crlf, "zh-TW").replace("\r\n", "\n"), traditional);
    }
    void extractsRealChangelogLanguageBlocks() {
        QFile file(QStringLiteral(GPM_SOURCE_DIR) + "/CHANGELOG.md"); QVERIFY(file.open(QIODevice::ReadOnly));
        const auto text = QString::fromUtf8(file.readAll());
        const auto current = text.section("## [Unreleased]", 1, 1).section("## [2.0.0]", 0, 0);
        for (const auto& language : {"en", "zh-TW", "zh-CN"}) {
            const auto notes = LocalizedReleaseNotes(current, language);
            QVERIFY(!notes.isEmpty()); QVERIFY(notes.contains("Validation")); QVERIFY(!notes.contains("gpm:lang"));
        }
    }
    void parsesOnlyTheInstallerChecksum() {
        QCOMPARE(InstallerChecksum(Checksum()), QCryptographicHash::hash(Installer, QCryptographicHash::Sha256).toHex());
        QCOMPARE(InstallerChecksum(Checksum().toUpper().replace("GOPIEMENU_SETUP.EXE", "GoPieMenu_Setup.exe")), InstallerChecksum(Checksum()));
        QVERIFY(InstallerChecksum("bad  GoPieMenu_Setup.exe").isEmpty());
        QVERIFY(InstallerChecksum(Checksum() + Checksum()).isEmpty());
        QVERIFY(InstallerChecksum(Checksum().replace("GoPieMenu_Setup.exe", "../GoPieMenu_Setup.exe")).isEmpty());
    }
    void checksOncePerRequestAndRemembersSkippedVersion() {
        QTemporaryDir dir; Network network; network.responses = {{Release()}, {Release()}, {Release("v2.2.0")}};
        auto options = Options(dir.path());
        UpdateManager first(options, &network);
        QSignalSpy offered(&first, &UpdateManager::updateAvailable);
        first.check(); first.check();
        QTRY_COMPARE(offered.count(), 1); QCOMPARE(network.requests.size(), 1);
        QVERIFY(first.available()); QVERIFY(first.skipVersion()); QVERIFY(!first.available());
        UpdateManager second(options, &network);
        QSignalSpy next(&second, &UpdateManager::updateAvailable);
        second.check(); QTRY_VERIFY(!second.checking()); QCOMPARE(next.count(), 0);
        second.check(); QTRY_COMPARE(next.count(), 1); QCOMPARE(second.version(), "2.2.0");
    }
    void manualCheckCanRevisitASkippedRelease() {
        QTemporaryDir dir; Network network; network.responses = {{Release()}, {Release()}};
        UpdateManager manager(Options(dir.path()), &network);
        manager.check(); QTRY_VERIFY(manager.available()); QVERIFY(manager.skipVersion());
        manager.check(true); QTRY_VERIFY(manager.available());
    }
    void downloadsVerifiesThenRequiresExplicitInstall() {
        QTemporaryDir dir; Network network; network.responses = {{Release()}, {Checksum()}, {Installer}};
        auto options = Options(dir.path(), "zh-TW"); int launches = 0;
        options.launchInstaller = [&](const QString& path, QString*) { ++launches; return QFile::exists(path); };
        UpdateManager manager(options, &network);
        manager.check(); QTRY_VERIFY(manager.available()); QVERIFY(manager.notes().contains("繁體"));
        QVERIFY(!manager.launchInstaller()); QCOMPARE(launches, 0);
        manager.download(); QTRY_VERIFY(manager.ready()); QCOMPARE(manager.progress(), 1.0);
        QVERIFY(!manager.downloading()); QCOMPARE(launches, 0);
        QFile file(manager.installerPath()); QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), Installer); file.close();
        QVERIFY(manager.launchInstaller()); QCOMPARE(launches, 1);
        QCOMPARE(network.requests.size(), 3);
        QCOMPARE(network.requests[0].url().host(), "api.github.com");
        QCOMPARE(network.requests[1].url().fileName(), "SHA256SUMS.txt");
        QCOMPARE(network.requests[2].url().fileName(), "GoPieMenu_Setup.exe");
    }
    void rejectsCorruptedDownloadAndAllowsRetry() {
        QTemporaryDir dir; Network network; network.responses = {{Release()}, {Checksum("wrong")}, {Installer}, {Checksum()}, {Installer}};
        UpdateManager manager(Options(dir.path()), &network);
        manager.check(); QTRY_VERIFY(manager.available()); manager.download();
        QTRY_VERIFY(!manager.downloading()); QVERIFY(!manager.ready()); QVERIFY(!manager.error().isEmpty());
        QVERIFY(!QFile::exists(manager.installerPath())); QVERIFY(!manager.launchInstaller());
        manager.download(); QTRY_VERIFY(manager.ready()); QVERIFY(manager.error().isEmpty());
    }
    void rechecksTheFileBeforeLaunchingAndKeepsAppOnUacCancel() {
        QTemporaryDir dir; Network network; network.responses = {{Release()}, {Checksum()}, {Installer}};
        auto options = Options(dir.path()); int launches = 0;
        options.launchInstaller = [&](const QString&, QString* error) { ++launches; *error = "UAC cancelled"; return false; };
        UpdateManager manager(options, &network);
        manager.check(); QTRY_VERIFY(manager.available()); manager.download(); QTRY_VERIFY(manager.ready());
        QVERIFY(!manager.launchInstaller()); QVERIFY(manager.ready()); QCOMPARE(launches, 1);
        QFile file(manager.installerPath()); QVERIFY(file.open(QIODevice::WriteOnly)); file.write("changed"); file.close();
        QVERIFY(!manager.launchInstaller()); QVERIFY(!manager.ready()); QCOMPARE(launches, 1);
    }
    void handlesNetworkFailuresAndTimeoutsWithoutOfferingAnUpdate() {
        QTemporaryDir dir; Network network;
        network.responses = {{{}, 404, QNetworkReply::ContentNotFoundError}, {{}, 403, QNetworkReply::ContentAccessDenied},
            {"invalid JSON"}, {{}, 200, QNetworkReply::NoError, -1}};
        auto options = Options(dir.path()); options.requestTimeoutMs = 30;
        UpdateManager manager(options, &network); QSignalSpy offered(&manager, &UpdateManager::updateAvailable);
        for (int attempt = 0; attempt < 4; ++attempt) {
            manager.check(); QTRY_VERIFY(!manager.checking()); QVERIFY(!manager.available());
            if (attempt == 0) QVERIFY(manager.error().isEmpty()); else QVERIFY(!manager.error().isEmpty());
        }
        QCOMPARE(offered.count(), 0);
    }
    void cancelsDownloadsAndRejectsMissingOrOversizedFiles() {
        QTemporaryDir dir; Network network; network.responses = {{Release()}, {Checksum(), 200, QNetworkReply::NoError, -1},
            {"invalid manifest"}, {Checksum()}, {Installer + "extra"}};
        UpdateManager manager(Options(dir.path()), &network);
        manager.check(); QTRY_VERIFY(manager.available()); manager.download(); QVERIFY(manager.downloading());
        manager.cancelDownload(); QVERIFY(!manager.downloading()); QVERIFY(!manager.ready());
        manager.download(); QTRY_VERIFY(!manager.downloading()); QVERIFY(!manager.ready());
        manager.download(); QTRY_VERIFY(!manager.downloading()); QVERIFY(!manager.ready());
        QVERIFY(!QFile::exists(manager.installerPath()));
    }
    void missingWindowsAssetsStillShowsNotes() {
        auto json = QJsonDocument::fromJson(Release()).object(); json["assets"] = QJsonArray();
        QTemporaryDir dir; Network network; network.responses = {{QJsonDocument(json).toJson()}};
        UpdateManager manager(Options(dir.path()), &network);
        manager.check(); QTRY_VERIFY(manager.available()); QVERIFY(!manager.canDownload());
        manager.download(); QCOMPARE(network.requests.size(), 1); QVERIFY(!manager.downloading());
    }
};
QTEST_GUILESS_MAIN(UpdateTests)
#include "UpdateTests.moc"
