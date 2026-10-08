#pragma once
#include "models/PieMenuConfig.h"
#include <array>

namespace gpm {
inline constexpr uintptr_t OwnInputTag = 0x47504d32u;
enum class InputEffect { None, Show, Confirm, Cancel, Move, Scroll };
struct InputDecision {
    bool Swallow = false;
    InputEffect Effect = InputEffect::None;
    QString ProfileId;
    int WheelDelta = 0;
};
// Pure input state machine, independent of Win32 callbacks and UI lifetimes.
class InputState {
public:
    void setProfiles(const std::vector<PieMenuConfig>& profiles);
    void reset();
    void synchronizeKey(int vk, bool down);
    ModifierKey modifiers() const;
    bool active() const { return !ActiveId.isEmpty(); }
    InputDecision key(int vk, bool down, const QString& process = {}, bool ownInjected = false);
    InputDecision mouse(MouseButton button, bool down, const QString& process = {}, bool ownInjected = false);
    InputDecision move() const;
    InputDecision wheel(int delta) const;
    InputDecision cancel();
private:
    const PieMenuConfig* match(MouseButton button, int vk, const QString& process) const;
    static bool isModifier(int vk);
    InputDecision activate(const PieMenuConfig& profile);
    std::vector<PieMenuConfig> Profiles;
    std::array<bool, 256> Down{}, Swallowed{};
    std::array<bool, 6> MouseDown{};
    QString ActiveId;
    TriggerDef ActiveTrigger;
};
}
