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
public:
    explicit SettingsWindow(ConfigManager* manager, IconService* icons, QObject* parent = nullptr);
    ~SettingsWindow() override;
    void show();
    void raise();
    void activateWindow();
    bool requestExit();
    QQuickWindow* window() const { return Window; }
    EditorSession* session() { return &Session; }
    void setPlatformIntegrationEnabled(bool enabled) { PlatformIntegration = enabled; }
    bool previewMode() const { return PreviewMode; }
    void setPreviewMode(bool enabled) { PreviewMode = enabled; emit previewModeChanged(); }
    QString iconDirectory() const { return Icons->directory(); }
    QString version() const { return QStringLiteral(APP_VERSION); }
    Q_INVOKABLE void chooseTarget(bool directory = false);
    Q_INVOKABLE void chooseApplicationFilter();
    Q_INVOKABLE void pickRunningApplication();
    Q_INVOKABLE void chooseColor(const QString& field, bool item = false);
    Q_INVOKABLE void importConfig();
    Q_INVOKABLE void exportConfig();
    Q_INVOKABLE void openIconDirectory();
    Q_INVOKABLE void addDroppedFiles(const QStringList& urls);
signals:
    void previewModeChanged();
    void ConfigUpdated();
    void RecordingChanged(bool recording);
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
};
}
