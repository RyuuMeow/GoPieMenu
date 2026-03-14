#pragma once

// =============================================================================
// GoPieMenu - Config Provider Interface
// =============================================================================

#include "models/AppConfig.h"

namespace gpm 
{

class IConfigProvider 
{
public:
    virtual ~IConfigProvider() = default;

    /** Load configuration from persistent storage */
    [[nodiscard]] virtual AppConfig Load() = 0;
    
    /** Save configuration to persistent storage */
    virtual bool Save(const AppConfig& InConfig) = 0;
    
    /** Get the absolute path to the configuration file */
    [[nodiscard]] virtual QString GetConfigFilePath() const = 0;
};

} // namespace gpm
