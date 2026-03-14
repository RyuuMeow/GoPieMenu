#pragma once

// =============================================================================
// GoPieMenu - LaunchAppAction
// =============================================================================

#include "core/IActionHandler.h"
#include <QProcess>
#include <QDebug>

namespace gpm 
{

class LaunchAppAction : public IActionHandler 
{
public:
    // === IActionHandler Implementation ===
    
    [[nodiscard]] virtual ActionType GetSupportedType() const override 
    { 
        return ActionType::LaunchApp; 
    }
    
    [[nodiscard]] virtual QString GetDisplayName() const override 
    { 
        return QStringLiteral("Launch Application"); 
    }
    
    [[nodiscard]] virtual QString GetDescription() const override 
    {
        return QStringLiteral("Launch an executable program. ActionData = path to .exe, Arguments = command line arguments.");
    }

    virtual bool Execute(const PieItem& Item) override 
    {
        qDebug() << "[LaunchApp] Launching:" << Item.ActionData << "Args:" << Item.Arguments;

        QStringList Args;
        if (!Item.Arguments.isEmpty()) 
        {
            Args = QProcess::splitCommand(Item.Arguments);
        }

        bool bOk = QProcess::startDetached(Item.ActionData, Args);
        if (!bOk) 
        {
            qWarning() << "[LaunchApp] Failed to launch:" << Item.ActionData;
        }
        return bOk;
    }
};

} // namespace gpm
