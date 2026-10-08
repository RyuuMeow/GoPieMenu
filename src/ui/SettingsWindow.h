#pragma once
#include <QObject>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QPointer>
#include "core/EditorSession.h"
#include "icons/IconCatalog.h"
#include "InputRecorder.h"

namespace gpm {
class SettingsWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString iconDirectory READ iconDirectory CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(bool previewMode READ previewMode WRITE setPreviewMode NOTIFY previewModeChanged)
    Q_PROPERTY(QStringList recentColors READ recentColors NOTIFY recentColorsChanged)
    Q_PROPERTY(QStringList runningApplications READ runningApplications NOTIFY runningApplicationsChanged)
    Q_PROPERTY(bool maximized READ maximized NOTIFY windowStateChanged)
public:
    explicit SettingsWindow(ConfigManager* manager, IconService* icons, QObject* parent = nullptr);
    ~SettingsWindow() override;
    void show();
    void raise();
    void activateWindow();
    Q_INVOKABLE void requestExit();
    Q_INVOKABLE bool resolveExit(const QString& choice);
    Q_INVOKABLE void toggleMaximized();
    bool maximized() const;
    QQuickWindow* window() const { return Window; }
    EditorSession* session() { return &Session; }
    void setPlatformIntegrationEnabled(bool enabled) { PlatformIntegration = enabled; }
    bool previewMode() const { return PreviewMode; }
    void setPreviewMode(bool enabled) { PreviewMode = enabled; emit previewModeChanged(); }
    QString iconDirectory() const { return Icons->directory(); }
    QString version() const { return QStringLiteral(APP_VERSION); }
    QStringList recentColors() const { return RecentColors; }
    QStringList runningApplications() const { return Applications; }
    Q_INVOKABLE void chooseTarget(bool directory = false);
    Q_INVOKABLE void chooseApplicationFilter();
    Q_INVOKABLE void pickRunningApplication();
    Q_INVOKABLE void refreshRunningApplications();
    Q_INVOKABLE void useRunningApplication(const QString& name);
    Q_INVOKABLE void chooseColor(const QString& field, bool item = false);
    Q_INVOKABLE void acceptColor(const QColor& color);
    Q_INVOKABLE void importConfig();
    Q_INVOKABLE void exportConfig();
    Q_INVOKABLE void openIconDirectory();
    Q_INVOKABLE void addDroppedFiles(const QStringList& urls);
signals:
    void previewModeChanged();
    void ConfigUpdated();
    void RecordingChanged(bool recording);
    void colorRequested(const QColor& color);
    void confirmExitRequested();
    void exitConfirmed();
    void runningAppsRequested();
    void recentColorsChanged();
    void runningApplicationsChanged();
    void windowStateChanged();
private:
    void addAppFilter(const QString& name);
    void installAutoStart();
    ConfigManager* Manager;
    IconService* Icons;
    EditorSession Session;
    InputRecorder Recorder;
    IconCatalog Catalog;
    QQmlApplicationEngine Engine;
    QPointer<QQuickWindow> Window;
    bool PlatformIntegration = true;
    bool PreviewMode = false;
    bool FirstShow = true;
    QString ColorField, ColorProfile, ColorItem;
    QStringList RecentColors, Applications;
};
}
