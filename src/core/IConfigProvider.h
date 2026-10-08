#pragma once
#include "ConfigValidation.h"

namespace gpm {
class IConfigProvider {
public:
    virtual ~IConfigProvider() = default;
    [[nodiscard]] virtual ConfigLoadResult Load() = 0;
    [[nodiscard]] virtual ConfigSaveResult Save(const AppConfig& config) = 0;
    [[nodiscard]] virtual QString GetConfigFilePath() const = 0;
};
}
