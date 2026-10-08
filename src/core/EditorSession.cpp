#include "EditorSession.h"
#include <QFileInfo>
#include <QUrl>
#include <algorithm>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace gpm {
namespace {
QString NewId() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }
void RenewIds(PieItem& item) {
    item.Id = NewId();
    for (auto& child : item.SubItems) RenewIds(child);
}
QString ActionName(ActionType type) {
    switch (type) {
    case ActionType::LaunchApp: return QStringLiteral("Open application");
    case ActionType::SendHotkey: return QStringLiteral("Keyboard shortcut");
    case ActionType::OpenFile: return QStringLiteral("Open file or folder");
    case ActionType::OpenURL: return QStringLiteral("Open website");
    case ActionType::RunCommand: return QStringLiteral("Run command");
    case ActionType::ListMenu: return QStringLiteral("Submenu");
    default: return QStringLiteral("Choose an action");
    }
}
QString DefaultIcon(ActionType type) {
    switch (type) {
    case ActionType::LaunchApp: return QStringLiteral("app-window.svg");
    case ActionType::SendHotkey: return QStringLiteral("key-command.svg");
    case ActionType::OpenFile: return QStringLiteral("folder.svg");
    case ActionType::OpenURL: return QStringLiteral("globe.svg");
    case ActionType::RunCommand: return QStringLiteral("terminal.svg");
    case ActionType::ListMenu: return QStringLiteral("list.svg");
    default: return {};
    }
}
}
QString KeyDisplayName(uint32_t vkCode) {
    if (!vkCode) return QStringLiteral("Not set");
#ifdef Q_OS_WIN
    UINT scan = MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC);
    if (vkCode == VK_LEFT || vkCode == VK_RIGHT || vkCode == VK_UP || vkCode == VK_DOWN ||
        vkCode == VK_HOME || vkCode == VK_END || vkCode == VK_PRIOR || vkCode == VK_NEXT ||
        vkCode == VK_INSERT || vkCode == VK_DELETE) scan |= 0x100;
    wchar_t name[128]{};
    if (GetKeyNameTextW(LONG(scan << 16), name, 128)) return QString::fromWCharArray(name);
#endif
    return QStringLiteral("Key %1").arg(vkCode);
}
QString TriggerDisplayName(const TriggerDef& trigger) {
    QString key = trigger.Mode == ActivationMode::MouseHold ? MouseButtonToString(trigger.Button) + " mouse" : KeyDisplayName(trigger.VKCode);
    const auto mods = ModifierKeyToString(trigger.Modifiers);
    if (!mods.isEmpty()) key.prepend(mods + " + ");
    return key;
}

EditorSession::EditorSession(ConfigManager* manager, QObject* parent)
    : QObject(parent), Manager(manager), Draft(manager->GetConfig()), AppliedBytes(Draft.Serialize()) {
    normalizeSelection();
    if (!manager->LoadError().isEmpty())
        Error = QStringLiteral("Your configuration could not be loaded. The original file has not been changed. ") + manager->LoadError();
}
const PieMenuConfig* EditorSession::currentProfile() const {
    auto it = std::ranges::find_if(Draft.Profiles, [&](const auto& p) { return p.Id == CurrentProfile; });
    return it == Draft.Profiles.end() ? nullptr : &*it;
}
PieMenuConfig* EditorSession::mutableProfile() {
    return const_cast<PieMenuConfig*>(currentProfile());
}
const PieItem* EditorSession::findItem(const QString& id) const {
    if (const auto* p = currentProfile()) {
        for (const auto& item : p->Items) {
            if (item.Id == id) return &item;
            for (const auto& sub : item.SubItems) if (sub.Id == id) return &sub;
        }
    }
    return nullptr;
}
PieItem* EditorSession::mutableItem(const QString& id) { return const_cast<PieItem*>(findItem(id)); }
std::vector<PieItem>* EditorSession::container(const QString& folder) {
    auto* p = mutableProfile();
    if (!p) return nullptr;
    if (folder.isEmpty()) return &p->Items;
    for (auto& item : p->Items)
        if (item.Id == folder && item.Action == ActionType::ListMenu) return &item.SubItems;
    return nullptr;
}
std::vector<PieItem>* EditorSession::owner(const QString& id) {
    auto* p = mutableProfile();
    if (!p) return nullptr;
    for (auto& item : p->Items) {
        if (item.Id == id) return &p->Items;
        for (const auto& sub : item.SubItems) if (sub.Id == id) return &item.SubItems;
    }
    return nullptr;
}
QVariantList EditorSession::profiles() const {
    QVariantList list;
    for (const auto& p : Draft.Profiles) list.append(QVariantMap{{"id", p.Id}, {"name", p.Name}, {"enabled", p.bEnabled}});
    return list;
}
QVariantMap EditorSession::profile() const {
    if (const auto* p = currentProfile()) {
        return {{"id", p->Id}, {"name", p->Name}, {"enabled", p->bEnabled},
            {"appFilter", p->AppFilter.join(", ")}, {"triggerMode", int(p->Trigger.Mode)},
            {"modifiers", int(ModifierKeyToUint(p->Trigger.Modifiers))},
            {"mouseButton", MouseButtonToString(p->Trigger.Button)}, {"vkCode", p->Trigger.VKCode},
            {"triggerSummary", TriggerDisplayName(p->Trigger)}, {"customStyle", p->StyleOverride.has_value()}};
    }
    return {};
}
QVariantMap EditorSession::itemMap(const PieItem& item) const {
    return {{"id", item.Id}, {"name", item.Name}, {"icon", item.Icon},
        {"action", int(item.Action)}, {"actionName", ActionName(item.Action)}, {"target", item.ActionData},
        {"arguments", item.Arguments}, {"color", item.Color ? item.Color->name(QColor::HexArgb) : QString()},
        {"childCount", int(item.SubItems.size())}};
}
QVariantList EditorSession::items() const {
    QVariantList result;
    const auto* p = currentProfile();
    if (!p) return result;
    const std::vector<PieItem>* values = &p->Items;
    if (!Folder.isEmpty()) {
        const auto* folder = findItem(Folder);
        if (!folder) return result;
        values = &folder->SubItems;
    }
    for (const auto& item : *values) result.append(itemMap(item));
    return result;
}
QVariantMap EditorSession::selectedItem() const {
    if (const auto* item = findItem(Selected)) return itemMap(*item);
    return {};
}
QVariantList EditorSession::destinations() const {
    QVariantList result{QVariantMap{{"id", ""}, {"name", "Main menu"}}};
    if (const auto* p = currentProfile()) for (const auto& item : p->Items)
        if (item.Action == ActionType::ListMenu) result.append(QVariantMap{{"id", item.Id}, {"name", item.Name}});
    return result;
}
QString EditorSession::folderName() const {
    if (const auto* item = findItem(Folder)) return item->Name;
    return {};
}
StyleConfig EditorSession::effectiveStyle() const {
    if (const auto* p = currentProfile()) return p->StyleOverride.value_or(Draft.GlobalStyle);
    return Draft.GlobalStyle;
}
QVariantMap EditorSession::style() const { return effectiveStyle().ToJson().toVariantMap(); }
EditorSession::Snapshot EditorSession::snapshot() const { return {Draft, CurrentProfile, Selected, Folder}; }
void EditorSession::restore(const Snapshot& s) {
    Draft = s.Config; CurrentProfile = s.Profile; Selected = s.Item; Folder = s.Folder;
    MergeKey.clear(); normalizeSelection(); notify();
}
void EditorSession::normalizeSelection() {
    if (!currentProfile()) CurrentProfile = Draft.Profiles.empty() ? QString() : Draft.Profiles.front().Id;
    if (const auto* item = findItem(Folder); !item || item->Action != ActionType::ListMenu) Folder.clear();
    if (!findItem(Selected)) Selected.clear();
}
void EditorSession::notify() {
    normalizeSelection();
    IsDirty = Draft.Serialize() != AppliedBytes;
    emit changed();
}
void EditorSession::mutate(const QString& mergeKey, const std::function<void()>& operation) {
    const auto before = snapshot();
    operation();
    if (before.Config.Serialize() == Draft.Serialize()) return;
    const bool merge = !mergeKey.isEmpty() && mergeKey == MergeKey && EditClock.isValid() &&
        EditClock.elapsed() < 650 && !Undo.empty();
    if (!merge) {
        Undo.push_back(before);
        if (Undo.size() > 100) Undo.pop_front();
    }
    Redo.clear(); MergeKey = mergeKey; EditClock.restart();
    clearError(); notify();
}
void EditorSession::selectProfile(const QString& id) {
    if (id == CurrentProfile) return;
    if (std::ranges::none_of(Draft.Profiles, [&](const auto& p) { return p.Id == id; })) return;
    CurrentProfile = id; Selected.clear(); Folder.clear(); MergeKey.clear(); notify();
}
void EditorSession::selectItem(const QString& id) {
    if (!id.isEmpty() && !findItem(id)) return;
    Selected = id; MergeKey.clear(); emit changed(); emit selectionActivated();
}
void EditorSession::enterFolder(const QString& id) {
    const auto* p = currentProfile();
    if (!p) return;
    for (const auto& item : p->Items) if (item.Id == id && item.Action == ActionType::ListMenu) {
        Folder = id; Selected.clear(); MergeKey.clear(); emit changed(); return;
    }
}
void EditorSession::leaveFolder() {
    Selected = Folder; Folder.clear(); MergeKey.clear(); emit changed();
}
void EditorSession::addProfile() {
    mutate({}, [&] {
        PieMenuConfig p;
        p.Id = NewId(); p.Name = QStringLiteral("New menu"); p.bEnabled = false;
        p.Trigger.Modifiers = ModifierKey::Ctrl;
        CurrentProfile = p.Id; Selected.clear(); Folder.clear(); Draft.Profiles.push_back(std::move(p));
    });
}
void EditorSession::duplicateProfile() {
    if (!currentProfile()) return;
    mutate({}, [&] {
        auto copy = *currentProfile();
        copy.Id = NewId(); copy.Name += QStringLiteral(" copy"); copy.bEnabled = false;
        for (auto& item : copy.Items) RenewIds(item);
        CurrentProfile = copy.Id; Selected.clear(); Folder.clear(); Draft.Profiles.push_back(std::move(copy));
    });
}
void EditorSession::removeProfile() {
    if (Draft.Profiles.size() <= 1) { reportError("Keep at least one menu."); return; }
    mutate({}, [&] {
        std::erase_if(Draft.Profiles, [&](const auto& p) { return p.Id == CurrentProfile; });
        CurrentProfile.clear(); Selected.clear(); Folder.clear();
    });
}
void EditorSession::setProfileField(const QString& field, const QVariant& value) {
    if (!mutableProfile()) return;
    mutate(CurrentProfile + "/" + field, [&] {
        auto& p = *mutableProfile();
        if (field == "name") p.Name = value.toString();
        else if (field == "enabled") p.bEnabled = value.toBool();
        else if (field == "appFilter") {
            p.AppFilter.clear();
            for (const auto& part : value.toString().split(',', Qt::SkipEmptyParts)) {
                const auto name = QFileInfo(part.trimmed()).fileName();
                if (!name.isEmpty() && !p.AppFilter.contains(name, Qt::CaseInsensitive)) p.AppFilter.append(name);
            }
        }
        else if (field == "triggerMode") p.Trigger.Mode = static_cast<ActivationMode>(std::clamp(value.toInt(), 0, 2));
        else if (field == "modifiers") p.Trigger.Modifiers = UintToModifierKey(value.toUInt() & 15);
        else if (field == "mouseButton") p.Trigger.Button = StringToMouseButton(value.toString());
        else if (field == "customStyle") {
            if (value.toBool()) p.StyleOverride = effectiveStyle();
            else p.StyleOverride.reset();
        }
    });
}
void EditorSession::setTrigger(int mode, int mods, const QString& button, int key) {
    if (!mutableProfile()) return;
    mutate({}, [&] { mutableProfile()->Trigger = {static_cast<ActivationMode>(std::clamp(mode, 0, 2)),
        UintToModifierKey(mods & 15), StringToMouseButton(button), uint32_t(std::clamp(key, 0, 255))}; });
}
QString EditorSession::addItem(int action) {
    auto* values = container(Folder);
    if (!values) return {};
    const auto type = static_cast<ActionType>(std::clamp(action, 1, 6));
    if (type == ActionType::ListMenu && !Folder.isEmpty()) { reportError("Submenus can only contain actions."); return {}; }
    if (Folder.isEmpty() && values->size() >= PieMenuConfig::MaxItems) { reportError("A pie can have at most 12 items."); return {}; }
    QString id;
    mutate({}, [&] {
        auto item = PieItem::Create(type == ActionType::ListMenu ? "New submenu" : "New action", type, {}, DefaultIcon(type));
        id = item.Id; Selected = id; container(Folder)->push_back(std::move(item));
    });
    return id;
}
void EditorSession::removeItem(const QString& id) {
    if (!findItem(id)) return;
    mutate({}, [&] {
        auto* values = owner(id);
        std::erase_if(*values, [&](const auto& item) { return item.Id == id; });
        Selected.clear();
        if (Folder == id) Folder.clear();
    });
}
void EditorSession::duplicateItem(const QString& id) {
    auto* values = owner(id);
    const auto* item = findItem(id);
    if (!values || !item) return;
    if (values == &mutableProfile()->Items && values->size() >= PieMenuConfig::MaxItems) { reportError("A pie can have at most 12 items."); return; }
    mutate({}, [&] {
        auto copy = *findItem(id); RenewIds(copy); copy.Name += " copy";
        Selected = copy.Id; owner(id)->push_back(std::move(copy));
    });
}
void EditorSession::setItemField(const QString& field, const QVariant& value) {
    auto* item = mutableItem(Selected);
    if (!item) return;
    if (field == "action" && item->Action == ActionType::ListMenu && !item->SubItems.empty()) {
        reportError("Move or remove the submenu actions before changing its type."); return;
    }
    if (field == "action" && value.toInt() == int(ActionType::ListMenu) && !Folder.isEmpty()) {
        reportError("Submenus can only contain actions."); return;
    }
    mutate(CurrentProfile + "/" + Selected + "/" + field, [&] {
        auto& i = *mutableItem(Selected);
        if (field == "name") i.Name = value.toString();
        else if (field == "icon") i.Icon = value.toString();
        else if (field == "arguments") i.Arguments = value.toString();
        else if (field == "color") {
            if (value.toString().isEmpty()) i.Color.reset();
            else i.Color = QColor(value.toString());
        }
        else if (field == "action") {
            const auto next = static_cast<ActionType>(std::clamp(value.toInt(), 1, 6));
            if (next != i.Action) {
                i.Action = next; i.ActionData.clear(); i.Arguments.clear(); i.Icon = DefaultIcon(next);
            }
        }
        else if (field == "target") {
            const auto suggested = [&](const QString& target) {
                if (i.Action == ActionType::OpenURL) return QUrl::fromUserInput(target).host();
                if (i.Action == ActionType::SendHotkey) return target;
                return QFileInfo(target).completeBaseName();
            };
            const bool autoName = i.Name.isEmpty() || i.Name == "New action" || i.Name == suggested(i.ActionData);
            i.ActionData = value.toString();
            if (autoName) {
                i.Name = suggested(i.ActionData);
                if (i.Name.isEmpty()) i.Name = "New action";
            }
        }
    });
}
bool EditorSession::moveItem(const QString& id, const QString& destinationFolder, int index) {
    const auto* item = findItem(id);
    auto* from = owner(id); auto* to = container(destinationFolder);
    if (!item || !from || !to || id == destinationFolder) return false;
    if (!destinationFolder.isEmpty() && item->Action == ActionType::ListMenu) {
        reportError("A submenu cannot be placed inside another submenu."); return false;
    }
    if (destinationFolder.isEmpty() && from != to && to->size() >= PieMenuConfig::MaxItems) {
        reportError("A pie can have at most 12 items."); return false;
    }
    mutate({}, [&] {
        auto copy = *findItem(id);
        auto* source = owner(id);
        std::erase_if(*source, [&](const auto& i) { return i.Id == id; });
        auto* destination = container(destinationFolder);
        const auto position = std::clamp(index, 0, int(destination->size()));
        destination->insert(destination->begin() + position, std::move(copy));
        Folder = destinationFolder; Selected = id;
    });
    return true;
}
void EditorSession::moveSelected(int delta) {
    auto* values = owner(Selected);
    if (!values) return;
    const auto it = std::ranges::find_if(*values, [&](const auto& i) { return i.Id == Selected; });
    const int index = int(std::distance(values->begin(), it));
    const int target = index + delta;
    if (target >= 0 && target < int(values->size())) moveItem(Selected, Folder, target);
}
void EditorSession::setStyleField(const QString& field, const QVariant& value) {
    if (!mutableProfile()) return;
    mutate("style/" + CurrentProfile + "/" + field, [&] {
        auto& p = *mutableProfile();
        auto next = effectiveStyle().ToJson();
        if (!next.contains(field)) return;
        next[field] = QJsonValue::fromVariant(value);
        if (p.StyleOverride) p.StyleOverride = StyleConfig::FromJson(next);
        else Draft.GlobalStyle = StyleConfig::FromJson(next);
    });
}
void EditorSession::setStylePreset(const QString& name) {
    if (!mutableProfile()) return;
    mutate({}, [&] {
        auto next = effectiveStyle();
        auto preset = name == "Frost" ? StyleConfig::Frost() : StyleConfig();
        if (name == "Slate") { preset.SectorColor = QColor(78, 88, 103, 225); preset.HoverColor = QColor(105, 140, 188, 240); }
        if (name == "Ocean") { preset.SectorColor = QColor(40, 66, 99, 225); preset.HoverColor = QColor(60, 130, 210, 240); }
        next.BackgroundColor = preset.BackgroundColor; next.SectorColor = preset.SectorColor;
        next.HoverColor = preset.HoverColor; next.BorderColor = preset.BorderColor; next.TextColor = preset.TextColor;
        next.CenterColor = preset.CenterColor; next.CenterDotColor = preset.CenterDotColor;
        next.bAutoContrast = preset.bAutoContrast; next.TextOutlineThickness = preset.TextOutlineThickness;
        if (mutableProfile()->StyleOverride) mutableProfile()->StyleOverride = next;
        else Draft.GlobalStyle = next;
    });
}
void EditorSession::setStartWithWindows(bool value) { mutate({}, [&] { Draft.bStartWithWindows = value; }); }
void EditorSession::undo() {
    if (Undo.empty()) return;
    Redo.push_back(snapshot()); auto s = std::move(Undo.back()); Undo.pop_back(); restore(s); clearError();
}
void EditorSession::redo() {
    if (Redo.empty()) return;
    Undo.push_back(snapshot()); auto s = std::move(Redo.back()); Redo.pop_back(); restore(s); clearError();
}
bool EditorSession::apply() {
    auto result = Manager->Commit(Draft);
    if (!result) { reportError(result.Error); return false; }
    AppliedBytes = Draft.Serialize(); IsDirty = false; MergeKey.clear();
    clearError(); emit changed(); emit applied(); return true;
}
void EditorSession::discard() {
    Draft = Manager->GetConfig(); AppliedBytes = Draft.Serialize();
    Undo.clear(); Redo.clear(); MergeKey.clear(); clearError(); notify();
}
bool EditorSession::importData(const QByteArray& data) {
    auto result = ParseConfig(data);
    if (!result) { reportError(result.Error); return false; }
    mutate({}, [&] {
        Draft = std::move(*result.Value); CurrentProfile.clear(); Selected.clear(); Folder.clear();
    });
    return true;
}
void EditorSession::reportError(const QString& message) { Error = message; emit errorChanged(); }
void EditorSession::clearError() { if (!Error.isEmpty()) { Error.clear(); emit errorChanged(); } }
}
