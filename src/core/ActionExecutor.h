#pragma once

// =============================================================================
// GoPieMenu - ActionExecutor (Strategy Pattern Registry)
// =============================================================================

#include "IActionHandler.h"
#include "models/PieItem.h"

#include <QObject>
#include <unordered_map>
#include <memory>
#include <vector>

namespace gpm 
{

/**
 * Central dispatcher that routes PieItem actions to registered handlers.
 */
class ActionExecutor : public QObject 
{
    Q_OBJECT
public:
    explicit ActionExecutor(QObject* Parent = nullptr);
    virtual ~ActionExecutor() override = default;

    /** Register a new action handler — takes ownership */
    void RegisterHandler(std::unique_ptr<IActionHandler> Handler);

    /** Execute the action for a pie item */
    bool Execute(const PieItem& Item);

    // === Query ===
    [[nodiscard]] IActionHandler* GetHandlerFor(ActionType Type) const;
    [[nodiscard]] std::vector<ActionType> GetRegisteredTypes() const;
    [[nodiscard]] QString GetDisplayNameFor(ActionType Type) const;

    /** Register all built-in action handlers */
    void RegisterBuiltins();

signals:
    void ActionExecuted(const QString& ItemName, bool bSuccess);
    void ActionFailed(const QString& ItemName, const QString& Error);

private:
    // === Handlers ===
    std::unordered_map<ActionType, std::unique_ptr<IActionHandler>> Handlers;
};

} // namespace gpm
