#pragma once

// =============================================================================
// GoPieMenu - Application-Level Configuration
// =============================================================================

#include "PieMenuConfig.h"
#include "StyleConfig.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <vector>

namespace gpm 
{

/**
 * Top-level configuration containing all layouts and global settings.
 */
struct AppConfig 
{
    // === Data ===
    std::vector<PieMenuConfig> Profiles;
    StyleConfig                GlobalStyle;
    bool                       bStartWithWindows = false;
    QString                    Language          = QStringLiteral("en");

    // === Factory ===
    [[nodiscard]] static AppConfig CreateDefault() 
    {
        AppConfig Config;
        Config.Profiles.push_back(PieMenuConfig::CreateDefault());
        return Config;
    }

    // === Serialization ===
    [[nodiscard]] QJsonObject ToJson() const 
    {
        QJsonObject Obj;

        QJsonArray ProfilesArr;
        for (const auto& P : Profiles) 
        {
            ProfilesArr.append(P.ToJson());
        }
        
        Obj["profiles"]    = ProfilesArr;
        Obj["globalStyle"] = GlobalStyle.ToJson();
        Obj["startWithWindows"] = bStartWithWindows;
        Obj["language"]    = Language;

        // Metadata
        Obj["version"]     = QStringLiteral("1.0");

        return Obj;
    }

    [[nodiscard]] static AppConfig FromJson(const QJsonObject& Obj) 
    {
        AppConfig Config;

        for (const auto& Val : Obj["profiles"].toArray()) 
        {
            Config.Profiles.push_back(PieMenuConfig::FromJson(Val.toObject()));
        }

        if (Obj.contains("globalStyle")) 
        {
            Config.GlobalStyle = StyleConfig::FromJson(Obj["globalStyle"].toObject());
        }

        Config.bStartWithWindows = Obj["startWithWindows"].toBool(false);
        Config.Language          = Obj["language"].toString(QStringLiteral("en"));

        return Config;
    }

    [[nodiscard]] QByteArray Serialize() const 
    {
        return QJsonDocument(ToJson()).toJson(QJsonDocument::Indented);
    }

    [[nodiscard]] static AppConfig Deserialize(const QByteArray& Data) 
    {
        auto Doc = QJsonDocument::fromJson(Data);
        if (Doc.isNull() || !Doc.isObject()) 
        {
            return CreateDefault();
        }
        return FromJson(Doc.object());
    }
};

} // namespace gpm
