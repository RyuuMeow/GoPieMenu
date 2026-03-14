// =============================================================================
// GoPieMenu - ActionExecutor Implementation
// =============================================================================

#include "ActionExecutor.h"
#include "actions/LaunchAppAction.h"
#include "actions/RunCommandAction.h"
#include "actions/SendHotkeyAction.h"
#include "actions/OpenFileAction.h"
#include "actions/OpenURLAction.h"

#include <QDebug>

namespace gpm 
{

ActionExecutor::ActionExecutor(QObject* Parent)
    : QObject(Parent)
{
}

void ActionExecutor::RegisterHandler(std::unique_ptr<IActionHandler> Handler)
{
    auto Type = Handler->GetSupportedType();
    qDebug() << "[ActionExecutor] Registered handler:" << Handler->GetDisplayName();
    Handlers[Type] = std::move(Handler);
}

bool ActionExecutor::Execute(const PieItem& Item)
{
    auto It = Handlers.find(Item.Action);
    if (It == Handlers.end()) 
    {
        auto Error = QStringLiteral("No handler for action type: %1")
                       .arg(ActionTypeToString(Item.Action));
        qWarning() << "[ActionExecutor]" << Error;
        emit ActionFailed(Item.Name, Error);
        return false;
    }

    auto& Handler = It->second;
    if (!Handler->Validate(Item)) 
    {
        auto Error = QStringLiteral("Invalid action data for: %1").arg(Item.Name);
        qWarning() << "[ActionExecutor]" << Error;
        emit ActionFailed(Item.Name, Error);
        return false;
    }

    bool bOk = Handler->Execute(Item);
    if (bOk) 
    {
        emit ActionExecuted(Item.Name, true);
    } 
    else 
    {
        emit ActionFailed(Item.Name, QStringLiteral("Execution failed"));
    }
    return bOk;
}

IActionHandler* ActionExecutor::GetHandlerFor(ActionType Type) const
{
    auto It = Handlers.find(Type);
    return It != Handlers.end() ? It->second.get() : nullptr;
}

std::vector<ActionType> ActionExecutor::GetRegisteredTypes() const
{
    std::vector<ActionType> Types;
    Types.reserve(Handlers.size());
    for (const auto& [Type, _] : Handlers) 
    {
        Types.push_back(Type);
    }
    return Types;
}

QString ActionExecutor::GetDisplayNameFor(ActionType Type) const
{
    auto* Handler = GetHandlerFor(Type);
    return Handler ? Handler->GetDisplayName() : ActionTypeToString(Type);
}

void ActionExecutor::RegisterBuiltins()
{
    RegisterHandler(std::make_unique<LaunchAppAction>());
    RegisterHandler(std::make_unique<RunCommandAction>());
    RegisterHandler(std::make_unique<SendHotkeyAction>());
    RegisterHandler(std::make_unique<OpenFileAction>());
    RegisterHandler(std::make_unique<OpenURLAction>());
}

} // namespace gpm
