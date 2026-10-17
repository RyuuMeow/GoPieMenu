#include <QtTest>
#include <QTemporaryDir>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QDir>
#include <QQuickItem>
#include <QQmlContext>
#include <QElapsedTimer>
#include <QRegularExpression>
#include <QScreen>
#include <QDesktopServices>
#include <cmath>
#include "ui/SettingsWindow.h"
#include "ui/quick/PiePreviewItem.h"
#include "ui/quick/ColorWheelItem.h"
#include "ui/PieMenuWidget.h"
#include "ui/RuntimeMenuController.h"
#include <QPushButton>
#include "ui/rendering/MenuScene.h"
#include "ui/TrayManager.h"
#include "core/ActionExecutor.h"
#include "core/TranslationManager.h"
#include "UpdateTestSupport.h"

using namespace gpm;
class UrlReceiver : public QObject {
    Q_OBJECT
public:
    QUrl url;
public slots:
    void receive(const QUrl& value) { url = value; }
};

class CountingAction : public IActionHandler {
public:
    CountingAction(ActionType type, int& validations, int& executions) : Type(type), Validations(validations), Executions(executions) {}
    ActionType GetSupportedType() const override { return Type; }
    QString GetDisplayName() const override { return "Test action"; }
    bool Validate(const PieItem&) const override { ++Validations; return true; }
    bool Execute(const PieItem&) override { ++Executions; return true; }
private:
    ActionType Type;
    int& Validations;
    int& Executions;
};

class UiTests : public QObject {
    Q_OBJECT
    static bool showEditor(SettingsWindow& editor) {
        editor.show();
        auto* window = editor.window();
        if (!window) return false;
        // Keep the logical test viewport stable when a CI desktop is scaled.
        // Small-window behavior is exercised explicitly at 800 x 520 below.
        window->resize(1160, 760);
        return QTest::qWaitForWindowExposed(window) &&
            QTest::qWaitFor([&] { return window->size() == QSize(1160, 760); });
    }
    static QQuickItem* visibleItem(QQuickItem* root, const QString& value, const char* property = "objectName") {
        if (root->isVisible() && root->property(property).toString() == value) return root;
        for (auto* child : root->childItems()) if (auto* result = visibleItem(child, value, property)) return result;
        return nullptr;
    }
    static QQuickItem* find(QQuickWindow* window, const QString& name, const char* property = "objectName") {
        return visibleItem(window->contentItem(), name, property);
    }
    static bool click(QQuickWindow* window, const QString& name, const char* property = "objectName") {
        QTest::qWait(30); // Let a newly loaded inspector/menu finish its layout before pointer input.
        auto* item = find(window, name, property);
        if (!item) return false;
        const auto point = item->mapToScene(QPointF(item->width()/2, item->height()/2)).toPoint();
        if (!QRect(QPoint(), window->size()).contains(point)) return false;
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, point);
        QCoreApplication::processEvents();
        return true;
    }
    static bool snapshot(QQuickWindow* window, const QString& name) {
        QCoreApplication::processEvents();
        const auto image = window->grabWindow();
        QDir().mkpath("artifacts");
        return !image.isNull() && image.save("artifacts/" + name + "-" + QString::number(window->devicePixelRatio()) + ".png");
    }
private slots:
    void submenuAppearanceTabsKeepTheirOwnDimensions() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        auto data = config.GetConfig();
        data.Profiles[0].Items[0].Action = ActionType::ListMenu;
        data.Profiles[0].Items[0].SubItems = {PieItem::Create("Copy", ActionType::SendHotkey, "Ctrl+C")};
        QVERIFY(config.Commit(data));
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        QVERIFY(showEditor(editor)); auto* window = editor.window(); auto* session = editor.session();
        QVERIFY(click(window, "appearanceButton"));
        QVERIFY(find(window, "fontSizeSlider"));
        QVERIFY(click(window, "submenuAppearanceButton"));
        QCOMPARE(session->folderId(), data.Profiles[0].Items[0].Id);
        QVERIFY(find(window, "submenuFontSizeSlider")); QVERIFY(!find(window, "fontSizeSlider"));
        session->setStyleField("submenuFontSize", 24);
        session->setStyleField("submenuIconSize", 80);
        session->setStyleField("submenuBorderWidth", 5);
        session->setStyleField("submenuTextOutlineThickness", 6);
        QVERIFY(click(window, "pieAppearanceButton"));
        QVERIFY(session->folderId().isEmpty()); QVERIFY(find(window, "fontSizeSlider"));
        QCOMPARE(session->effectiveStyle().FontSize, data.GlobalStyle.FontSize);
        ListScene before; before.build(data.Profiles[0].Items[0].SubItems, session->effectiveStyle());
        const auto render = [](const ListScene& scene) {
            QImage image(scene.size(1).toSize(), QImage::Format_ARGB32_Premultiplied); image.fill(Qt::transparent);
            QPainter painter(&image); scene.draw(painter, 0, 1); return image;
        };
        const auto image = render(before);
        session->setStyleField("fontSize", 8); session->setStyleField("iconSize", 12);
        session->setStyleField("borderWidth", 0); session->setStyleField("textOutlineThickness", 0);
        ListScene after; after.build(data.Profiles[0].Items[0].SubItems, session->effectiveStyle());
        QCOMPARE(render(after), image); QVERIFY(after.rowHeight() >= 90);
        session->setStyleField("submenuTextOutlineThickness", 0);
        after.build(data.Profiles[0].Items[0].SubItems, session->effectiveStyle());
        QVERIFY(render(after) != image);
    }
    void persistentDisplayModeEditor() {
        for (const auto& language : {QString("en"), QString("zh-TW"), QString("zh-CN")}) {
            TranslationManager translations; QVERIFY(translations.load(language));
            QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json"));
            IconService icons; SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
            QVERIFY(showEditor(editor)); auto* window = editor.window();
            QVERIFY(click(window, "triggerSummaryButton"));
            auto* combo = find(window, "displayModeCombo"); QVERIFY(combo);
            combo->setProperty("currentIndex", 1);
            QVERIFY(QMetaObject::invokeMethod(combo, "activated", Q_ARG(int, 1)));
            QTRY_VERIFY(editor.session()->profile()["persistent"].toBool());
            QVERIFY(!find(window, "activationModeCombo"));
            QCOMPARE(editor.session()->profile()["triggerSummary"].toString(), QCoreApplication::translate("gpm::EditorSession", "Persistent floating menu"));
            QVERIFY(snapshot(window, "persistent-settings-" + language));
            editor.session()->undo();
            QTRY_VERIFY(!editor.session()->profile()["persistent"].toBool());
            QTRY_VERIFY(find(window, "activationModeCombo"));
        }
    }
    void persistentClicksDragAndSubmenuToggle() {
        // OS focus and key delivery are covered by NativeInputTests with SendInput.
        IconService icons; PieMenuWidget pie(&icons);
        auto profile = PieMenuConfig::CreateDefault(); profile.Persistent = true;
        profile.Items[0].Action = ActionType::ListMenu;
        profile.Items[0].SubItems = {PieItem::Create("Copy", ActionType::SendHotkey, "Ctrl+C")};
        auto style = StyleConfig::Frost(); style.AnimationDuration = 0;
        pie.ShowAt(QGuiApplication::primaryScreen()->availableGeometry().center(), profile, style, true);
        QVERIFY(QTest::qWaitForWindowExposed(&pie));
        QSignalSpy chosen(&pie, &PieMenuWidget::ItemSelected);
        QSignalSpy moved(&pie, &PieMenuWidget::PositionChanged);
        QSignalSpy hidden(&pie, &PieMenuWidget::HidePersistentRequested);
        auto center = pie.mapFromGlobal(pie.MenuCenter());
        const auto top = center - QPoint(0, 90);
        pie.UpdateMousePos(pie.mapToGlobal(top));
        auto* list = pie.findChild<ListMenuWidget*>("runtimeSubmenu"); QVERIFY(list);
        QVERIFY(!list->IsOpen()); // Hover never opens a resident submenu.
        QTest::mouseClick(&pie, Qt::LeftButton, Qt::NoModifier, top);
        QVERIFY(list->IsOpen());
        QTest::mouseClick(list, Qt::LeftButton, Qt::NoModifier, QPoint(50, 35));
        QTest::mouseClick(list, Qt::LeftButton, Qt::NoModifier, QPoint(50, 35));
        QCOMPARE(chosen.count(), 2); QVERIFY(list->IsOpen()); QVERIFY(pie.IsOpen());
        QTest::mouseClick(&pie, Qt::LeftButton, Qt::NoModifier, center + QPoint(90, 0));
        QCOMPARE(chosen.count(), 3); QVERIFY(list->IsOpen());
        const auto oldCenter = pie.MenuCenter();
        const auto oldListPosition = list->pos();
        QTest::mousePress(&pie, Qt::LeftButton, Qt::NoModifier, center);
        const QPoint shift(50, 35);
        QMouseEvent move(QEvent::MouseMove, center + shift, oldCenter + shift, Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&pie, &move);
        QTest::mouseRelease(&pie, Qt::LeftButton, Qt::NoModifier, pie.mapFromGlobal(pie.MenuCenter()));
        QCOMPARE(pie.MenuCenter(), oldCenter + shift); QCOMPARE(moved.count(), 1);
        QVERIFY(list->pos() != oldListPosition); QCOMPARE(chosen.count(), 3);
        center = pie.mapFromGlobal(pie.MenuCenter());
        QTest::mouseClick(&pie, Qt::LeftButton, Qt::NoModifier, center - QPoint(0, 90));
        QVERIFY(!list->IsOpen());
        QTest::mousePress(&pie, Qt::LeftButton, Qt::NoModifier, center + QPoint(90, 0));
        QTest::mouseRelease(&pie, Qt::LeftButton, Qt::NoModifier, center);
        QCOMPARE(chosen.count(), 3); // A release on another target cancels the click.
        QTest::mouseClick(&pie, Qt::RightButton, Qt::NoModifier, center);
        auto* context = pie.findChild<QWidget*>("persistentContextMenu"); QVERIFY(context && context->isVisible());
        pie.DismissContextOutside(context->geometry().bottomRight() + QPoint(10, 10));
        QVERIFY(!context->isVisible()); QVERIFY(pie.IsOpen());
        QTest::mouseClick(&pie, Qt::RightButton, Qt::NoModifier, center);
        auto* hide = pie.findChild<QPushButton*>("hidePersistentButton"); QVERIFY(hide);
        QTest::mouseClick(hide, Qt::LeftButton); QCOMPARE(hidden.count(), 1);
    }
    void persistentControllerSwitchesHidesAndRestores() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json"));
        auto data = config.GetConfig(); data.GlobalStyle.AnimationDuration = 0;
        auto& global = data.Profiles[0]; global.Id = "global"; global.Persistent = true;
        auto app = global; app.Id = "app"; app.AppFilter = {"paint.exe"};
        app.Items[0].Action = ActionType::ListMenu;
        app.Items[0].SubItems = {PieItem::Create("Copy", ActionType::SendHotkey, "Ctrl+C")};
        auto shortcut = global; shortcut.Id = "shortcut"; shortcut.Persistent = false; shortcut.AppFilter = {"browser.exe"};
        auto globalKey = shortcut; globalKey.Id = "globalKey"; globalKey.AppFilter.clear();
        data.Profiles.push_back(app); data.Profiles.push_back(shortcut); data.Profiles.push_back(globalKey);
        QVERIFY(config.Commit(data));
        QSettings settings(dir.filePath("runtime.ini"), QSettings::IniFormat);
        IconService icons; PieMenuWidget pie(&icons);
        RuntimeMenuController controller(&config, &pie, &settings);
        TrayManager tray;
        connect(&tray, &TrayManager::PersistentHiddenToggled, &controller, &RuntimeMenuController::SetHidden);
        connect(&controller, &RuntimeMenuController::HiddenChanged, &tray, &TrayManager::SetPersistentHidden);
        auto* trayToggle = tray.findChild<QSystemTrayIcon*>()->contextMenu()->findChild<QAction*>("trayPersistentAction");
        QVERIFY(trayToggle);
        controller.SetForegroundProcess("paint.exe"); QCOMPARE(controller.CurrentProfile(), "app");
        const auto original = config.GetConfig().Serialize();
        const auto center = pie.MenuCenter();
        pie.PositionChanged(center + QPoint(30, 20));
        pie.SubmenuChanged(app.Items[0].Id);
        controller.SetForegroundProcess("other.exe"); QCOMPARE(controller.CurrentProfile(), "global");
        controller.SetForegroundProcess("paint.exe"); QCOMPARE(controller.CurrentProfile(), "app");
        QCOMPARE(pie.MenuCenter(), center + QPoint(30, 20));
        QVERIFY(pie.findChild<ListMenuWidget*>("runtimeSubmenu")->IsOpen());
        pie.HidePersistentRequested();
        QVERIFY(controller.IsHidden()); QVERIFY(trayToggle->isChecked()); QVERIFY(!pie.isVisible());
        QVERIFY(!controller.Trigger("globalKey", center));
        controller.SetForegroundProcess("other.exe"); QVERIFY(!pie.isVisible());
        controller.SetForegroundProcess("paint.exe"); QVERIFY(!pie.isVisible());
        trayToggle->trigger(); QVERIFY(!controller.IsHidden()); QCOMPARE(controller.CurrentProfile(), "app");
        controller.SetForegroundProcess("browser.exe");
        QVERIFY(controller.Trigger("shortcut", center)); QVERIFY(!pie.IsPersistent());
        controller.ConfirmShortcut(center + QPoint(90, 0));
        QTRY_COMPARE(controller.CurrentProfile(), "global"); QVERIFY(pie.IsPersistent());
        controller.SetPaused(true); QVERIFY(!pie.isVisible());
        controller.SetPaused(false); QVERIFY(pie.IsOpen());
        controller.SetRecording(true); QVERIFY(!pie.isVisible());
        controller.SetRecording(false); QVERIFY(pie.IsOpen());
        QCOMPARE(config.GetConfig().Serialize(), original);
        auto removed = config.GetConfig();
        removed.Profiles[1].Items[0] = PieItem::Create("Replacement", ActionType::SendHotkey, "Ctrl+V");
        controller.SetForegroundProcess("paint.exe");
        QVERIFY(config.Commit(removed));
        QCOMPARE(controller.CurrentProfile(), "app");
        QVERIFY(!pie.findChild<ListMenuWidget*>("runtimeSubmenu")->IsOpen());
        QVERIFY(config.Commit(data));
        controller.SetHidden(true);
        PieMenuWidget restoredPie(&icons); RuntimeMenuController restored(&config, &restoredPie, &settings);
        QVERIFY(restored.IsHidden()); restored.SetForegroundProcess("paint.exe"); QVERIFY(!restoredPie.isVisible());
        restored.SetHidden(false); QCOMPARE(restoredPie.MenuCenter(), center + QPoint(30, 20));
        QVERIFY(restoredPie.findChild<ListMenuWidget*>("runtimeSubmenu")->IsOpen());
    }
    void initTestCase() {
        QQuickStyle::setStyle("Basic");
        for (auto* screen : QGuiApplication::screens()) qInfo() << "Screen" << screen->name() << screen->geometry() << screen->availableGeometry() << screen->devicePixelRatio();
    }
    void init() { QTest::failOnWarning(QRegularExpression(".*(qrc:/editor|QQml|Binding loop|ReferenceError|TypeError).*")); }
    void hiddenUpdatesUseTrayAndReopenTheirNotes() {
        QTemporaryDir dir; update_test::Network network; network.responses = {{update_test::Release()}};
        UpdateManager updates(update_test::Options(dir.path()), &network);
        ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons, nullptr, &updates); editor.setPlatformIntegrationEnabled(false);
        QSignalSpy toast(&editor, &SettingsWindow::updateToastRequested);
        QSignalSpy dialog(&editor, &SettingsWindow::updateDialogRequested);
        updates.check(); QTRY_COMPARE(toast.count(), 1); QCOMPARE(dialog.count(), 0);
        QCOMPARE(toast[0][0].toString(), "2.1.0");
        QVERIFY(showEditor(editor)); QTRY_COMPARE(dialog.count(), 1);
        auto* popup = editor.window()->findChild<QObject*>("updateDialog"); QVERIFY(popup);
        QTRY_VERIFY(popup->property("visible").toBool());
        QVERIFY(click(editor.window(), "skipUpdateButton"));
        QVERIFY(!updates.available()); QTRY_VERIFY(!popup->property("visible").toBool());
        TrayManager tray;
        auto* icon = tray.findChild<QSystemTrayIcon*>(); QVERIFY(icon);
        auto* action = icon->contextMenu()->findChild<QAction*>("trayUpdateAction"); QVERIFY(action);
        QVERIFY(!action->isVisible()); tray.SetUpdateAvailable("2.1.0"); QVERIFY(action->isVisible());
        QSignalSpy clicked(&tray, &TrayManager::UpdateRequested);
        action->trigger(); QCOMPARE(clicked.count(), 1);
        QVERIFY(QMetaObject::invokeMethod(icon, "messageClicked")); QCOMPARE(clicked.count(), 2);
        tray.SetUpdateAvailable({}); QVERIFY(QMetaObject::invokeMethod(icon, "messageClicked"));
        QCOMPARE(clicked.count(), 2);
    }
    void visibleUpdatesWaitForDialogsAndUseTheInterfaceLanguage() {
        for (const auto& language : {QString("en"), QString("zh-TW"), QString("zh-CN")}) {
            TranslationManager translations; QVERIFY(translations.load(language));
            QTemporaryDir dir; update_test::Network network; network.responses = {{update_test::Release()}};
            UpdateManager updates(update_test::Options(dir.path(), language), &network);
            ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
            SettingsWindow editor(&config, &icons, nullptr, &updates); editor.setPlatformIntegrationEnabled(false);
            QVERIFY(showEditor(editor)); auto* w = editor.window();
            QSignalSpy toast(&editor, &SettingsWindow::updateToastRequested);
            QVERIFY(click(w, "applicationSettingsButton"));
            updates.check(); QTRY_VERIFY(updates.available());
            auto* popup = w->findChild<QObject*>("updateDialog"); QVERIFY(popup);
            QVERIFY(!popup->property("visible").toBool()); QCOMPARE(toast.count(), 0);
            QTest::keyClick(w, Qt::Key_Escape);
            QTRY_VERIFY(popup->property("visible").toBool());
            QCOMPARE(popup->property("title").toString(), QCoreApplication::translate("UpdateDialog", "Update available"));
            auto* notes = find(w, "updateReleaseNotes"); QVERIFY(notes);
            // TextArea canonicalizes Markdown paragraph spacing when exposing text.
            QCOMPARE(notes->property("text").toString().simplified(), updates.notes().simplified());
            QVERIFY(!updates.notes().contains("gpm:lang"));
            w->resize(800, 520); QTRY_COMPARE(w->size(), QSize(800, 520));
            QVERIFY(snapshot(w, "update-" + language));
            QVERIFY(click(w, "skipUpdateButton")); QVERIFY(!updates.available());
        }
    }
    void installingUpdatesProtectsDraftsAndHandlesCancellation() {
        QTemporaryDir dir; update_test::Network network;
        network.responses = {{update_test::Release()}, {update_test::Checksum()}, {update_test::Installer}};
        auto options = update_test::Options(dir.path()); int launches = 0; bool allowInstall = false;
        options.launchInstaller = [&](const QString&, QString* error) {
            ++launches; if (!allowInstall) *error = "Simulated cancelled permission prompt"; return allowInstall;
        };
        UpdateManager updates(options, &network);
        ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons, nullptr, &updates); editor.setPlatformIntegrationEnabled(false);
        QVERIFY(showEditor(editor)); auto* w = editor.window();
        auto* session = editor.session(); session->setProfileField("name", "Draft name");
        updates.check(); QTRY_VERIFY(updates.available());
        QTRY_VERIFY(find(w, "downloadUpdateButton")); QVERIFY(click(w, "downloadUpdateButton"));
        QTRY_VERIFY(updates.ready()); QCOMPARE(launches, 0);
        QSignalSpy exits(&editor, &SettingsWindow::exitConfirmed);
        QVERIFY(click(w, "installUpdateButton"));
        QTRY_VERIFY(find(w, "exitCancelButton")); QVERIFY(click(w, "exitCancelButton"));
        QTest::qWait(100); QCOMPARE(launches, 0); QCOMPARE(exits.count(), 0); QVERIFY(session->dirty());
        // Cancelling installation must not turn a subsequent ordinary Quit into an install.
        editor.requestExit(); QVERIFY(editor.resolveExit("discard")); QCOMPARE(launches, 0); QCOMPARE(exits.count(), 1);
        auto* exitPopup = w->findChild<QObject*>("exitDialog"); QVERIFY(exitPopup);
        QVERIFY(QMetaObject::invokeMethod(exitPopup, "close")); QTest::qWait(100);
        exits.clear(); session->setProfileField("name", ""); // Applying this draft is invalid.
        editor.showUpdate(); QTRY_VERIFY(find(w, "installUpdateButton")); QVERIFY(click(w, "installUpdateButton"));
        QTRY_VERIFY(find(w, "exitApplyButton")); QVERIFY(click(w, "exitApplyButton"));
        QCOMPARE(exits.count(), 0); QCOMPARE(launches, 0); QVERIFY(session->dirty());
        session->setProfileField("name", "Saved name");
        QVERIFY(click(w, "exitApplyButton")); QCOMPARE(exits.count(), 0); QCOMPARE(launches, 1);
        QVERIFY(!session->dirty()); QVERIFY(updates.ready());
        QVERIFY(click(w, "exitCancelButton")); QTest::qWait(100);
        allowInstall = true; editor.showUpdate(); QTRY_VERIFY(find(w, "installUpdateButton"));
        QVERIFY(click(w, "installUpdateButton")); QCOMPARE(exits.count(), 1); QCOMPARE(launches, 2);
    }
    void longReleaseNotesKeepUpdateActionsVisible() {
        QTemporaryDir dir; update_test::Network network;
        auto body = update_test::Notes;
        body.replace("- English update.", QString("- A detailed release change with enough text to wrap in a compact window.\n").repeated(80));
        network.responses = {{update_test::Release("v2.1.0", body)}};
        UpdateManager updates(update_test::Options(dir.path()), &network);
        ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons, nullptr, &updates); editor.setPlatformIntegrationEnabled(false);
        QVERIFY(showEditor(editor)); auto* w = editor.window(); w->resize(800, 520);
        updates.check(); QTRY_VERIFY(find(w, "skipUpdateButton"));
        QVERIFY(snapshot(w, "update-long-notes"));
        QVERIFY(click(w, "skipUpdateButton")); QVERIFY(!updates.available());
    }
    void chineseEditorsAndLanguageSelection() {
        const auto previousFont = QGuiApplication::font();
        for (const auto& language : {QString("zh-TW"), QString("zh-CN")}) {
            TranslationManager translations; QVERIFY(translations.load(language));
            QGuiApplication::setFont(QFont(InterfaceFontFamily(language)));
            QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
            auto initial = config.GetConfig(); initial.Language = language; QVERIFY(config.Commit(initial));
            SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
            QCOMPARE(editor.fontFamily(), "Microsoft JhengHei UI");
            QVERIFY(showEditor(editor)); auto* w = editor.window();
            auto* apply = find(w, "applyButton"); QVERIFY(apply);
            QCOMPARE(apply->property("text").toString(), QCoreApplication::translate("Main", "Apply"));
            QVERIFY(apply->property("text").toString() != "Apply");
            QVERIFY(click(w, "applicationSettingsButton"));
            auto* combo = find(w, "languageCombo"); QVERIFY(combo);
            QTRY_COMPARE(combo->property("currentValue").toString(), language);
            combo->setProperty("currentIndex", 1);
            QVERIFY(QMetaObject::invokeMethod(combo, "activated", Q_ARG(int, combo->property("currentIndex").toInt())));
            QCOMPARE(editor.session()->language(), "en");
            QVERIFY(editor.session()->dirty());
            QCOMPARE(config.GetConfig().Language, language);
            QTRY_VERIFY(find(w, "restartNowButton"));
            QCOMPARE(find(w, "restartNowButton")->property("kind").toString(), "primary");
            QVERIFY(snapshot(w, "restart-" + language));
            QVERIFY(click(w, "restartLaterButton"));
            QVERIFY(editor.session()->dirty());
            QCOMPARE(config.GetConfig().Language, language);
        }
        QGuiApplication::setFont(previousFont);
    }
    void languageRestartSavesBeforeLaunching() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        QSignalSpy exited(&editor, &SettingsWindow::exitConfirmed);
        QVERIFY(editor.chooseLanguage("zh-TW"));
        QVERIFY(!editor.chooseLanguage("zh-TW"));
        int launches = 0;
        editor.setRestartHandler([&] {
            ++launches;
            return config.GetConfig().Language == "zh-TW" && !editor.session()->dirty();
        });
        QVERIFY(editor.restartForLanguage());
        QCOMPARE(launches, 1); QCOMPARE(exited.count(), 1);
        ConfigManager saved(nullptr, dir.filePath("config.json"));
        QCOMPARE(saved.GetConfig().Language, "zh-TW");
        QVERIFY(editor.chooseLanguage("en"));
        editor.setRestartHandler([] { return false; });
        QVERIFY(!editor.restartForLanguage());
        QCOMPARE(exited.count(), 1);
        QCOMPARE(config.GetConfig().Language, "en");
        QVERIFY(!editor.session()->error().isEmpty());
    }
    void languageRestartSaveFailureKeepsDraft() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.path()); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        QSignalSpy exited(&editor, &SettingsWindow::exitConfirmed);
        bool launched = false;
        editor.setRestartHandler([&] { launched = true; return true; });
        QVERIFY(editor.chooseLanguage("zh-TW"));
        QVERIFY(!editor.restartForLanguage());
        QVERIFY(!launched); QCOMPARE(exited.count(), 0);
        QVERIFY(editor.session()->dirty());
        QCOMPARE(config.GetConfig().Language, "system");
    }
    void appearanceFollowsTheSelectedProfile() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        auto initial = config.GetConfig();
        auto other = initial.Profiles[0]; other.Id = "other-menu"; other.Name = "Other menu";
        initial.Profiles.push_back(other); QVERIFY(config.Commit(initial));
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        auto* w = editor.window(); QVERIFY(w); QVERIFY(showEditor(editor));
        auto* session = editor.session(); const auto first = session->profileId();
        const auto originalStyle = session->effectiveStyle().ToJson();
        auto* arrange = find(w, "arrangeButton"); QVERIFY(arrange);
        auto* arrangeIcon = arrange->findChild<QQuickItem*>("buttonIcon"); QVERIFY(arrangeIcon);
        QTRY_COMPARE(arrangeIcon->childItems().first()->property("status").toInt(), 1);
        QVERIFY(click(w, "arrangeButton")); QVERIFY(arrange->property("checked").toBool());
        QVERIFY(click(w, "arrangeButton")); QVERIFY(!arrange->property("checked").toBool());
        QVERIFY(click(w, "appearanceButton"));
        QVERIFY(find(w, "Appearance applies only to this menu.", "text"));
        QVERIFY(click(w, "Ocean", "text"));
        auto* size = find(w, "outerRadiusSlider"); QVERIFY(size);
        QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, size->mapToScene(QPointF(size->width() * .65, size->height()/2)).toPoint());
        QVERIFY(session->effectiveStyle().OuterRadius > 200);
        const auto firstStyle = session->effectiveStyle().ToJson();
        session->selectProfile(other.Id);
        QTRY_COMPARE(size->property("value").toDouble(), initial.GlobalStyle.OuterRadius);
        QCOMPARE(session->effectiveStyle().ToJson(), originalStyle);
        QVERIFY(click(w, "Slate", "text"));
        const auto otherStyle = session->effectiveStyle().ToJson();
        session->selectProfile(first);
        QTRY_COMPARE(size->property("value").toDouble(), firstStyle["outerRadius"].toDouble());
        QCOMPARE(session->effectiveStyle().ToJson(), firstStyle);
        QVERIFY(snapshot(w, "appearance-profile"));
        QVERIFY(click(w, "applyButton")); QVERIFY(!session->dirty());
        QCOMPARE(config.GetConfig().GlobalStyle.ToJson(), originalStyle);
        PieScene scene;
        scene.build(config.GetConfig().Profiles[0], config.GetConfig().GlobalStyle);
        QCOMPARE(scene.style().ToJson(), firstStyle);
        scene.build(config.GetConfig().Profiles[1], config.GetConfig().GlobalStyle);
        QCOMPARE(scene.style().ToJson(), otherStyle);
    }
    void emptyActionsDoNotDispatch() {
        ActionExecutor executor; int validations = 0, executions = 0;
        QSignalSpy failed(&executor, &ActionExecutor::ActionFailed), executed(&executor, &ActionExecutor::ActionExecuted);
        for (int action = 1; action <= 5; ++action) {
            const auto type = static_cast<ActionType>(action);
            executor.RegisterHandler(std::make_unique<CountingAction>(type, validations, executions));
            for (const auto& data : {QString(), QString(" \t\n")})
                QVERIFY(executor.Execute(PieItem::Create("Empty", type, data, {}, "arguments alone do not execute")));
        }
        QVERIFY(executor.Execute(PieItem::Create("None", ActionType::None, "ignored")));
        QCOMPARE(validations, 0); QCOMPARE(executions, 0); QCOMPARE(failed.count(), 0); QCOMPARE(executed.count(), 0);
        QVERIFY(executor.Execute(PieItem::Create("Configured", ActionType::LaunchApp, "test-only")));
        QCOMPARE(validations, 1); QCOMPARE(executions, 1); QCOMPARE(executed.count(), 1);
    }
    void actionContextMenusAndPersistentHint() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        auto* w = editor.window(); QVERIFY(w); QVERIFY(showEditor(editor));
        auto* session = editor.session(); auto* canvas = w->findChild<PiePreviewItem*>("pieCanvas"); QVERIFY(canvas);
        auto* menu = w->findChild<QObject*>("actionContextMenu"); QVERIFY(menu);
        const auto original = session->items()[0].toMap()["id"].toString();
        auto point = canvas->mapToScene(canvas->itemCenter(0)).toPoint();
        QTest::mouseClick(w, Qt::RightButton, Qt::NoModifier, point);
        QTRY_VERIFY(menu->property("visible").toBool()); QVERIFY(session->selectedId().isEmpty());
        QTest::qWait(100); // Let the first asynchronous icon requests reach the preview capture.
        QVERIFY(snapshot(w, "action-context-menu"));
        QVERIFY(click(w, "contextDuplicateAction"));
        const auto duplicate = session->selectedId(); QVERIFY(duplicate != original); QCOMPARE(session->items().size(), 5);
        QTest::qWait(30); point = canvas->mapToScene(canvas->itemCenter(0)).toPoint();
        QTest::mouseClick(w, Qt::RightButton, Qt::NoModifier, point);
        QTRY_VERIFY(menu->property("visible").toBool());
        QVERIFY(click(w, "contextDeleteAction"));
        QCOMPARE(session->items().size(), 4); QVERIFY(!session->findItem(original)); QCOMPARE(session->selectedId(), duplicate);
        session->undo(); QVERIFY(session->findItem(original));
        auto* hint = find(w, "previewHint"); QVERIFY(hint);
        const auto hintText = hint->property("text").toString(); const auto hintPosition = hint->mapToScene(QPointF());
        QVERIFY(click(w, "menuEnabledToggle")); QVERIFY(!session->profile()["enabled"].toBool());
        QVERIFY(find(w, "previewHint")); QCOMPARE(hint->property("text").toString(), hintText);
        QCOMPARE(hint->mapToScene(QPointF()), hintPosition);

        const auto folder = session->addItem(int(ActionType::ListMenu)); session->enterFolder(folder);
        const auto child = session->addItem(int(ActionType::LaunchApp)); session->selectItem({}); QTest::qWait(30);
        point = canvas->mapToScene(canvas->itemCenter(0)).toPoint();
        QTest::mouseClick(w, Qt::RightButton, Qt::NoModifier, point);
        QTRY_VERIFY(menu->property("visible").toBool()); QVERIFY(session->selectedId().isEmpty());
        QVERIFY(click(w, "contextDuplicateAction")); QCOMPARE(session->items().size(), 2);
        const auto childCopy = session->selectedId();
        QVERIFY(click(w, "arrangeButton")); QTest::qWait(30);
        auto* row = find(w, "arrange-row-" + child); QVERIFY(row);
        QTest::mouseClick(w, Qt::RightButton, Qt::NoModifier, row->mapToScene(QPointF(110,20)).toPoint());
        QTRY_VERIFY(menu->property("visible").toBool()); QCOMPARE(session->selectedId(), childCopy);
        QVERIFY(click(w, "contextDeleteAction"));
        QVERIFY(!session->findItem(child)); QCOMPARE(session->items().size(), 1); QCOMPARE(session->selectedId(), childCopy);
        session->selectItem({}); QTest::qWait(30);
        QTest::mouseClick(w, Qt::RightButton, Qt::NoModifier, canvas->mapToScene(QPointF(8,8)).toPoint());
        QVERIFY(!menu->property("visible").toBool()); QVERIFY(session->selectedId().isEmpty());
    }
    void githubLinkAndTrayMenu() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        auto* w = editor.window(); QVERIFY(w); QVERIFY(showEditor(editor));
        QVERIFY(click(w, "applicationSettingsButton"));
        auto* github = find(w, "projectGithubButton"); QVERIFY(github);
        auto* githubIcon = github->findChild<QQuickItem*>("buttonIcon"); QVERIFY(githubIcon); QVERIFY(!githubIcon->childItems().isEmpty());
        QTRY_COMPARE(githubIcon->childItems().first()->property("status").toInt(), 1); // Image.Ready
        QVERIFY(snapshot(w, "settings-github"));
        UrlReceiver receiver; QDesktopServices::setUrlHandler("https", &receiver, "receive");
        QVERIFY(click(w, "projectGithubButton"));
        QCOMPARE(receiver.url, QUrl("https://github.com/RyuuMeow/GoPieMenu"));
        QDesktopServices::unsetUrlHandler("https");
        QTest::keyClick(w, Qt::Key_Escape);
        TrayManager tray;
        auto* icon = tray.findChild<QSystemTrayIcon*>(); QVERIFY(icon); QVERIFY(!icon->icon().isNull());
        QVERIFY(!QIcon(":/logo/GoPieMenu.ico").pixmap(32,32).isNull());
        auto* menu = icon->contextMenu(); QVERIFY(menu);
        QSignalSpy pause(&tray, &TrayManager::PauseToggled);
        menu->popup(w->mapToGlobal(QPoint(100,160))); QTRY_VERIFY(menu->isVisible());
        menu->setActiveAction(menu->actions()[0]); QTest::qWait(30);
        QVERIFY(menu->grab().save("artifacts/tray-menu-" + QString::number(w->devicePixelRatio()) + ".png"));
        auto* pauseAction = menu->findChild<QAction*>("trayPauseAction"); QVERIFY(pauseAction);
        QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier, menu->actionGeometry(pauseAction).center());
        QCOMPARE(pause.count(), 1); QVERIFY(pause[0][0].toBool());
        QVERIFY(!menu->isVisible());
    }
    void placementAlwaysFits() {
        const auto config = AppConfig::CreateDefault();
        PieScene scene; auto style = config.GlobalStyle; style.OuterRadius = 400;
        scene.build(config.Profiles[0], style);
        for (const auto& area : {QRect(0,0,1920,1040), QRect(-1280,0,1280,680), QRect(0,0,960,500)})
            for (const auto& point : {area.topLeft(), area.topRight(), area.bottomLeft(), area.bottomRight(), area.center()}) {
                const auto placement = PlaceMenu(scene.bounds(), point, area);
                QVERIFY(area.contains(placement.Window));
                QVERIFY(placement.Scale > 0 && placement.Scale <= 1);
            }
    }
    void editorLoadsAndSelectsOnCanvas() {
        QTemporaryDir dir;
        ConfigManager config(nullptr, dir.filePath("config.json"));
        IconService icons;
        SettingsWindow editor(&config, &icons);
        editor.setPlatformIntegrationEnabled(false);
        QVERIFY(editor.window());
        QVERIFY(showEditor(editor));
        qInfo() << "Editor window" << editor.window()->geometry() << editor.window()->isVisible() << editor.window()->visibility();
        QVERIFY(QTest::qWaitForWindowExposed(editor.window()));
        auto* canvas = editor.window()->findChild<PiePreviewItem*>("pieCanvas");
        QVERIFY(canvas);
        QTRY_VERIFY(canvas->width() > 0);
        const auto point = canvas->mapToScene(canvas->itemCenter(0)).toPoint();
        QTest::mouseClick(editor.window(), Qt::LeftButton, Qt::NoModifier, point);
        QTRY_VERIFY(!editor.session()->selectedId().isEmpty());
        QVERIFY(click(editor.window(), "itemNameField"));
        QTest::keyClick(editor.window(), Qt::Key_A, Qt::ControlModifier);
        for (const char c : QByteArray("My action")) QTest::keyClick(editor.window(), c);
        QVERIFY(editor.session()->dirty());
        QCOMPARE(config.GetConfig().Profiles[0].Items[0].Name, QString("Notepad"));
        QTest::qWait(250);
        const auto frame = editor.window()->grabWindow();
        const auto physical = canvas->mapToScene(canvas->itemCenter(0)) * editor.window()->devicePixelRatio();
        QVERIFY(frame.pixelColor(physical.toPoint()) != frame.pixelColor(10, 160));
        QVERIFY(snapshot(editor.window(), "editor"));
        editor.window()->resize(800, 520);
        auto* drawer = editor.window()->findChild<QObject*>("compactDrawer"); QVERIFY(drawer);
        QTRY_COMPARE(drawer->property("position").toDouble(), 1.0);
        QCOMPARE(editor.window()->width(), 800);
        QVERIFY(find(editor.window(), "itemNameField"));
        QVERIFY(snapshot(editor.window(), "compact"));
        editor.window()->close();
        QVERIFY(editor.session()->dirty());
        editor.show();
        QCOMPARE(editor.session()->selectedItem()["name"].toString(), QString("My action"));
    }
    void shortcutIconsSubmenusAndApply() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        QVERIFY(editor.window()); QVERIFY(showEditor(editor));
        auto* w = editor.window(); auto* session = editor.session();
        QVERIFY(click(w, "profileSelector"));
        QTRY_VERIFY(find(w, "New Pie Menu", "text"));
        QTest::keyClick(w, Qt::Key_Escape);
        QVERIFY(click(w, "addActionButton"));
        QTRY_VERIFY(find(w, "Keyboard shortcut", "text"));
        QVERIFY(click(w, "Keyboard shortcut", "text"));
        QTRY_COMPARE(session->selectedItem()["action"].toInt(), 3);
        QTRY_VERIFY(find(w, "actionTypeCombo"));
        QTRY_COMPARE(find(w, "actionTypeCombo")->property("displayText").toString(), QString("Keyboard shortcut"));
        QVERIFY(click(w, "actionRecorderButton"));
        QTest::keyClick(w, Qt::Key_K, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(session->selectedItem()["target"].toString(), QString("Ctrl+Shift+K"));
        QVERIFY(click(w, "chooseIconButton"));
        QTRY_VERIFY(find(w, "iconGrid"));
        auto* catalog = qvariant_cast<QObject*>(qmlContext(w)->contextProperty("iconCatalog"));
        QVERIFY(catalog);
        QTRY_VERIFY(catalog->property("count").toInt() > 1000);
        QVERIFY(click(w, "iconSearch"));
        QElapsedTimer timer; timer.start();
        for (const char c : QByteArray("folder")) QTest::keyClick(w, c);
        while (catalog->property("query").toString() != "folder" && timer.elapsed() < 500) QTest::qWait(1);
        QCOMPARE(catalog->property("query").toString(), QString("folder"));
        qInfo() << "Search including 16ms debounce:" << timer.elapsed() << "ms";
        QVERIFY(catalog->property("count").toInt() > 0 && catalog->property("count").toInt() < 100);
        QVERIFY(snapshot(w, "icons"));
        auto* grid = find(w, "iconGrid"); QVERIFY(grid);
        QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, grid->mapToScene(QPointF(40,40)).toPoint());
        QTRY_VERIFY(session->selectedItem()["icon"].toString().contains("folder"));
        for (int i = 0; i < 3; ++i) {
            timer.restart(); QVERIFY(click(w, "chooseIconButton"));
            QTRY_VERIFY(find(w, "iconGrid"));
            const auto frame = w->grabWindow(); QVERIFY(!frame.isNull());
            qInfo() << "Warm picker open + first frame:" << timer.elapsed() << "ms";
            QTest::keyClick(w, Qt::Key_Escape); QCoreApplication::processEvents();
        }
        const auto folder = session->addItem(int(ActionType::ListMenu));
        QVERIFY(click(w, "enterSubmenuButton"));
        QCOMPARE(session->folderId(), folder);
        const auto child = session->addItem(int(ActionType::OpenURL));
        session->setItemField("target", "https://example.com");
        const auto second = session->addItem(int(ActionType::SendHotkey));
        session->setItemField("target", "Ctrl+C");
        session->moveSelected(-1);
        QCOMPARE(session->items()[0].toMap()["id"].toString(), second);
        QVERIFY(snapshot(w, "submenu"));
        session->undo(); QCOMPARE(session->items()[0].toMap()["id"].toString(), child);
        session->leaveFolder();
        QVERIFY(click(w, "arrangeButton"));
        QTest::qWait(30);
        const auto movedId = session->items()[0].toMap()["id"].toString();
        const auto nextId = session->items()[1].toMap()["id"].toString();
        auto* handle = find(w, "drag-handle-" + movedId); QVERIFY(handle);
        auto* nextHandle = find(w, "drag-handle-" + nextId); QVERIFY(nextHandle);
        const auto from = handle->mapToScene(QPointF(12, 20)).toPoint();
        const auto to = nextHandle->mapToScene(QPointF(12, 30)).toPoint();
        QTest::mousePress(w, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(w, from + QPoint(0, 16), 20);
        QTest::mouseMove(w, to, 30);
        QTest::qWait(30);
        QTest::mouseRelease(w, Qt::LeftButton, Qt::NoModifier, to);
        QTRY_COMPARE_WITH_TIMEOUT(session->items()[1].toMap()["id"].toString(), movedId, 2000);
        QVERIFY(snapshot(w, "arrange"));
        session->undo();
        QVERIFY(click(w, "arrangeButton"));
        QVERIFY(click(w, "appearanceButton"));
        QVERIFY(snapshot(w, "appearance"));
        QVERIFY(click(w, "triggerSummaryButton"));
        session->setProfileField("triggerMode", 1);
        QVERIFY(click(w, "triggerRecorderButton"));
        QTest::keyClick(w, Qt::Key_M, Qt::ControlModifier);
        QCOMPARE(session->profile()["vkCode"].toInt(), int('M'));
        QVERIFY(snapshot(w, "trigger"));
        QVERIFY(click(w, "applyButton"));
        QVERIFY2(!session->dirty(), qPrintable(session->error()));
        QCOMPARE(config.GetConfig().Profiles[0].Items.size(), size_t(6));
    }
    void menuLifecycleAndStableControls() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        auto* w = editor.window(); QVERIFY(w); QVERIFY(showEditor(editor));
        auto* session = editor.session();
        const auto first = session->profileId();
        auto* apply = find(w, "applyButton"); QVERIFY(apply); QVERIFY(!apply->isEnabled());
        auto* discard = find(w, "discardButton"); QVERIFY(discard); QVERIFY(!discard->isEnabled());
        const auto applyPosition = apply->mapToScene(QPointF());
        auto* manage = find(w, "manageMenusButton"); QVERIFY(manage);
        auto* menu = w->findChild<QObject*>("profileMenu"); QVERIFY(menu);
        auto* tip = manage->findChild<QObject*>("manageMenusButtonTip"); QVERIFY(tip);
        QTest::mouseMove(w, QPoint(w->width()/2, w->height()/2)); QTest::qWait(30);
        QTest::mouseMove(w, manage->mapToScene(QPointF(20,20)).toPoint());
        QTRY_VERIFY_WITH_TIMEOUT(tip->property("visible").toBool(), 1500);
        QVERIFY(click(w, "manageMenusButton"));
        QTRY_VERIFY(menu->property("visible").toBool());
        QVERIFY(!tip->property("visible").toBool());
        QVERIFY(snapshot(w, "menu-popup"));
        QVERIFY(click(w, "manageMenusButton"));
        QTRY_VERIFY(!menu->property("visible").toBool());
        QVERIFY(click(w, "newMenuButton"));
        QVERIFY(session->profileId() != first);
        QTRY_COMPARE(find(w, "profileSelector")->property("currentValue").toString(), session->profileId());
        QCOMPARE(find(w, "profileSelector")->property("displayText").toString(), QString("New menu"));
        QVERIFY(apply->isEnabled()); QCOMPARE(apply->mapToScene(QPointF()), applyPosition);
        auto* toggle = find(w, "menuEnabledToggle"); QVERIFY(toggle);
        QVERIFY(!toggle->property("checked").toBool()); QVERIFY(!toggle->isEnabled());
        session->setProfileField("name", "Second menu");
        QVERIFY(click(w, "manageMenusButton"));
        QVERIFY(click(w, "New menu", "text"));
        QTRY_COMPARE(find(w, "profileSelector")->property("currentValue").toString(), session->profileId());
        QVERIFY(click(w, "manageMenusButton"));
        QVERIFY(click(w, "Delete menu", "text"));
        QCOMPARE(session->profileId(), first);
        QTRY_COMPARE(find(w, "profileSelector")->property("currentIndex").toInt(), 0);
        QVERIFY(click(w, "discardButton"));
        QVERIFY(!apply->isEnabled()); QCOMPARE(apply->mapToScene(QPointF()), applyPosition);
        QVERIFY(click(w, "manageMenusButton"));
        QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, QPoint(w->width()/2, w->height()/2));
        QTRY_VERIFY(!menu->property("visible").toBool());
    }
    void renameKeepsPreviewGeometry() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        auto* w = editor.window(); QVERIFY(w); QVERIFY(showEditor(editor));
        auto* session = editor.session();
        session->selectItem(session->items()[1].toMap()["id"].toString());
        auto* canvas = w->findChild<PiePreviewItem*>("pieCanvas"); QVERIFY(canvas);
        QTest::qWait(50);
        const auto firstPoint = canvas->mapToScene(canvas->itemCenter(0));
        const auto secondPoint = canvas->mapToScene(canvas->itemCenter(1));
        const auto size = canvas->size();
        for (const auto& name : {QString("X"), QString(180, QChar('W')), QString::fromUtf8("非常長的動作名稱，用來確認中央預覽不會移動")}) {
            session->setItemField("name", name); QTest::qWait(30);
            QCOMPARE(canvas->size(), size);
            QCOMPARE(canvas->mapToScene(canvas->itemCenter(0)), firstPoint);
            QCOMPARE(canvas->mapToScene(canvas->itemCenter(1)), secondPoint);
        }
        auto* button = find(w, "chooseIconButton"); QVERIFY(button);
        auto* icon = button->findChild<QQuickItem*>("buttonIcon"); QVERIFY(icon);
        const auto center = icon->mapToItem(button, QPointF(icon->width()/2, icon->height()/2));
        QVERIFY(std::abs(center.x() - button->width()/2) < 1);
        QVERIFY(std::abs(center.y() - button->height()/2) < 1);
        QVERIFY(click(w, "itemAdvancedButton"));
        session->setItemField("color", "#dd5555");
        QVERIFY(snapshot(w, "text-actions"));
        QVERIFY(click(w, "resetItemColorButton"));
        QVERIFY(session->selectedItem()["color"].toString().isEmpty());
    }
    void dragRowsToBothBoundariesAndCancel() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        auto* w = editor.window(); QVERIFY(w); QVERIFY(showEditor(editor));
        auto* session = editor.session();
        const auto folder = session->addItem(int(ActionType::ListMenu)); session->enterFolder(folder);
        QStringList ids;
        for (int i = 0; i < 3; ++i) { ids.append(session->addItem(3)); session->setItemField("name", QString("Action %1").arg(i)); }
        session->selectItem({});
        QVERIFY(click(w, "arrangeButton")); QTest::qWait(50);
        auto* viewport = find(w, "arrangeViewport"); QVERIFY(viewport);
        // Begin over the row's blank area, then insert before the first row.
        auto* row = find(w, "arrange-row-" + ids[2]); QVERIFY(row);
        auto from = row->mapToScene(QPointF(row->width()*.65, 20)).toPoint();
        auto to = viewport->mapToScene(QPointF(120, 2)).toPoint();
        QTest::mousePress(w, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(w, from - QPoint(0,20), 20); QTest::mouseMove(w, to, 30);
        QTRY_VERIFY(find(w, "insertionLine")); QVERIFY(find(w, "dragPreview"));
        QVERIFY(snapshot(w, "drag-insertion"));
        QTest::mouseRelease(w, Qt::LeftButton, Qt::NoModifier, to);
        QTRY_COMPARE(session->items()[0].toMap()["id"].toString(), ids[2]);
        QVERIFY(session->selectedId().isEmpty()); QVERIFY(!find(w, "wideInspector"));
        session->undo(); QTest::qWait(30);
        session->selectItem(ids[1]); QTest::qWait(30);
        // Begin over the label, then insert after the last row.
        row = find(w, "arrange-row-" + ids[0]); QVERIFY(row);
        from = row->mapToScene(QPointF(105,20)).toPoint();
        to = viewport->mapToScene(QPointF(120, viewport->height()-2)).toPoint();
        QTest::mousePress(w, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(w, from + QPoint(0,20), 20); QTest::mouseMove(w, to, 30);
        QTRY_VERIFY(find(w, "insertionLine"));
        QTest::mouseRelease(w, Qt::LeftButton, Qt::NoModifier, to);
        QTRY_COMPARE(session->items()[2].toMap()["id"].toString(), ids[0]);
        QCOMPARE(session->selectedId(), ids[1]);
        const auto before = session->items(); QTest::qWait(30);
        row = find(w, "arrange-row-" + ids[0]); QVERIFY(row);
        from = row->mapToScene(QPointF(110,20)).toPoint();
        to = viewport->mapToScene(QPointF(120,2)).toPoint();
        QTest::mousePress(w, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(w, to, 30); QTRY_VERIFY(find(w, "insertionLine"));
        QTest::keyClick(w, Qt::Key_Escape);
        QTest::mouseRelease(w, Qt::LeftButton, Qt::NoModifier, to);
        QCOMPARE(session->items(), before); QVERIFY(!find(w, "insertionLine"));
    }
    void dragAutoscrollKeepsGestureAlive() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        auto* w = editor.window(); QVERIFY(w); QVERIFY(showEditor(editor));
        auto* session = editor.session();
        const auto folder = session->addItem(int(ActionType::ListMenu)); session->enterFolder(folder);
        QString first;
        for (int i = 0; i < 12; ++i) { const auto id = session->addItem(3); if (!i) first = id; }
        QVERIFY(click(w, "arrangeButton")); QTest::qWait(30);
        auto* list = find(w, "arrangeList"); auto* viewport = find(w, "arrangeViewport"); QVERIFY(list); QVERIFY(viewport);
        auto* row = find(w, "arrange-row-" + first); QVERIFY(row);
        auto from = row->mapToScene(QPointF(110,20)).toPoint();
        auto to = viewport->mapToScene(QPointF(120,viewport->height()-2)).toPoint();
        QTest::mousePress(w, Qt::LeftButton, Qt::NoModifier, from); QTest::mouseMove(w, to, 30);
        QTRY_VERIFY_WITH_TIMEOUT(list->property("contentY").toReal() >= list->property("contentHeight").toReal() - list->height() - 1, 4000);
        QVERIFY(find(w, "insertionLine"));
        QTest::mouseRelease(w, Qt::LeftButton, Qt::NoModifier, to);
        QTRY_COMPARE(session->items().last().toMap()["id"].toString(), first);
    }
    void mouseModifiersAndKeyboardRecordingKeepMode() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        auto* w = editor.window(); QVERIFY(w); QVERIFY(showEditor(editor));
        auto* session = editor.session();
        QVERIFY(click(w, "triggerSummaryButton"));
        QVERIFY(click(w, "modifierWin")); QVERIFY(click(w, "modifierShift"));
        QCOMPARE(session->profile()["modifiers"].toInt(), 11);
        QCOMPARE(session->profile()["triggerMode"].toInt(), 0);
        QVERIFY(click(w, "mouseButtonCombo"));
        QTest::keyClick(w, Qt::Key_Down); QTest::keyClick(w, Qt::Key_Down); QTest::keyClick(w, Qt::Key_Return);
        QTRY_COMPARE(session->profile()["mouseButton"].toString(), QString("X1"));
        QCOMPARE(session->profile()["triggerMode"].toInt(), 0);
        QVERIFY(snapshot(w, "mouse-trigger"));
        auto* recorder = qvariant_cast<QObject*>(qmlContext(w)->contextProperty("recorder")); QVERIFY(recorder);
        QVERIFY(QMetaObject::invokeMethod(recorder, "start", Q_ARG(QString, QString("trigger"))));
        QTest::keyClick(w, Qt::Key_K, Qt::ControlModifier);
        QCOMPARE(session->profile()["triggerMode"].toInt(), 0);
        QCOMPARE(session->profile()["modifiers"].toInt(), 11);
        QVERIFY(QMetaObject::invokeMethod(recorder, "cancel"));
        session->setProfileField("triggerMode", 2);
        QVERIFY(click(w, "triggerRecorderButton"));
        QTest::keyClick(w, Qt::Key_K, Qt::MetaModifier | Qt::ShiftModifier);
        QCOMPARE(session->profile()["triggerMode"].toInt(), 2);
        QCOMPARE(session->profile()["vkCode"].toInt(), int('K'));
        QCOMPARE(session->profile()["modifiers"].toInt(), 10);
        QCOMPARE(session->profile()["mouseButton"].toString(), QString("X1"));
    }
    void colorWheelConversionAndDialogTransactions() {
        ColorWheelItem numeric;
        QVERIFY(numeric.setHex("#33669980"));
        QCOMPARE(numeric.hex(), QString("#33669980"));
        QCOMPARE(numeric.color().alpha(), 128);
        QVERIFY(!numeric.setHex("#bad-input")); QCOMPARE(numeric.hex(), QString("#33669980"));
        numeric.setHue(1.0/3); numeric.setSaturation(1); numeric.setValue(1); numeric.setAlpha(1);
        QCOMPARE(numeric.hex(), QString("#00FF00"));
        numeric.setColor(Qt::black); QVERIFY(std::abs(numeric.hue() - 1.0/3) < .001);

        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        auto* w = editor.window(); QVERIFY(w); QVERIFY(showEditor(editor));
        auto* session = editor.session();
        session->selectItem(session->items()[0].toMap()["id"].toString());
        QVERIFY(click(w, "itemAdvancedButton"));
        QVERIFY(click(w, "Custom color", "text"));
        auto* popup = w->findChild<QObject*>("colorPicker"); QVERIFY(popup);
        QTRY_VERIFY(popup->property("visible").toBool());
        auto* wheel = w->findChild<ColorWheelItem*>("colorWheel"); QVERIFY(wheel);
        QTest::qWait(30); // Dialog body/footer layout settles after opening.
        QTest::keyClick(w, Qt::Key_S, Qt::ControlModifier);
        QVERIFY(!QFile::exists(config.GetConfigFilePath()));
        const auto original = session->selectedItem()["color"].toString();
        QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, wheel->mapToScene(QPointF(wheel->width()-17, wheel->height()/2)).toPoint());
        QVERIFY(wheel->hue() < .02 || wheel->hue() > .98);
        QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, wheel->mapToScene(QPointF(wheel->width()/2, wheel->height()/2)).toPoint());
        QVERIFY(std::abs(wheel->saturation()-.5) < .02); QVERIFY(std::abs(wheel->value()-.5) < .02);
        QCOMPARE(session->selectedItem()["color"].toString(), original);
        QVERIFY(click(w, "cancelColorButton"));
        QCOMPARE(session->selectedItem()["color"].toString(), original);
        editor.chooseColor("color", true);
        QVERIFY(click(w, "colorHexField"));
        QTest::keyClick(w, Qt::Key_A, Qt::ControlModifier);
        for (const char c : QByteArray("#33669980")) QTest::keyClick(w, c);
        QCOMPARE(wheel->hex(), QString("#33669980"));
        QVERIFY(click(w, "useColorButton"));
        QCOMPARE(QColor(session->selectedItem()["color"].toString()), QColor("#80336699"));
        QCOMPARE(editor.recentColors().front(), QString("#80336699"));
        editor.chooseColor("color", true);
        QVERIFY(click(w, "colorHexField"));
        QTest::keyClick(w, Qt::Key_A, Qt::ControlModifier); QTest::keyClick(w, Qt::Key_Z);
        QTest::qWait(50); QVERIFY(find(w, "colorHexField")->hasActiveFocus());
        QVERIFY(!find(w, "useColorButton")->isEnabled());
        QVERIFY(click(w, "recentColor0"));
        QCOMPARE(wheel->hex(), QString("#33669980")); QVERIFY(find(w, "useColorButton")->isEnabled());
        QVERIFY(snapshot(w, "color-picker"));
        w->resize(800,520); QTest::qWait(30);
        QVERIFY(snapshot(w, "color-picker-compact"));
        QTest::keyClick(w, Qt::Key_Escape);
        QTRY_VERIFY(!popup->property("visible").toBool());
    }
    void unifiedExitDialogAndWindowControls() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json")); IconService icons;
        SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
        auto* w = editor.window(); QVERIFY(w); QVERIFY(showEditor(editor));
        QVERIFY(w->flags() & Qt::FramelessWindowHint);
        const auto normalSize = w->size();
        QVERIFY(click(w, "windowMaximizeButton")); QTRY_VERIFY(editor.maximized());
        QCOMPARE(w->geometry(), w->screen()->availableGeometry());
        QCOMPARE(find(w, "windowMaximizeButton")->property("hint").toString(), QString("Restore"));
        QVERIFY(click(w, "windowMaximizeButton")); QTRY_COMPARE(w->visibility(), QWindow::Windowed);
        QVERIFY(!editor.maximized());
        QTRY_COMPARE(w->size(), normalSize);
        QVERIFY(click(w, "menuEnabledToggle")); QVERIFY(!editor.session()->profile()["enabled"].toBool());
        QVERIFY(click(w, "menuEnabledToggle")); QVERIFY(editor.session()->profile()["enabled"].toBool());
        QSignalSpy exit(&editor, &SettingsWindow::exitConfirmed);
        editor.session()->setProfileField("name", "Unsaved draft"); editor.requestExit();
        auto* popup = w->findChild<QObject*>("exitDialog"); QVERIFY(popup);
        QTRY_VERIFY(popup->property("visible").toBool());
        QVERIFY(snapshot(w, "unsaved-dialog"));
        QVERIFY(click(w, "exitCancelButton")); QVERIFY(editor.session()->dirty()); QCOMPARE(exit.count(), 0);
        editor.session()->addItem(int(ActionType::SendHotkey));
        editor.session()->setItemField("target", "Ctrl+UnknownKey");
        editor.requestExit(); QVERIFY(click(w, "exitApplyButton"));
        QVERIFY(popup->property("visible").toBool()); QVERIFY(!editor.session()->error().isEmpty()); QCOMPARE(exit.count(), 0);
        editor.session()->setItemField("target", "Ctrl+Q");
        QVERIFY(click(w, "exitApplyButton")); QCOMPARE(exit.count(), 1); QVERIFY(!editor.session()->dirty());
        editor.session()->setProfileField("name", "Discard this"); editor.requestExit();
        QVERIFY(click(w, "exitDiscardButton")); QCOMPARE(exit.count(), 2);
        QCOMPARE(editor.session()->profile()["name"].toString(), QString("Unsaved draft"));
        editor.pickRunningApplication();
        auto* apps = w->findChild<QObject*>("runningAppsDialog"); QVERIFY(apps);
        QTRY_VERIFY(apps->property("visible").toBool()); QVERIFY(find(w, "runningAppsSearch"));
        QVERIFY(snapshot(w, "running-apps")); QTest::keyClick(w, Qt::Key_Escape);
        QVERIFY(click(w, "windowCloseButton")); QTRY_VERIFY(!w->isVisible());
    }
    void runtimeRapidReopenAndLongList() {
        IconService icons; PieMenuWidget pie(&icons);
        auto config = AppConfig::CreateDefault(); config.GlobalStyle.AnimationDuration = 0;
        const auto area = QGuiApplication::primaryScreen()->availableGeometry();
        const auto origin = area.center();
        QSignalSpy chosen(&pie, &PieMenuWidget::ItemSelected);
        for (int i = 0; i < 20; ++i) {
            pie.ShowAt(origin, config.Profiles[0], config.GlobalStyle);
            pie.UpdateMousePos(origin - QPoint(0, 100));
            QCOMPARE(pie.ConfirmSelection(), 0);
            QCOMPARE(pie.ConfirmSelection(), -1);
        }
        QCOMPARE(chosen.count(), 20);
        pie.ShowAt(origin, config.Profiles[0], config.GlobalStyle);
        QTest::qWait(100); QVERIFY(pie.IsOpen()); QVERIFY(pie.isVisible()); pie.HideMenu();
        std::vector<PieItem> items;
        for (int i = 0; i < 100; ++i) items.push_back(PieItem::Create(QString::number(i), ActionType::SendHotkey, "Ctrl+C"));
        ListMenuWidget list(&icons); QSignalSpy listChosen(&list, &ListMenuWidget::ItemSelected);
        list.ShowAt(area.bottomRight(), items, config.GlobalStyle);
        QVERIFY(area.contains(list.geometry()));
        list.UpdateMousePos(list.pos() + QPoint(30, 36));
        for (int i = 0; i < 100; ++i) list.Scroll(-120);
        list.UpdateMousePos(list.pos() + QPoint(80, list.height() - 40));
        QCOMPARE(list.GetHoveredIndex(), 99);
        list.ConfirmSelection(); list.ConfirmSelection(); QCOMPARE(listChosen.count(), 1);
    }
    void iconIndexAndCachePerformance() {
        QTemporaryDir dir; IconService service(dir.path());
        IconCatalog catalog(&service);
        QElapsedTimer timer; timer.start();
        QTRY_VERIFY_WITH_TIMEOUT(catalog.rowCount() > 1000, 3000);
        qInfo() << "Cold metadata index:" << catalog.rowCount() << "icons," << timer.elapsed() << "ms";
        timer.restart(); catalog.setQuery("folder");
        qInfo() << "Index filter:" << timer.nsecsElapsed() / 1e6 << "ms";
        QVERIFY(catalog.rowCount() > 0);
        auto backend = service.backend(); timer.restart();
        const auto cold = backend->load("folder.svg", 64, QColor("#324566"));
        QVERIFY(!cold.isNull());
        timer.restart();
        for (int i = 0; i < 100; ++i) QCOMPARE(backend->load("folder.svg", 64, QColor("#324566")), cold);
        qInfo() << "Warm thumbnail average:" << timer.nsecsElapsed() / 1e8 << "ms";
        QVERIFY(backend->load("folder.svg", 64, Qt::red) != cold);
        QCOMPARE(backend->load("folder.svg", 128, Qt::red).size(), QSize(128,128));
    }
    void closeDuringColdIconLoading() {
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json"));
        IconService icons;
        for (int pass = 0; pass < 5; ++pass) {
            icons.invalidate();
            SettingsWindow editor(&config, &icons); editor.setPlatformIntegrationEnabled(false);
            auto* w = editor.window(); QVERIFY(w);
            QVERIFY(showEditor(editor));
            editor.session()->selectItem(editor.session()->items()[0].toMap()["id"].toString());
            QTest::qWait(20);
            auto* picker = w->findChild<QObject*>("iconPicker"); QVERIFY(picker);
            auto* catalog = qvariant_cast<QObject*>(qmlContext(w)->contextProperty("iconCatalog"));
            QTRY_VERIFY(catalog->property("count").toInt() > 1000);
            for (int i = 0; i < 12; ++i) {
                QVERIFY(QMetaObject::invokeMethod(picker, "open"));
                QCoreApplication::processEvents();
                catalog->setProperty("query", i % 2 ? "folder" : "");
                QTest::qWait(1);
                QVERIFY(QMetaObject::invokeMethod(picker, "close"));
            }
            // Destroy the engine with asynchronous responses still in flight.
        }
    }
};
QTEST_MAIN(UiTests)
#include "UiTests.moc"
