#pragma once

// =============================================================================
// GoPieMenu - Hook Provider Interface
// =============================================================================

#include "models/Types.h"

#include <QObject>
#include <QPoint>
#include <QString>

namespace gpm
{

class IHookProvider : public QObject 
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~IHookProvider() override = default;

    virtual bool Install() = 0;
    virtual void Uninstall() = 0;
    [[nodiscard]] virtual bool IsInstalled() const = 0;

signals:
    // Emitted when a trigger combination is detected
    void Triggered(const QString& ProfileId, const QPoint& MousePos);
    
    // Emitted when the trigger is released (for press-and-hold mode)
    void TriggerReleased(const QPoint& MousePos);
    
    // Emitted on mouse move while trigger is held
    void MouseMoved(const QPoint& MousePos);
    
    // Emitted when a background cancellation occurs (e.g. Esc pressed)
    void CancelRequested();
};

} // namespace gpm
