#pragma once

// =============================================================================
// GoPieMenu - Pie Item Data Model
// =============================================================================

#include "Types.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <optional>
#include <vector>

namespace gpm 
{

/**
 * Represents a single item (sector) within a pie menu.
 */
struct PieItem 
{
    // === Data ===
    QString               Id;
    QString               Name;
    QString               Icon;
    ActionType            Action = ActionType::None;
    QString               ActionData;
    QString               Arguments;
    std::optional<QColor> Color;
    std::vector<PieItem>  SubItems;

    // === Factory ===
    [[nodiscard]] static PieItem Create(
        const QString& InName,
        ActionType InType,
        const QString& InData,
        const QString& InIcon = {},
        const QString& InArgs = {}
    ) 
    {
        PieItem NewItem;
        NewItem.Id         = QUuid::createUuid().toString(QUuid::WithoutBraces);
        NewItem.Name       = InName;
        NewItem.Icon       = InIcon;
        NewItem.Action     = InType;
        NewItem.ActionData = InData;
        NewItem.Arguments  = InArgs;
        return NewItem;
    }

    // === Serialization ===
    [[nodiscard]] QJsonObject ToJson() const 
    {
        QJsonObject Obj;
        Obj["id"]         = Id;
        Obj["name"]       = Name;
        Obj["icon"]       = Icon;
        Obj["actionType"] = ActionTypeToString(Action);
        Obj["actionData"] = ActionData;
        Obj["actionArgs"] = Arguments;
        
        if (Color.has_value()) 
        {
            Obj["color"] = Color->name(QColor::HexArgb);
        }
        
        if (!SubItems.empty()) 
        {
            QJsonArray Arr;
            for (const auto& Child : SubItems) 
            {
                Arr.append(Child.ToJson());
            }
            Obj["subItems"] = Arr;
        }
        return Obj;
    }

    [[nodiscard]] static PieItem FromJson(const QJsonObject& Obj) 
    {
        PieItem Item;
        Item.Id         = Obj["id"].toString(QUuid::createUuid().toString(QUuid::WithoutBraces));
        Item.Name       = Obj["name"].toString();
        Item.Icon       = Obj["icon"].toString();
        Item.Action     = StringToActionType(Obj["actionType"].toString());
        Item.ActionData = Obj["actionData"].toString();
        Item.Arguments  = Obj["actionArgs"].toString();
        
        if (Obj.contains("color")) 
        {
            Item.Color = QColor(Obj["color"].toString());
        }
        
        if (Obj.contains("subItems")) 
        {
            for (const auto& ChildVal : Obj["subItems"].toArray()) 
            {
                Item.SubItems.push_back(PieItem::FromJson(ChildVal.toObject()));
            }
        }
        return Item;
    }
};

} // namespace gpm
