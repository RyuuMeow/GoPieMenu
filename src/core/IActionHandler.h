#pragma once

// =============================================================================
// GoPieMenu - Action Handler Interface
// =============================================================================

#include "models/PieItem.h"

#include <QString>
#include <memory>

namespace gpm 
{

class IActionHandler 
{
public:
    virtual ~IActionHandler() = default;

    /** The action type this handler supports */
    [[nodiscard]] virtual ActionType GetSupportedType() const = 0;

    /** A human-readable name for this action type (used in UI) */
    [[nodiscard]] virtual QString GetDisplayName() const = 0;

    /** Execute the action described by the PieItem */
    virtual bool Execute(const PieItem& Item) = 0;

    /** Validate that the item's action data is well-formed */
    [[nodiscard]] virtual bool Validate(const PieItem& Item) const 
    {
        return !Item.ActionData.isEmpty();
    }

    /** A brief description for tooltip / help text */
    [[nodiscard]] virtual QString GetDescription() const 
    { 
        return {}; 
    }
};

using ActionHandlerPtr = std::unique_ptr<IActionHandler>;

} // namespace gpm
