#pragma once

// =============================================================================
// GoPieMenu - OpenFileAction
// =============================================================================

#include "core/IActionHandler.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

namespace gpm 
{

class OpenFileAction : public IActionHandler 
{
public:
    // === IActionHandler Implementation ===
    
    [[nodiscard]] virtual ActionType GetSupportedType() const override 
    { 
        return ActionType::OpenFile; 
    }
    
    [[nodiscard]] virtual QString GetDisplayName() const override 
    { 
        return QStringLiteral("Open File / Folder"); 
    }
    
    [[nodiscard]] virtual QString GetDescription() const override 
    {
        return QStringLiteral("Open a file or folder with its default application. actionData = file/folder path.");
    }

    virtual bool Execute(const PieItem& Item) override 
    {
        qDebug() << "[OpenFile] Opening:" << Item.ActionData;
        return QDesktopServices::openUrl(QUrl::fromLocalFile(Item.ActionData));
    }
};

} // namespace gpm
