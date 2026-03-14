#pragma once

// =============================================================================
// GoPieMenu - OpenURLAction
// =============================================================================

#include "core/IActionHandler.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

namespace gpm 
{

class OpenURLAction : public IActionHandler 
{
public:
    // === IActionHandler Implementation ===
    
    [[nodiscard]] virtual ActionType GetSupportedType() const override 
    { 
        return ActionType::OpenURL; 
    }
    
    [[nodiscard]] virtual QString GetDisplayName() const override 
    { 
        return QStringLiteral("Open URL"); 
    }
    
    [[nodiscard]] virtual QString GetDescription() const override 
    {
        return QStringLiteral("Open a URL in the default web browser. actionData = URL.");
    }

    virtual bool Execute(const PieItem& Item) override 
    {
        qDebug() << "[OpenURL] Opening:" << Item.ActionData;
        
        QUrl Url(Item.ActionData);
        if (Url.scheme().isEmpty()) 
        {
            Url.setScheme(QStringLiteral("https"));
        }
        return QDesktopServices::openUrl(Url);
    }

    [[nodiscard]] virtual bool Validate(const PieItem& Item) const override 
    {
        return !Item.ActionData.isEmpty() && QUrl(Item.ActionData).isValid();
    }
};

} // namespace gpm
