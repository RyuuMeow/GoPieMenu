#include "InputState.h"
#include <algorithm>

namespace gpm {
void InputState::reset() {
    Down.fill(false); Swallowed.fill(false); MouseDown.fill(false); ActiveId.clear();
}
void InputState::setProfiles(const std::vector<PieMenuConfig>& profiles) { Profiles = profiles; reset(); }
void InputState::synchronizeKey(int vk, bool down) { if (vk >= 0 && vk < 256) Down[vk] = down; }
bool InputState::isModifier(int vk) {
    return vk == 0x10 || vk == 0x11 || vk == 0x12 || vk == 0x5b || vk == 0x5c || (vk >= 0xa0 && vk <= 0xa5);
}
ModifierKey InputState::modifiers() const {
    auto result = ModifierKey::None;
    if (Down[0x11] || Down[0xa2] || Down[0xa3]) result = result | ModifierKey::Ctrl;
    if (Down[0x10] || Down[0xa0] || Down[0xa1]) result = result | ModifierKey::Shift;
    if (Down[0x12] || Down[0xa4] || Down[0xa5]) result = result | ModifierKey::Alt;
    if (Down[0x5b] || Down[0x5c]) result = result | ModifierKey::Win;
    return result;
}
const PieMenuConfig* InputState::match(MouseButton button, int vk, const QString& process) const {
    const PieMenuConfig* best = nullptr;
    int score = -1;
    for (const auto& p : Profiles) {
        if (!p.bEnabled || p.Items.empty() || p.Trigger.Modifiers != modifiers()) continue;
        if (p.Trigger.Mode == ActivationMode::MouseHold) {
            if (button == MouseButton::None || p.Trigger.Button != button) continue;
        } else if (vk == 0 || p.Trigger.VKCode != uint32_t(vk)) continue;
        const int candidate = p.AppFilter.isEmpty() ? 0 :
            (!process.isEmpty() && p.AppFilter.contains(process, Qt::CaseInsensitive) ? 1 : -1);
        if (candidate > score) { best = &p; score = candidate; }
    }
    return best;
}
InputDecision InputState::activate(const PieMenuConfig& p) {
    ActiveId = p.Id; ActiveTrigger = p.Trigger;
    return {true, InputEffect::Show, p.Id};
}
InputDecision InputState::cancel() {
    const bool wasActive = active(); ActiveId.clear();
    return {wasActive, wasActive ? InputEffect::Cancel : InputEffect::None, {}};
}
InputDecision InputState::key(int vk, bool down, const QString& process, bool ownInjected) {
    if (ownInjected || vk < 0 || vk >= 256) return {};
    const bool repeat = down && Down[vk];
    const bool consumedUp = !down && Swallowed[vk];
    Down[vk] = down;
    if (!down) Swallowed[vk] = false;
    if (isModifier(vk)) return {};
    if (repeat) return {Swallowed[vk], InputEffect::None, {}};
    if (active()) {
        if (vk == 0x1b && down) { Swallowed[vk] = true; return cancel(); }
        if (ActiveTrigger.Mode == ActivationMode::KeyHold && !down && ActiveTrigger.VKCode == uint32_t(vk)) {
            ActiveId.clear(); return {true, InputEffect::Confirm, {}};
        }
        if (ActiveTrigger.Mode == ActivationMode::KeyToggle && down && ActiveTrigger.VKCode == uint32_t(vk) &&
            ActiveTrigger.Modifiers == modifiers()) {
            Swallowed[vk] = true; return cancel();
        }
        if (down) Swallowed[vk] = true;
        return {down || consumedUp, InputEffect::None, {}};
    }
    if (down) if (const auto* p = match(MouseButton::None, vk, process)) {
        Swallowed[vk] = true; return activate(*p);
    }
    return {consumedUp, InputEffect::None, {}};
}
InputDecision InputState::mouse(MouseButton button, bool down, const QString& process, bool ownInjected) {
    if (ownInjected || button == MouseButton::None) return {};
    const auto index = int(button);
    const bool consumedUp = !down && MouseDown[index];
    if (!down) MouseDown[index] = false;
    if (active()) {
        if (down) MouseDown[index] = true;
        if (!down && ((ActiveTrigger.Mode == ActivationMode::MouseHold && ActiveTrigger.Button == button) ||
                      (ActiveTrigger.Mode == ActivationMode::KeyToggle && button == MouseButton::Left && consumedUp))) {
            ActiveId.clear(); return {true, InputEffect::Confirm, {}};
        }
        return {down || consumedUp, InputEffect::None, {}};
    }
    if (down) if (const auto* p = match(button, 0, process)) {
        MouseDown[index] = true; return activate(*p);
    }
    return {consumedUp, InputEffect::None, {}};
}
InputDecision InputState::move() const { return {false, active() ? InputEffect::Move : InputEffect::None, {}}; }
InputDecision InputState::wheel(int delta) const { return {active(), active() ? InputEffect::Scroll : InputEffect::None, {}, delta}; }
}
