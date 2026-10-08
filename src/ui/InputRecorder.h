#pragma once
#include <QObject>
#include "core/EditorSession.h"
namespace gpm {
class InputRecorder : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY changed)
    Q_PROPERTY(QString target READ target NOTIFY changed)
public:
    explicit InputRecorder(EditorSession* session, QObject* parent = nullptr);
    ~InputRecorder() override;
    bool active() const { return Active; }
    QString target() const { return Target; }
    Q_INVOKABLE void start(const QString& target);
    Q_INVOKABLE void cancel();
signals:
    void changed();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
private:
    EditorSession* Session;
    bool Active = false;
    QString Target, Profile, Item;
};
}
