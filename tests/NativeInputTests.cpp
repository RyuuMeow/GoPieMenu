#include <QtTest>
#include <QProcess>
#include <QScreen>
#include <QTemporaryDir>
#include <QPushButton>
#include "core/HookManager.h"
#include "core/ActionExecutor.h"
#include "core/MenuPolicy.h"
#include "ui/RuntimeMenuController.h"

using namespace gpm;
namespace {
constexpr UINT KeyCount = WM_APP + 1;
bool moveMouse(HWND window, const QPoint& local);
bool mouseButton(bool down, bool right = false);
bool activateTarget(HWND window) {
    SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    const bool moved = moveMouse(window, QPoint(30, 30)); QTest::qWait(50);
    POINT point{}; GetCursorPos(&point);
    const bool hit = moved && WindowFromPoint(point) == window;
    if (hit) { mouseButton(true); QTest::qWait(50); mouseButton(false); }
    const bool active = hit && QTest::qWaitFor([window] { return GetForegroundWindow() == window; });
    SetWindowPos(window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    return active;
}
struct TargetProcess {
    QProcess process;
    HWND window = nullptr;
    POINT cursor{};
    TargetProcess() { GetCursorPos(&cursor); }
    ~TargetProcess() {
        if (window) PostMessageW(window, WM_CLOSE, 0, 0);
        if (!process.waitForFinished(3000)) { process.kill(); process.waitForFinished(); }
        SetCursorPos(cursor.x, cursor.y);
    }
    bool start() {
        process.start(QCoreApplication::applicationDirPath() + "/gpm_input_target.exe", QStringList{});
        if (!process.waitForStarted()) return false;
        if (!QTest::qWaitFor([this] {
            window = FindWindowW(L"GoPieMenuInputTestTarget", nullptr);
            DWORD pid = 0; GetWindowThreadProcessId(window, &pid);
            return window && pid == process.processId() && IsWindowVisible(window);
        })) return false;
        return activateTarget(window);
    }
};
bool moveMouse(HWND window, const QPoint& local) {
    POINT point{local.x(), local.y()};
    ClientToScreen(window, &point);
    INPUT input{}; input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
    input.mi.dx = MulDiv(point.x - GetSystemMetrics(SM_XVIRTUALSCREEN), 65535, GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1);
    input.mi.dy = MulDiv(point.y - GetSystemMetrics(SM_YVIRTUALSCREEN), 65535, GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1);
    return SendInput(1, &input, sizeof(INPUT)) == 1;
}
bool moveMouse(QWidget* widget, const QPoint& local) {
    return moveMouse(reinterpret_cast<HWND>(widget->winId()),
        QPoint(qRound(local.x() * widget->devicePixelRatioF()), qRound(local.y() * widget->devicePixelRatioF())));
}
bool mouseButton(bool down, bool right) {
    INPUT input{}; input.type = INPUT_MOUSE;
    input.mi.dwFlags = right ? (down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP) :
        (down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP);
    return SendInput(1, &input, sizeof(INPUT)) == 1;
}
bool clickMouse(QWidget* widget, const QPoint& local, bool right = false) {
    if (!moveMouse(widget, local)) return false;
    POINT point{};
    // Layered windows become hit-testable after their backing surface is painted.
    if (!QTest::qWaitFor([&] {
        GetCursorPos(&point);
        return GetAncestor(WindowFromPoint(point), GA_ROOT) == reinterpret_cast<HWND>(widget->window()->winId());
    }, 1000)) {
        qWarning() << "Native click missed" << local << QPoint(point.x, point.y) << "hit" << WindowFromPoint(point)
            << "expected" << reinterpret_cast<HWND>(widget->window()->winId()) << "visible" << widget->isVisible()
            << "DPR" << widget->devicePixelRatioF() << "Qt cursor" << QCursor::pos();
        return false;
    }
    if (!mouseButton(true, right)) return false;
    QTest::qWait(50);
    if (!mouseButton(false, right)) return false;
    QTest::qWait(100);
    return true;
}
}
class NativeInputTests : public QObject {
    Q_OBJECT
private slots:
    void persistentNativeClicksKeepTarget() {
        TargetProcess target; QVERIFY(target.start());
        QTemporaryDir dir; ConfigManager config(nullptr, dir.filePath("config.json"));
        auto data = config.GetConfig(); data.GlobalStyle.AnimationDuration = 0;
        auto profile = PieMenuConfig::CreateDefault(); profile.Persistent = true;
        profile.AppFilter = {"gpm_input_target.exe"};
        profile.Items[0].Action = ActionType::ListMenu;
        profile.Items[0].SubItems = {PieItem::Create("Test key", ActionType::SendHotkey, "Ctrl+F24")};
        profile.Items[1].Action = ActionType::SendHotkey; profile.Items[1].ActionData = "Ctrl+F24";
        data.Profiles = {profile}; QVERIFY(config.Commit(data));
        IconService icons; PieMenuWidget pie(&icons); RuntimeMenuController runtime(&config, &pie);
        HookManager hooks(&config); ActionExecutor executor; executor.RegisterBuiltins();
        connect(&hooks, &HookManager::ForegroundChanged, &runtime, &RuntimeMenuController::SetForegroundProcess);
        connect(&hooks, &HookManager::PointerPressed, &pie, &PieMenuWidget::DismissContextOutside);
        connect(&pie, &PieMenuWidget::ItemSelected, &executor, [&](int, const PieItem& item) {
            const auto* eligible = PersistentMenu(config.GetConfig().Profiles, hooks.foregroundProcess());
            if (eligible && eligible->Id == runtime.CurrentProfile()) executor.Execute(item);
        });
        QVERIFY(hooks.Install());
        QVERIFY(QTest::qWaitForWindowExposed(&pie)); QTest::qWait(100);
        QCOMPARE(GetForegroundWindow(), target.window);
        auto center = pie.mapFromGlobal(pie.MenuCenter());
        QVERIFY(clickMouse(&pie, center + QPoint(90, 0)));
        QCOMPARE(GetForegroundWindow(), target.window);
        QTRY_COMPARE(SendMessageW(target.window, KeyCount, 0, 0), LRESULT(1));
        QVERIFY(pie.IsOpen());
        QVERIFY(clickMouse(&pie, center - QPoint(0, 90)));
        auto* list = pie.findChild<ListMenuWidget*>("runtimeSubmenu"); QVERIFY(list && list->IsOpen());
        QVERIFY(clickMouse(list, QPoint(50, 35)));
        QCOMPARE(GetForegroundWindow(), target.window);
        QTRY_COMPARE(SendMessageW(target.window, KeyCount, 0, 0), LRESULT(2));
        QVERIFY(clickMouse(list, QPoint(50, 35)));
        QTRY_COMPARE(SendMessageW(target.window, KeyCount, 0, 0), LRESULT(3));
        QVERIFY(moveMouse(&pie, QPoint(-60, -60))); QTest::qWait(300);
        QVERIFY(pie.IsOpen()); QVERIFY(list->IsOpen());
        QCOMPARE(GetForegroundWindow(), target.window);

        const auto oldCenter = pie.MenuCenter();
        QSignalSpy moved(&pie, &PieMenuWidget::PositionChanged);
        QVERIFY(moveMouse(&pie, center)); QTest::qWait(50);
        QVERIFY(mouseButton(true)); QTest::qWait(50);
        QVERIFY(moveMouse(&pie, center + QPoint(40, 25))); QTest::qWait(100);
        QVERIFY(mouseButton(false)); QTest::qWait(100);
        QCOMPARE(moved.count(), 1); QVERIFY(pie.MenuCenter() != oldCenter);
        QCOMPARE(GetForegroundWindow(), target.window);
        QCOMPARE(SendMessageW(target.window, KeyCount, 0, 0), LRESULT(3));
        QVERIFY(list->IsOpen());

        center = pie.mapFromGlobal(pie.MenuCenter());
        QVERIFY(clickMouse(&pie, center, true));
        auto* context = pie.findChild<QWidget*>("persistentContextMenu"); QVERIFY(context && context->isVisible());
        auto* hide = pie.findChild<QPushButton*>("hidePersistentButton"); QVERIFY(hide);
        QVERIFY(clickMouse(context, hide->geometry().center()));
        QTRY_VERIFY(runtime.IsHidden()); QVERIFY(!pie.IsOpen());
        QCOMPARE(GetForegroundWindow(), target.window);
        runtime.SetHidden(false);
        QVERIFY(pie.IsOpen()); QVERIFY(list->IsOpen());

        // A genuine switch to a different process hides the scoped menu, and
        // returning restores its position/submenu without changing focus.
        QWidget other; other.setGeometry(50, 50, 300, 200); other.show();
        QVERIFY(QTest::qWaitForWindowExposed(&other));
        QVERIFY(activateTarget(reinterpret_cast<HWND>(other.winId())));
        QTRY_VERIFY(!pie.IsOpen());
        QVERIFY(activateTarget(target.window));
        QTRY_VERIFY(pie.IsOpen()); QTRY_VERIFY(list->IsOpen());
        QCOMPARE(GetForegroundWindow(), target.window);
        QVERIFY(clickMouse(list, QPoint(50, 35)));
        QTRY_COMPARE(SendMessageW(target.window, KeyCount, 0, 0), LRESULT(4));
    }
};
QTEST_MAIN(NativeInputTests)
#include "NativeInputTests.moc"
