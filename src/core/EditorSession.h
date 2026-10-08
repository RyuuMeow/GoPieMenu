#pragma once

#include "ConfigManager.h"
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QElapsedTimer>
#include <deque>
#include <functional>

namespace gpm {

// The UI only edits this draft. Runtime consumers only see ConfigManager's committed snapshot.
class EditorSession : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList profiles READ profiles NOTIFY changed)
    Q_PROPERTY(QString profileId READ profileId NOTIFY changed)
    Q_PROPERTY(QVariantMap profile READ profile NOTIFY changed)
    Q_PROPERTY(QVariantList items READ items NOTIFY changed)
    Q_PROPERTY(QVariantList destinations READ destinations NOTIFY changed)
    Q_PROPERTY(QVariantMap selectedItem READ selectedItem NOTIFY changed)
    Q_PROPERTY(QString selectedId READ selectedId NOTIFY changed)
    Q_PROPERTY(QString folderId READ folderId NOTIFY changed)
    Q_PROPERTY(QString folderName READ folderName NOTIFY changed)
    Q_PROPERTY(QVariantMap style READ style NOTIFY changed)
    Q_PROPERTY(bool dirty READ dirty NOTIFY changed)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY changed)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY changed)
    Q_PROPERTY(bool startWithWindows READ startWithWindows NOTIFY changed)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
public:
    explicit EditorSession(ConfigManager* manager, QObject* parent = nullptr);
    QVariantList profiles() const;
    QVariantMap profile() const;
    QVariantList items() const;
    QVariantList destinations() const;
    QVariantMap selectedItem() const;
    QVariantMap style() const;
    QString profileId() const { return CurrentProfile; }
    QString selectedId() const { return Selected; }
    QString folderId() const { return Folder; }
    QString folderName() const;
    bool dirty() const { return IsDirty; }
    bool canUndo() const { return !Undo.empty(); }
    bool canRedo() const { return !Redo.empty(); }
    bool startWithWindows() const { return Draft.bStartWithWindows; }
    QString error() const { return Error; }
    const AppConfig& draft() const { return Draft; }
    const PieMenuConfig* currentProfile() const;
    const PieItem* findItem(const QString& id) const;
    StyleConfig effectiveStyle() const;

    Q_INVOKABLE void selectProfile(const QString& id);
    Q_INVOKABLE void selectItem(const QString& id);
    Q_INVOKABLE void enterFolder(const QString& id);
    Q_INVOKABLE void leaveFolder();
    Q_INVOKABLE void addProfile();
    Q_INVOKABLE void duplicateProfile();
    Q_INVOKABLE void removeProfile();
    Q_INVOKABLE void setProfileField(const QString& field, const QVariant& value);
    Q_INVOKABLE void setTrigger(int mode, int modifiers, const QString& button, int vkCode);
    Q_INVOKABLE QString addItem(int action = 1);
    Q_INVOKABLE void removeItem(const QString& id);
    Q_INVOKABLE void duplicateItem(const QString& id);
    Q_INVOKABLE void setItemField(const QString& field, const QVariant& value);
    Q_INVOKABLE bool moveItem(const QString& id, const QString& destinationFolder, int index);
    Q_INVOKABLE void moveSelected(int delta);
    Q_INVOKABLE void setStyleField(const QString& field, const QVariant& value);
    Q_INVOKABLE void setStylePreset(const QString& name);
    Q_INVOKABLE void setStartWithWindows(bool enabled);
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE bool apply();
    Q_INVOKABLE void discard();
    Q_INVOKABLE void clearError();
    bool importData(const QByteArray& data);
    void reportError(const QString& message);

signals:
    void changed();
    void errorChanged();
    void applied();
    void selectionActivated();

private:
    struct Snapshot {
        AppConfig Config;
        QString Profile;
        QString Item;
        QString Folder;
    };
    Snapshot snapshot() const;
    void restore(const Snapshot& value);
    void mutate(const QString& mergeKey, const std::function<void()>& operation);
    void notify();
    void normalizeSelection();
    PieMenuConfig* mutableProfile();
    PieItem* mutableItem(const QString& id);
    std::vector<PieItem>* container(const QString& folder);
    std::vector<PieItem>* owner(const QString& id);
    QVariantMap itemMap(const PieItem& item) const;

    ConfigManager* Manager;
    AppConfig Draft;
    QByteArray AppliedBytes;
    QString CurrentProfile, Selected, Folder, Error, MergeKey;
    QElapsedTimer EditClock;
    std::deque<Snapshot> Undo, Redo;
    bool IsDirty = false;
};

QString KeyDisplayName(uint32_t vkCode);
QString TriggerDisplayName(const TriggerDef& trigger);
}
