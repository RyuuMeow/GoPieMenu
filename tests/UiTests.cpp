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
#include <cmath>
#include "ui/SettingsWindow.h"
#include "ui/quick/PiePreviewItem.h"
#include "ui/quick/ColorWheelItem.h"
#include "ui/PieMenuWidget.h"
#include "ui/rendering/MenuScene.h"

using namespace gpm;
class UiTests : public QObject {
    Q_OBJECT
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
        QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier, item->mapToScene(QPointF(item->width()/2, item->height()/2)).toPoint());
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
    void initTestCase() {
        QQuickStyle::setStyle("Basic");
        for (auto* screen : QGuiApplication::screens()) qInfo() << "Screen" << screen->name() << screen->geometry() << screen->availableGeometry() << screen->devicePixelRatio();
    }
    void init() { QTest::failOnWarning(QRegularExpression(".*(qrc:/editor|QQml|Binding loop|ReferenceError|TypeError).*")); }
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
        editor.show();
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
        QVERIFY(editor.window()); editor.show(); QVERIFY(QTest::qWaitForWindowExposed(editor.window()));
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
        auto* w = editor.window(); QVERIFY(w); editor.show(); QVERIFY(QTest::qWaitForWindowExposed(w));
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
        auto* w = editor.window(); QVERIFY(w); editor.show(); QVERIFY(QTest::qWaitForWindowExposed(w));
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
        auto* w = editor.window(); QVERIFY(w); editor.show(); QVERIFY(QTest::qWaitForWindowExposed(w));
        auto* session = editor.session();
        const auto folder = session->addItem(int(ActionType::ListMenu)); session->enterFolder(folder);
        QStringList ids;
        for (int i = 0; i < 3; ++i) { ids.append(session->addItem(3)); session->setItemField("name", QString("Action %1").arg(i)); }
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
        session->undo(); QTest::qWait(30);
        // Begin over the label, then insert after the last row.
        row = find(w, "arrange-row-" + ids[0]); QVERIFY(row);
        from = row->mapToScene(QPointF(105,20)).toPoint();
        to = viewport->mapToScene(QPointF(120, viewport->height()-2)).toPoint();
        QTest::mousePress(w, Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(w, from + QPoint(0,20), 20); QTest::mouseMove(w, to, 30);
        QTRY_VERIFY(find(w, "insertionLine"));
        QTest::mouseRelease(w, Qt::LeftButton, Qt::NoModifier, to);
        QTRY_COMPARE(session->items()[2].toMap()["id"].toString(), ids[0]);
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
        auto* w = editor.window(); QVERIFY(w); editor.show(); QVERIFY(QTest::qWaitForWindowExposed(w));
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
        auto* w = editor.window(); QVERIFY(w); editor.show(); QVERIFY(QTest::qWaitForWindowExposed(w));
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
        auto* w = editor.window(); QVERIFY(w); editor.show(); QVERIFY(QTest::qWaitForWindowExposed(w));
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
        auto* w = editor.window(); QVERIFY(w); editor.show(); QVERIFY(QTest::qWaitForWindowExposed(w));
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
            editor.show(); QVERIFY(QTest::qWaitForWindowExposed(w));
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
