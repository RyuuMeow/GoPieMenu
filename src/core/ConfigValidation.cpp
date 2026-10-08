#include "ConfigValidation.h"
#include "Shortcut.h"
#include <QJsonParseError>
#include <QSet>
#include <QUrl>
#include <cmath>
#include <functional>

namespace gpm {
namespace {
bool InRange(double value, double low, double high) {
    return std::isfinite(value) && value >= low && value <= high;
}
QString CheckFields(const QJsonObject& object, const QStringList& keys, QJsonValue::Type type) {
    for (const auto& key : keys)
        if (object.contains(key) && object[key].type() != type)
            return QStringLiteral("Invalid value type for %1.").arg(key);
    return {};
}
QString CheckJsonStyle(const QJsonObject& style) {
    auto error = CheckFields(style, {"outerRadius", "innerRadius", "iconSize", "gapAngle", "gapAngleDegrees",
        "fontSize", "animationDuration", "animationDurationMs", "hoverScale", "opacity", "borderWidth", "textOutlineThickness"}, QJsonValue::Double);
    if (!error.isEmpty()) return error;
    error = CheckFields(style, {"backgroundColor", "sectorColor", "hoverColor", "borderColor", "textColor",
        "centerColor", "centerDotColor", "fontFamily"}, QJsonValue::String);
    return error.isEmpty() ? CheckFields(style, {"autoContrast", "autoContrastText"}, QJsonValue::Bool) : error;
}
QString CheckJsonItem(const QJsonObject& item, int depth) {
    if (depth > 1) return QStringLiteral("Submenus can contain actions, but cannot contain other submenus.");
    if (auto error = CheckFields(item, {"id", "name", "icon", "actionType", "actionData", "actionArgs", "color"}, QJsonValue::String); !error.isEmpty()) return error;
    const auto action = item.value("actionType").toString("None");
    if (action != "None" && ActionTypeToString(StringToActionType(action)) != action)
        return QStringLiteral("Unknown action type: %1").arg(action);
    if (item.contains("subItems") && !item["subItems"].isArray())
        return QStringLiteral("Submenu items must be a list.");
    for (const auto& child : item["subItems"].toArray()) {
        if (!child.isObject()) return QStringLiteral("Each submenu item must be an object.");
        if (auto error = CheckJsonItem(child.toObject(), depth + 1); !error.isEmpty()) return error;
    }
    return {};
}
}
QString ValidateStyle(const StyleConfig& s, int itemCount) {
    if (!InRange(s.OuterRadius, 80, 400) || !InRange(s.InnerRadius, 10, 150) || s.InnerRadius >= s.OuterRadius - 8)
        return QStringLiteral("The inner radius must be smaller than the outer radius (80–400).");
    if (!InRange(s.IconSize, 12, 96) || !InRange(s.FontSize, 8, 32))
        return QStringLiteral("Choose an icon size of 12–96 and a font size of 8–32.");
    if (!InRange(s.GapAngle, 0, 30) || (itemCount > 1 && s.GapAngle >= 360.0 / itemCount))
        return QStringLiteral("The gap must be smaller than each sector.");
    if (!InRange(s.Opacity, 0.1, 1) || !InRange(s.BorderWidth, 0, 8) ||
        !InRange(s.TextOutlineThickness, 0, 10) || !InRange(s.HoverScale, 1, 1.25) ||
        s.AnimationDuration < 0 || s.AnimationDuration > 1000)
        return QStringLiteral("The appearance contains an out-of-range value.");
    for (const auto& color : {s.BackgroundColor, s.SectorColor, s.HoverColor, s.BorderColor,
                             s.TextColor, s.CenterColor, s.CenterDotColor})
        if (!color.isValid()) return QStringLiteral("The appearance contains an invalid color.");
    return {};
}
QString ValidateConfig(const AppConfig& config) {
    if (config.Profiles.empty()) return QStringLiteral("Keep at least one menu.");
    if (auto error = ValidateStyle(config.GlobalStyle); !error.isEmpty()) return error;
    QSet<QString> profileIds;
    for (const auto& profile : config.Profiles) {
        const auto prefix = profile.Name + QStringLiteral(": ");
        if (profile.Id.isEmpty() || profileIds.contains(profile.Id)) return QStringLiteral("Menu IDs must be unique.");
        profileIds.insert(profile.Id);
        if (profile.Name.trimmed().isEmpty()) return QStringLiteral("Give each menu a name.");
        if (profile.Items.size() > PieMenuConfig::MaxItems) return prefix + QStringLiteral("A pie can have at most 12 items.");
        if (!profile.Trigger.IsValid() || (ModifierKeyToUint(profile.Trigger.Modifiers) & ~15) || profile.Trigger.VKCode > 255)
            return prefix + QStringLiteral("Choose a valid trigger.");
        if (profile.bEnabled && profile.Items.empty()) return prefix + QStringLiteral("Add an action or disable this empty menu.");
        if (auto error = ValidateStyle(profile.StyleOverride.value_or(config.GlobalStyle), int(profile.Items.size())); !error.isEmpty())
            return prefix + error;
        QSet<QString> itemIds;
        std::function<QString(const PieItem&, bool)> check = [&](const PieItem& item, bool child) -> QString {
            if (item.Id.isEmpty() || itemIds.contains(item.Id)) return QStringLiteral("Item IDs must be unique within a menu.");
            itemIds.insert(item.Id);
            if (child && item.Action == ActionType::ListMenu) return QStringLiteral("A submenu can only contain actions.");
            if (item.Action != ActionType::ListMenu && !item.SubItems.empty()) return QStringLiteral("Only submenus can have children.");
            if (item.Color && !item.Color->isValid()) return QStringLiteral("An item contains an invalid color.");
            if (profile.bEnabled) {
                if (item.Name.trimmed().isEmpty()) return QStringLiteral("Give each action a name.");
                if (item.Action == ActionType::None) return item.Name + QStringLiteral(": choose an action.");
                if (item.Action != ActionType::ListMenu && item.ActionData.trimmed().isEmpty())
                    return item.Name + QStringLiteral(": enter a target or record a shortcut.");
                if (item.Action == ActionType::SendHotkey && ParseShortcut(item.ActionData).empty())
                    return item.Name + QStringLiteral(": record a valid keyboard shortcut.");
                if (item.Action == ActionType::OpenURL) {
                    const auto url = QUrl::fromUserInput(item.ActionData);
                    if (!url.isValid() || url.scheme().isEmpty()) return item.Name + QStringLiteral(": enter a valid URL.");
                }
            }
            for (const auto& sub : item.SubItems) if (auto error = check(sub, true); !error.isEmpty()) return error;
            return {};
        };
        for (const auto& item : profile.Items) if (auto error = check(item, false); !error.isEmpty()) return prefix + error;
    }
    return {};
}
ConfigLoadResult ParseConfig(const QByteArray& data) {
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return {{}, QStringLiteral("Invalid JSON at byte %1: %2").arg(parseError.offset).arg(parseError.errorString())};
    if (!document.isObject()) return {{}, QStringLiteral("The configuration must be a JSON object.")};
    const auto root = document.object();
    if (auto error = CheckFields(root, {"startWithWindows"}, QJsonValue::Bool); !error.isEmpty()) return {{}, error};
    if (auto error = CheckFields(root, {"language"}, QJsonValue::String); !error.isEmpty()) return {{}, error};
    if (root.contains("version") && root["version"].toString() != "1.0")
        return {{}, QStringLiteral("This configuration version is not supported.")};
    if (!root["profiles"].isArray()) return {{}, QStringLiteral("The configuration must contain a profiles list.")};
    if (root.contains("globalStyle") && !root["globalStyle"].isObject())
        return {{}, QStringLiteral("The global appearance must be an object.")};
    if (auto error = CheckJsonStyle(root["globalStyle"].toObject()); !error.isEmpty()) return {{}, error};
    for (const auto& value : root["profiles"].toArray()) {
        if (!value.isObject()) return {{}, QStringLiteral("Each menu must be an object.")};
        const auto profile = value.toObject();
        if (auto error = CheckFields(profile, {"id", "name"}, QJsonValue::String); !error.isEmpty()) return {{}, error};
        if (auto error = CheckFields(profile, {"enabled"}, QJsonValue::Bool); !error.isEmpty()) return {{}, error};
        if (auto error = CheckFields(profile, {"trigger", "styleOverride"}, QJsonValue::Object); !error.isEmpty()) return {{}, error};
        if (auto error = CheckFields(profile, {"appFilter"}, QJsonValue::Array); !error.isEmpty()) return {{}, error};
        for (const auto& app : profile["appFilter"].toArray()) if (!app.isString()) return {{}, QStringLiteral("Application filters must be text.")};
        if (auto error = CheckJsonStyle(profile["styleOverride"].toObject()); !error.isEmpty()) return {{}, error};
        if (!profile["items"].isArray()) return {{}, QStringLiteral("Each menu must contain an items list.")};
        const auto trigger = profile["trigger"].toObject();
        if (auto error = CheckFields(trigger, {"mode", "mouseButton"}, QJsonValue::String); !error.isEmpty()) return {{}, error};
        const auto mode = trigger["mode"].toString("MouseHold");
        if (ActivationModeToString(StringToActivationMode(mode)) != mode)
            return {{}, QStringLiteral("Unknown activation mode: %1").arg(mode)};
        if (trigger.contains("modifiers") && (trigger["modifiers"].toInt(-1) < 0 || trigger["modifiers"].toInt() > 15))
            return {{}, QStringLiteral("Invalid trigger modifiers.")};
        if (trigger.contains("vkCode") && (trigger["vkCode"].toInt(-1) < 0 || trigger["vkCode"].toInt() > 255))
            return {{}, QStringLiteral("Invalid trigger key.")};
        for (const auto& item : profile["items"].toArray()) {
            if (!item.isObject()) return {{}, QStringLiteral("Each action must be an object.")};
            if (auto error = CheckJsonItem(item.toObject(), 0); !error.isEmpty()) return {{}, error};
        }
    }
    auto config = AppConfig::FromJson(root);
    if (auto error = ValidateConfig(config); !error.isEmpty()) return {{}, error};
    return {std::move(config), {}};
}
}
