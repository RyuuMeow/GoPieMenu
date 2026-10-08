#include <QtTest>
#include <QTemporaryDir>
#include <QFile>
#include "core/EditorSession.h"
#include "core/ConfigValidation.h"
#include "core/InputState.h"
#include "core/Shortcut.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif

using namespace gpm;

class CoreTests : public QObject {
    Q_OBJECT
private slots:
    void recordedAndLegacyShortcutParsing() {
        QCOMPARE(ParseShortcut("Ctrl+Shift+K"), (std::vector<int>{0xa2, 0xa0, 'K'}));
        QCOMPARE(ParseShortcut("Win+F24"), (std::vector<int>{0x5b, 0x87}));
        QCOMPARE(ParseShortcut("Ctrl+PgDown"), (std::vector<int>{0xa2, 0x22}));
        QVERIFY(ParseShortcut("Ctrl+UnknownKey").empty());
        QVERIFY(ParseShortcut("Ctrl").empty());
        QVERIFY(ParseShortcut("Ctrl+A+B").empty());
        QVERIFY(!ParseShortcut("Ctrl++").empty());
    }
    void inputHoldRepeatAndMatchingReleases() {
        auto p = PieMenuConfig::CreateDefault();
        p.Trigger = {ActivationMode::KeyHold, ModifierKey::Ctrl, MouseButton::None, 'M'};
        InputState state; state.setProfiles({p});
        state.key(0xa2, true); state.key(0xa3, true); state.key(0xa2, false);
        QCOMPARE(state.modifiers(), ModifierKey::Ctrl);
        QVERIFY(!state.key('W', true).Swallow); // Already-held keys must receive their release.
        QCOMPARE(state.key('M', true).Effect, InputEffect::Show);
        QVERIFY(!state.key('W', false).Swallow);
        const auto repeat = state.key('M', true);
        QVERIFY(repeat.Swallow); QCOMPARE(repeat.Effect, InputEffect::None);
        QVERIFY(state.key('X', true).Swallow);
        QCOMPARE(state.key('M', false).Effect, InputEffect::Confirm);
        QVERIFY(state.key('X', false).Swallow);
        QCOMPARE(state.key('M', false).Effect, InputEffect::None);
        QVERIFY(!state.active());
    }
    void inputToggleEscapeAndInjectedEvents() {
        auto p = PieMenuConfig::CreateDefault();
        p.Trigger = {ActivationMode::KeyToggle, ModifierKey::None, MouseButton::None, 'M'};
        InputState state; state.setProfiles({p});
        QCOMPARE(state.key('M', true, {}, true).Effect, InputEffect::None);
        QCOMPARE(state.key('M', true).Effect, InputEffect::Show);
        QVERIFY(state.key('M', false).Swallow);
        QCOMPARE(state.mouse(MouseButton::Left, true).Effect, InputEffect::None);
        QCOMPARE(state.mouse(MouseButton::Left, false).Effect, InputEffect::Confirm);
        QCOMPARE(state.mouse(MouseButton::Left, false).Effect, InputEffect::None);
        QCOMPARE(state.key('M', true).Effect, InputEffect::Show);
        QCOMPARE(state.key(0x1b, true).Effect, InputEffect::Cancel);
        QVERIFY(state.key(0x1b, false).Swallow);
        QVERIFY(state.key('M', false).Swallow);
        QCOMPARE(state.key('M', true).Effect, InputEffect::Show);
        state.key('M', false);
        QCOMPARE(state.key('M', true).Effect, InputEffect::Cancel);
    }
    void inputMouseScopeAndFailClosed() {
        auto p = PieMenuConfig::CreateDefault(); p.AppFilter = {"paint.exe"};
        InputState state; state.setProfiles({p}); state.synchronizeKey(0xa2, true);
        QCOMPARE(state.mouse(MouseButton::Right, true, {}).Effect, InputEffect::None);
        QCOMPARE(state.mouse(MouseButton::Right, true, "PAINT.EXE").Effect, InputEffect::Show);
        QCOMPARE(state.wheel(-120).Effect, InputEffect::Scroll);
        state.key(0xa2, false); // Releasing modifiers first must still complete the gesture.
        QCOMPARE(state.mouse(MouseButton::Right, false).Effect, InputEffect::Confirm);
        auto global = p; global.Id = "global"; global.AppFilter.clear();
        state.setProfiles({global,p}); state.synchronizeKey(0xa2, true);
        QCOMPARE(state.mouse(MouseButton::Right, true, "paint.exe").ProfileId, p.Id);
        state.cancel(); QVERIFY(state.mouse(MouseButton::Right, false).Swallow);
        QCOMPARE(state.mouse(MouseButton::Right, true, "other.exe").ProfileId, global.Id);
    }
    void malformedSavedFileIsNotReplaced() {
        QTemporaryDir dir;
        QFile file(dir.filePath("config.json")); QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{broken original"); file.close();
        { ConfigManager manager(nullptr, file.fileName()); QVERIFY(!manager.LoadError().isEmpty()); }
        QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), QByteArray("{broken original"));
    }
    void draftSurvivesSelectionAndNeverTouchesRuntime() {
        QTemporaryDir dir;
        ConfigManager manager(nullptr, dir.filePath("config.json"));
        EditorSession s(&manager);
        const auto original = manager.GetConfig().Serialize();
        const auto first = s.profileId();
        s.selectItem(s.items()[0].toMap()["id"].toString());
        s.setItemField("name", "Edited action");
        s.setProfileField("name", "Edited menu");
        s.addProfile();
        const auto second = s.profileId();
        s.setProfileField("name", "Second menu");
        s.selectProfile(first);
        QCOMPARE(s.profile()["name"].toString(), QString("Edited menu"));
        QCOMPARE(s.items()[0].toMap()["name"].toString(), QString("Edited action"));
        s.selectProfile(second);
        QCOMPARE(s.profile()["name"].toString(), QString("Second menu"));
        QCOMPARE(manager.GetConfig().Serialize(), original);
        QVERIFY(!QFile::exists(manager.GetConfigFilePath()));
        QVERIFY(s.dirty());
    }
    void applyDiscardUndoRedo() {
        QTemporaryDir dir;
        ConfigManager manager(nullptr, dir.filePath("config.json"));
        EditorSession s(&manager);
        const auto original = s.draft().Serialize();
        s.addProfile(); s.duplicateProfile(); s.removeProfile();
        s.discard();
        QCOMPARE(s.draft().Serialize(), original);
        QVERIFY(!s.dirty());
        s.setProfileField("name", "Renamed");
        s.undo();
        QCOMPARE(s.draft().Serialize(), original);
        s.redo();
        QCOMPARE(s.profile()["name"].toString(), QString("Renamed"));
        QVERIFY(s.apply());
        QVERIFY(!s.dirty());
        ConfigManager reopened(nullptr, manager.GetConfigFilePath());
        QCOMPARE(reopened.GetConfig().Profiles[0].Name, QString("Renamed"));
        s.undo();
        QVERIFY(s.dirty());
        s.discard();
        QCOMPARE(s.profile()["name"].toString(), QString("Renamed"));
    }
    void invalidImportPreservesEverything() {
        QTemporaryDir dir;
        ConfigManager manager(nullptr, dir.filePath("config.json"));
        EditorSession s(&manager);
        QVERIFY(s.apply());
        QFile file(manager.GetConfigFilePath()); QVERIFY(file.open(QIODevice::ReadOnly));
        const auto disk = file.readAll(); file.close();
        s.setProfileField("name", "Unsaved draft");
        const auto before = s.draft().Serialize();
        QVERIFY(!s.importData("{broken"));
        QCOMPARE(s.draft().Serialize(), before);
        QVERIFY(s.dirty());
        QVERIFY(!s.error().isEmpty());
        QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), disk);
    }
    void importIsAnUndoableDraft() {
        QTemporaryDir dir;
        ConfigManager manager(nullptr, dir.filePath("config.json"));
        EditorSession s(&manager);
        auto config = AppConfig::CreateDefault(); config.Profiles[0].Name = "Imported";
        QVERIFY(s.importData(config.Serialize()));
        QCOMPARE(s.profile()["name"].toString(), QString("Imported"));
        QVERIFY(s.dirty());
        QVERIFY(!QFile::exists(manager.GetConfigFilePath()));
        s.undo();
        QCOMPARE(s.draft().Serialize(), manager.GetConfig().Serialize());
    }
    void saveFailureKeepsDraftAndRuntime() {
        QTemporaryDir dir;
        QFile blocker(dir.filePath("blocked")); QVERIFY(blocker.open(QIODevice::WriteOnly)); blocker.write("keep"); blocker.close();
        ConfigManager manager(nullptr, dir.filePath("blocked/config.json"));
        EditorSession s(&manager);
        const auto original = manager.GetConfig().Serialize();
        s.setProfileField("name", "Do not lose this");
        QVERIFY(!s.apply());
        QVERIFY(s.dirty());
        QVERIFY(!s.error().isEmpty());
        QCOMPARE(manager.GetConfig().Serialize(), original);
        QCOMPARE(s.profile()["name"].toString(), QString("Do not lose this"));
    }
    void atomicReplacementFailurePreservesExistingFile() {
#ifdef Q_OS_WIN
        QTemporaryDir dir; ConfigManager manager(nullptr, dir.filePath("config.json"));
        EditorSession s(&manager); QVERIFY(s.apply());
        const auto original = manager.GetConfig().Serialize();
        const auto path = QDir::toNativeSeparators(manager.GetConfigFilePath());
        HANDLE locked = CreateFileW(reinterpret_cast<LPCWSTR>(path.utf16()), GENERIC_READ, FILE_SHARE_READ,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        QVERIFY(locked != INVALID_HANDLE_VALUE);
        s.setProfileField("name", "Keep this draft");
        const bool saved = s.apply();
        CloseHandle(locked);
        QVERIFY(!saved); QVERIFY(s.dirty()); QVERIFY(!s.error().isEmpty());
        QCOMPARE(manager.GetConfig().Serialize(), original);
        QFile file(path); QVERIFY(file.open(QIODevice::ReadOnly)); QCOMPARE(file.readAll(), original);
#endif
    }
    void deletingSelectedItemAndSwitchingProfilesIsSafe() {
        QTemporaryDir dir;
        ConfigManager manager(nullptr, dir.filePath("config.json"));
        EditorSession s(&manager);
        for (int i = 0; i < 25; ++i) {
            s.selectItem(s.items()[0].toMap()["id"].toString());
            s.addProfile();
            s.setItemField("name", "Nothing selected");
            s.removeProfile();
            s.selectItem(s.items()[0].toMap()["id"].toString());
        }
        const auto id = s.selectedId();
        s.removeItem(id);
        QVERIFY(s.selectedId().isEmpty());
        QVERIFY(!s.findItem(id));
        s.undo();
        QVERIFY(s.findItem(id));
    }
    void movingSubmenusCannotDropChildren() {
        QTemporaryDir dir;
        ConfigManager manager(nullptr, dir.filePath("config.json"));
        EditorSession s(&manager);
        const auto folder = s.addItem(int(ActionType::ListMenu));
        s.enterFolder(folder);
        const auto child = s.addItem(int(ActionType::LaunchApp));
        s.setItemField("target", "notepad.exe");
        s.leaveFolder();
        const auto other = s.addItem(int(ActionType::ListMenu));
        const auto before = s.draft().Serialize();
        QVERIFY(!s.moveItem(folder, other, 0));
        QCOMPARE(s.draft().Serialize(), before);
        QCOMPARE(s.findItem(folder)->SubItems.size(), size_t(1));
        QVERIFY(s.moveItem(child, other, 0));
        QCOMPARE(s.findItem(folder)->SubItems.size(), size_t(0));
        QCOMPARE(s.findItem(other)->SubItems[0].Id, child);
        s.undo();
        QCOMPARE(s.findItem(folder)->SubItems[0].Id, child);
    }
    void legacyRoundtripAndGeometryValidation() {
        auto config = AppConfig::CreateDefault();
        config.GlobalStyle = StyleConfig(); // Old dark styles are not replaced.
        const auto parsed = ParseConfig(config.Serialize());
        QVERIFY2(bool(parsed), qPrintable(parsed.Error));
        QCOMPARE(parsed.Value->Serialize(), config.Serialize());
        config.GlobalStyle.InnerRadius = 100; config.GlobalStyle.OuterRadius = 80;
        QVERIFY(!ParseConfig(config.Serialize()));
        QVERIFY(!ParseConfig("{}"));
        QVERIFY(!ParseConfig("[]"));
        auto json = AppConfig::CreateDefault().ToJson();
        json["version"] = "99";
        QVERIFY(!ParseConfig(QJsonDocument(json).toJson()));
        json = AppConfig::CreateDefault().ToJson();
        auto style = json["globalStyle"].toObject(); style["outerRadius"] = "bad";
        json["globalStyle"] = style;
        QVERIFY(!ParseConfig(QJsonDocument(json).toJson()));
        json = AppConfig::CreateDefault().ToJson();
        auto profiles = json["profiles"].toArray(); auto profile = profiles[0].toObject();
        profile["trigger"] = "not an object"; profiles[0] = profile; json["profiles"] = profiles;
        QVERIFY(!ParseConfig(QJsonDocument(json).toJson()));
    }
};
QTEST_GUILESS_MAIN(CoreTests)
#include "CoreTests.moc"
