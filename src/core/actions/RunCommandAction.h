#pragma once

// =============================================================================
// GoPieMenu - RunCommandAction
// =============================================================================

#include "core/IActionHandler.h"
#include <QProcess>
#include <QDebug>

namespace gpm 
{

class RunCommandAction : public IActionHandler 
{
public:
    // === IActionHandler Implementation ===
    
    [[nodiscard]] virtual ActionType GetSupportedType() const override 
    { 
        return ActionType::RunCommand; 
    }
    
    [[nodiscard]] virtual QString GetDisplayName() const override 
    { 
        return QStringLiteral("Run Command"); 
    }
    
    [[nodiscard]] virtual QString GetDescription() const override 
    {
        return QStringLiteral("Execute a shell command via cmd.exe. actionData = the command string.");
    }

    virtual bool Execute(const PieItem& Item) override 
    {
        qDebug() << "[RunCommand] Executing:" << Item.ActionData << "Args:" << Item.Arguments;

        QString FullCommand = Item.ActionData;
        if (!Item.Arguments.isEmpty()) 
        {
            FullCommand += QStringLiteral(" ") + Item.Arguments;
        }

        QStringList Args;
        Args << "/c" << FullCommand;

        bool bOk = QProcess::startDetached(QStringLiteral("cmd.exe"), Args);
        if (!bOk) 
        {
            qWarning() << "[RunCommand] Failed to execute command:" << FullCommand;
        }
        return bOk;
    }
};

} // namespace gpm
