#pragma once
#include "models/PieMenuConfig.h"

namespace gpm {
inline int MenuScope(const PieMenuConfig& profile, const QString& process) {
    if (!profile.bEnabled || profile.Items.empty()) return -1;
    if (profile.AppFilter.isEmpty()) return 0;
    return !process.isEmpty() && profile.AppFilter.contains(process, Qt::CaseInsensitive) ? 1 : -1;
}
// Stable profile order breaks ties. Visibility is deliberately absent from arbitration.
inline const PieMenuConfig* PersistentMenu(const std::vector<PieMenuConfig>& profiles, const QString& process) {
    const PieMenuConfig* best = nullptr;
    int score = -1;
    for (const auto& profile : profiles) {
        const int scope = MenuScope(profile, process);
        if (profile.Persistent && scope > score) { best = &profile; score = scope; }
    }
    return best;
}
inline bool CanTriggerMenu(const std::vector<PieMenuConfig>& profiles, const PieMenuConfig& profile, const QString& process) {
    const int scope = MenuScope(profile, process);
    if (profile.Persistent || scope < 0) return false;
    const auto* resident = PersistentMenu(profiles, process);
    return !resident || scope > MenuScope(*resident, process);
}
}
