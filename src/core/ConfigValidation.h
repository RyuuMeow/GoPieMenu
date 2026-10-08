#pragma once

#include "models/AppConfig.h"
#include <optional>

namespace gpm {
struct ConfigLoadResult {
    std::optional<AppConfig> Value;
    QString Error;
    explicit operator bool() const { return Value.has_value(); }
};
struct ConfigSaveResult {
    bool Success = false;
    QString Error;
    explicit operator bool() const { return Success; }
};
// Parsing never substitutes defaults for damaged user data.
[[nodiscard]] ConfigLoadResult ParseConfig(const QByteArray& data);
[[nodiscard]] QString ValidateConfig(const AppConfig& config);
[[nodiscard]] QString ValidateStyle(const StyleConfig& style, int itemCount = 0);
}
