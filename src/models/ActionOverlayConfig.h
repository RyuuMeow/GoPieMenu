#pragma once
#include <QJsonObject>
#include <optional>

namespace gpm {
struct ActionOverlayConfig {
    bool enabled = false;
    int position = 4; // Two rows of three anchors, in reading order.
    int opacity = 75;
    int margin = 24;
    int size = 1; // Small, medium, large.
    int durationMs = 1000;

    QJsonObject toJson() const {
        return {{"enabled", enabled}, {"position", position}, {"opacity", opacity},
                {"margin", margin}, {"size", size}, {"durationMs", durationMs}};
    }
    static std::optional<ActionOverlayConfig> parse(const QJsonObject& obj) {
        ActionOverlayConfig result;
        if (obj.contains("enabled") && !obj["enabled"].isBool()) return {};
        result.enabled = obj["enabled"].toBool();
        auto read = [&](const QString& key, int& target, int min, int max) {
            if (!obj.contains(key)) return true;
            const int value = obj[key].toInt(-1);
            if (value < min || value > max) return false;
            target = value;
            return true;
        };
        if (!read("position", result.position, 0, 5) || !read("opacity", result.opacity, 0, 100)
            || !read("margin", result.margin, 0, 160) || !read("size", result.size, 0, 2)
            || !read("durationMs", result.durationMs, 250, 5000)) return {};
        return result;
    }
};
}
