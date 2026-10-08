#include "SettingsWindow.h"
#include "quick/PiePreviewItem.h"
#include "widgets/WindowPickerDialog.h"
#include <QQmlContext>
#include <QQuickStyle>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QSaveFile>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QSettings>
#include <QUrl>
#include <QScreen>
#include <QGuiApplication>
#include <QCursor>

static void InitializeEditorResources() { Q_INIT_RESOURCE(editor); }

namespace gpm {
SettingsWindow::SettingsWindow(ConfigManager* manager, IconService* icons, QObject* parent)
    : QObject(parent), Manager(manager), Icons(icons), Session(manager), Recorder(&Session), Catalog(icons) {
    InitializeEditorResources();
    static const bool registered = [] {
        qmlRegisterType<PiePreviewItem>("GoPieMenu", 1, 0, "PiePreview");
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
bool SettingsWindow::requestExit() {
    if (!Session.dirty()) return true;
    show();
    const auto choice = QMessageBox::question(nullptr, tr("Unsaved changes"), tr("Apply your changes before quitting?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if (choice == QMessageBox::Cancel) return false;
    if (choice == QMessageBox::Discard) return true;
    return Session.apply();
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
    WindowPickerDialog dialog;
    if (dialog.exec() == QDialog::Accepted) addAppFilter(dialog.GetSelectedProcessName());
}
void SettingsWindow::chooseColor(const QString& field, bool item) {
    const auto initial = item ? Session.selectedItem()["color"].toString() : Session.style()[field].toString();
    const auto color = QColorDialog::getColor(QColor(initial), nullptr, tr("Choose color"), QColorDialog::ShowAlphaChannel);
    if (!color.isValid()) return;
    if (item) Session.setItemField("color", color.name(QColor::HexArgb));
    else Session.setStyleField(field, color.name(QColor::HexArgb));
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
