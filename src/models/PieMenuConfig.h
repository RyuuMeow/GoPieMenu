#pragma once

// =============================================================================
// GoPieMenu - Pie Menu Configuration (Profile)
// =============================================================================

#include "PieItem.h"
#include "StyleConfig.h"
#include "Types.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>

#include <vector>
#include <optional>

namespace gpm 
{

/**
 * A complete profile for one pie menu.
 */
struct PieMenuConfig 
{
    // === Data ===
    QString                     Id;
    QString                     Name;
    std::vector<PieItem>        Items;
    TriggerDef                  Trigger;
    QStringList                 AppFilter;
    std::optional<StyleConfig>  StyleOverride;
    bool                        bEnabled = true;

    // === Limits ===
    static constexpr int MinItems = 2;
    static constexpr int MaxItems = 12;

    // === Factory ===
    [[nodiscard]] static PieMenuConfig CreateDefault() 
    {
        PieMenuConfig Config;
        Config.Id   = QUuid::createUuid().toString(QUuid::WithoutBraces);
        Config.Name = QStringLiteral("New Pie Menu");
        Config.Trigger.Modifiers = ModifierKey::Ctrl;
        Config.Trigger.Button    = MouseButton::Right;

        // Add some demo items
        Config.Items.push_back(PieItem::Create(
            QStringLiteral("Notepad"), ActionType::LaunchApp,
            QStringLiteral("notepad.exe"), QStringLiteral("notepad")));
        Config.Items.push_back(PieItem::Create(
            QStringLiteral("Calculator"), ActionType::LaunchApp,
            QStringLiteral("calc.exe"), QStringLiteral("calculator")));
        Config.Items.push_back(PieItem::Create(
            QStringLiteral("Explorer"), ActionType::LaunchApp,
            QStringLiteral("explorer.exe"), QStringLiteral("folder")));
        Config.Items.push_back(PieItem::Create(
            QStringLiteral("CMD"), ActionType::RunCommand,
            QStringLiteral("cmd /k echo Hello!"), QStringLiteral("terminal")));

        return Config;
    }

    // === Serialization ===
    [[nodiscard]] QJsonObject ToJson() const 
    {
        QJsonObject Obj;
        Obj["id"]      = Id;
        Obj["name"]    = Name;
        Obj["enabled"] = bEnabled;

        // Trigger
        QJsonObject TriggerObj;
        TriggerObj["mode"]        = ActivationModeToString(Trigger.Mode);
        TriggerObj["modifiers"]   = static_cast<int>(ModifierKeyToUint(Trigger.Modifiers));
        TriggerObj["mouseButton"] = MouseButtonToString(Trigger.Button);
        TriggerObj["vkCode"]      = static_cast<int>(Trigger.VKCode);
        Obj["trigger"] = TriggerObj;

        // App filter
        QJsonArray FilterArr;
        for (const auto& App : AppFilter) 
        {
            FilterArr.append(App);
        }
        Obj["appFilter"] = FilterArr;

        // Items
        QJsonArray ItemsArr;
        for (const auto& Item : Items) 
        {
            ItemsArr.append(Item.ToJson());
        }
        Obj["items"] = ItemsArr;

        // Style override
        if (StyleOverride.has_value()) 
        {
            Obj["styleOverride"] = StyleOverride->ToJson();
        }

        return Obj;
    }

    [[nodiscard]] static PieMenuConfig FromJson(const QJsonObject& Obj) 
    {
        PieMenuConfig Config;
        Config.Id       = Obj["id"].toString(QUuid::createUuid().toString(QUuid::WithoutBraces));
        Config.Name     = Obj["name"].toString(QStringLiteral("Unnamed"));
        Config.bEnabled = Obj["enabled"].toBool(true);

        // Trigger
        if (auto TriggerObj = Obj["trigger"].toObject(); !TriggerObj.isEmpty()) 
        {
            Config.Trigger.Mode      = StringToActivationMode(TriggerObj["mode"].toString("MouseHold"));
            Config.Trigger.Modifiers = UintToModifierKey(static_cast<uint16_t>(TriggerObj["modifiers"].toInt(0)));
            Config.Trigger.Button    = StringToMouseButton(TriggerObj["mouseButton"].toString("Right"));
            Config.Trigger.VKCode    = static_cast<uint32_t>(TriggerObj["vkCode"].toInt(0));
        }

        // App filter
        for (const auto& Val : Obj["appFilter"].toArray()) 
        {
            Config.AppFilter.append(Val.toString());
        }

        // Items
        for (const auto& Val : Obj["items"].toArray()) 
        {
            Config.Items.push_back(PieItem::FromJson(Val.toObject()));
        }

        // Style override
        if (Obj.contains("styleOverride")) 
        {
            Config.StyleOverride = StyleConfig::FromJson(Obj["styleOverride"].toObject());
        }

        return Config;
    }
};

} // namespace gpm
