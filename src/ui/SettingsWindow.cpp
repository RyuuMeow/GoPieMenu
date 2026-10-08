#include "SettingsWindow.h"
#include "quick/PiePreviewItem.h"
#include "quick/ColorWheelItem.h"
#include "RunningApplications.h"
#include <QQmlContext>
#include <QQuickStyle>
#include <QFileDialog>
#include <QSaveFile>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QSettings>
#include <QUrl>
#include <QScreen>
#include <QGuiApplication>
#include <QCursor>
#include <QStyleHints>
#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#endif

static void InitializeEditorResources() { Q_INIT_RESOURCE(editor); }

namespace gpm {
SettingsWindow::SettingsWindow(ConfigManager* manager, IconService* icons, QObject* parent)
    : QObject(parent), Manager(manager), Icons(icons), Session(manager), Recorder(&Session), Catalog(icons) {
    InitializeEditorResources();
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
    for (const auto& value : QSettings().value("editor/recentColors").toStringList()) {
        if (QColor(value).isValid() && !RecentColors.contains(value)) RecentColors.append(value);
        if (RecentColors.size() == 12) break;
    }
    static const bool registered = [] {
        qmlRegisterType<PiePreviewItem>("GoPieMenu", 1, 0, "PiePreview");
        qmlRegisterType<ColorWheelItem>("GoPieMenu", 1, 0, "ColorWheel");
        qmlRegisterUncreatableType<EditorSession>("GoPieMenu", 1, 0, "EditorSession", "Provided by the application");
        qmlRegisterUncreatableType<IconService>("GoPieMenu", 1, 0, "IconService", "Provided by the application");
        return true;
    }();
    Q_UNUSED(registered);
    Engine.addImageProvider("icons", Icons->createProvider());
    Engine.rootContext()->setContextProperty("editor", &Session);
    Engine.rootContext()->setContextProperty("iconService", Icons);
    Engine.rootContext()->setContextProperty("iconCatalog", &Catalog);
    Engine.rootContext()->setContextProperty("recorder", &Recorder);
    Engine.rootContext()->setContextProperty("appController", this);
    connect(&Session, &EditorSession::applied, this, [this] { installAutoStart(); emit ConfigUpdated(); });
    connect(&Recorder, &InputRecorder::changed, this, [this] { emit RecordingChanged(Recorder.active()); });
    Engine.load(QUrl("qrc:/editor/Main.qml"));
    if (!Engine.rootObjects().isEmpty()) Window = qobject_cast<QQuickWindow*>(Engine.rootObjects().first());
    if (Window) connect(Window, &QWindow::windowStateChanged, this, &SettingsWindow::windowStateChanged);
#ifdef Q_OS_WIN
    if (Window) {
        const auto handle = reinterpret_cast<HWND>(Window->winId());
        const DWM_WINDOW_CORNER_PREFERENCE corners = DWMWCP_ROUND;
        DwmSetWindowAttribute(handle, DWMWA_WINDOW_CORNER_PREFERENCE, &corners, sizeof(corners));
        const BOOL dark = FALSE;
        DwmSetWindowAttribute(handle, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
        const COLORREF border = RGB(226, 231, 239);
        DwmSetWindowAttribute(handle, DWMWA_BORDER_COLOR, &border, sizeof(border));
    }
#endif
}
SettingsWindow::~SettingsWindow() = default;
void SettingsWindow::show() {
    if (!Window) return;
    bool onScreen = false;
    for (auto* screen : QGuiApplication::screens())
        onScreen |= screen->availableGeometry().intersects(Window->geometry());
    if (FirstShow || !onScreen) {
        auto* screen = QGuiApplication::screenAt(QCursor::pos());
        if (!screen) screen = QGuiApplication::primaryScreen();
        if (screen) {
            const auto area = screen->availableGeometry();
            Window->setScreen(screen);
            const QSize size(std::min(Window->width(), area.width() - 24),
                             std::min(Window->height(), area.height() - 48));
            Window->setGeometry(QRect(area.center() - QPoint(size.width()/2, size.height()/2), size));
        }
    }
    FirstShow = false;
    Window->show(); Window->raise(); Window->requestActivate();
}
void SettingsWindow::raise() { if (Window) Window->raise(); }
void SettingsWindow::activateWindow() { if (Window) Window->requestActivate(); }
bool SettingsWindow::maximized() const {
    if (!Window) return false;
#ifdef Q_OS_WIN
    return IsZoomed(reinterpret_cast<HWND>(Window->winId()));
#else
    return Window->windowState() == Qt::WindowMaximized;
#endif
}
void SettingsWindow::toggleMaximized() {
    if (!Window) return;
#ifdef Q_OS_WIN
    // Qt 6.9 emulates frameless maximization with MoveWindow and can report
    // FullScreen on a monitor without a taskbar. Preserve the native window state.
    const auto handle = reinterpret_cast<HWND>(Window->winId());
    ShowWindow(handle, IsZoomed(handle) ? SW_RESTORE : SW_MAXIMIZE);
#else
    if (Window->windowState() == Qt::WindowMaximized) Window->showNormal();
    else Window->showMaximized();
#endif
    emit windowStateChanged();
}
void SettingsWindow::requestExit() {
    if (!Session.dirty()) { emit exitConfirmed(); return; }
    show();
    emit confirmExitRequested();
}
bool SettingsWindow::resolveExit(const QString& choice) {
    if (choice == "apply") { if (!Session.apply()) return false; }
    else if (choice == "discard") Session.discard();
    else return false;
    emit exitConfirmed(); return true;
}
void SettingsWindow::chooseTarget(bool directory) {
    const auto type = static_cast<ActionType>(Session.selectedItem()["action"].toInt());
    QString path;
    if (directory) path = QFileDialog::getExistingDirectory(nullptr, tr("Choose folder"));
    else path = QFileDialog::getOpenFileName(nullptr, type == ActionType::LaunchApp ? tr("Choose application") : tr("Choose file"),
        {}, type == ActionType::LaunchApp ? tr("Applications (*.exe *.bat *.cmd);;All files (*)") : tr("All files (*)"));
    if (!path.isEmpty()) Session.setItemField("target", path);
}
void SettingsWindow::addAppFilter(const QString& name) {
    auto values = Session.profile()["appFilter"].toString().split(',', Qt::SkipEmptyParts);
    for (auto& value : values) value = value.trimmed();
    if (!name.isEmpty() && !values.contains(name, Qt::CaseInsensitive)) values.append(name);
    Session.setProfileField("appFilter", values.join(", "));
}
void SettingsWindow::chooseApplicationFilter() {
    const auto path = QFileDialog::getOpenFileName(nullptr, tr("Choose application"), {}, tr("Applications (*.exe)"));
    if (!path.isEmpty()) addAppFilter(QFileInfo(path).fileName());
}
void SettingsWindow::pickRunningApplication() {
    refreshRunningApplications(); emit runningAppsRequested();
}
void SettingsWindow::refreshRunningApplications() {
    Applications = RunningApplications(); emit runningApplicationsChanged();
}
void SettingsWindow::useRunningApplication(const QString& name) {
    if (Applications.contains(name, Qt::CaseInsensitive)) addAppFilter(name);
}
void SettingsWindow::chooseColor(const QString& field, bool item) {
    const auto initial = item ? Session.selectedItem()["color"].toString() : Session.style()[field].toString();
    ColorField = field; ColorProfile = Session.profileId(); ColorItem = item ? Session.selectedId() : QString();
    auto color = QColor(initial);
    if (!color.isValid()) color = Session.effectiveStyle().SectorColor;
    emit colorRequested(color);
}
void SettingsWindow::acceptColor(const QColor& color) {
    if (!color.isValid() || ColorField.isEmpty()) return;
    if (ColorProfile != Session.profileId() || (!ColorItem.isEmpty() && ColorItem != Session.selectedId())) {
        Session.reportError(tr("The selection changed. Choose the color again.")); return;
    }
    const auto value = color.name(QColor::HexArgb);
    if (!ColorItem.isEmpty()) Session.setItemField(ColorField, value);
    else Session.setStyleField(ColorField, value);
    ColorField.clear();
    RecentColors.removeAll(value); RecentColors.prepend(value);
    while (RecentColors.size() > 12) RecentColors.removeLast();
    if (PlatformIntegration) {
        QSettings settings;
        settings.setValue("editor/recentColors", RecentColors); settings.sync();
        if (settings.status() != QSettings::NoError) Session.reportError(tr("Color updated, but recent colors could not be saved."));
    }
    emit recentColorsChanged();
}
void SettingsWindow::importConfig() {
    const auto path = QFileDialog::getOpenFileName(nullptr, tr("Import configuration"), {}, tr("JSON files (*.json)"));
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { Session.reportError(file.errorString()); return; }
    // Import is staged and undoable. Only Apply can replace the live configuration.
    Session.importData(file.readAll());
}
void SettingsWindow::exportConfig() {
    const auto error = ValidateConfig(Session.draft());
    if (!error.isEmpty()) { Session.reportError(error); return; }
    const auto path = QFileDialog::getSaveFileName(nullptr, tr("Export configuration"), "GoPieMenu.json", tr("JSON files (*.json)"));
    if (path.isEmpty()) return;
    QSaveFile file(path); file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly)) { Session.reportError(file.errorString()); return; }
    const auto data = Session.draft().Serialize();
    if (file.write(data) != data.size() || !file.commit()) Session.reportError(file.errorString());
}
void SettingsWindow::openIconDirectory() {
    if (!QDir().mkpath(Icons->directory()) || !QDesktopServices::openUrl(QUrl::fromLocalFile(Icons->directory())))
        Session.reportError(tr("Cannot open the icon folder."));
    Catalog.refresh();
}
void SettingsWindow::addDroppedFiles(const QStringList& urls) {
    for (const auto& value : urls) {
        const QUrl url(value);
        const auto path = url.isLocalFile() ? url.toLocalFile() : value;
        const auto file = QFileInfo(path);
        const auto type = !url.isLocalFile() && !url.scheme().isEmpty() ? ActionType::OpenURL :
            (file.suffix().compare("exe", Qt::CaseInsensitive) == 0 ? ActionType::LaunchApp : ActionType::OpenFile);
        if (Session.addItem(int(type)).isEmpty()) break;
        Session.setItemField("target", path);
    }
}
void SettingsWindow::installAutoStart() {
    if (!PlatformIntegration) return;
#ifdef Q_OS_WIN
    QSettings registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (Manager->GetConfig().bStartWithWindows)
        registry.setValue("GoPieMenu", "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\"");
    else registry.remove("GoPieMenu");
    registry.sync();
    if (registry.status() != QSettings::NoError) Session.reportError(tr("Settings were saved, but Windows startup could not be updated."));
#endif
}
}
