#pragma once

// =============================================================================
// GoPieMenu - Style Configuration
// =============================================================================

#include <QColor>
#include <QFont>
#include <QJsonObject>

#include <optional>

namespace gpm 
{

/**
 * Visual style settings for a pie menu.
 */
struct StyleConfig 
{
    // === Geometry ===
    double OuterRadius       = 150.0;
    double InnerRadius       = 45.0;
    double IconSize          = 28.0;
    double GapAngle          = 3.0;

    // === Colors ===
    QColor BackgroundColor   = QColor(30, 30, 30, 220);
    QColor SectorColor       = QColor(55, 55, 65, 200);
    QColor HoverColor        = QColor(80, 120, 200, 220);
    QColor BorderColor       = QColor(100, 100, 120, 150);
    QColor TextColor         = QColor(240, 240, 245);
    QColor CenterColor       = QColor(45, 45, 55, 240);
    QColor CenterDotColor    = QColor(100, 140, 220);

    // === Typography ===
    QString FontFamily       = QStringLiteral("Segoe UI");
    double  FontSize         = 11.0;

    // === Animation ===
    int     AnimationDuration = 150;
    double  HoverScale        = 1.05;

    // === Appearance ===
    double  Opacity           = 0.95;
    double  BorderWidth       = 1.0;
    bool    bAutoContrast     = false;
    double  TextOutlineThickness = 3.0;

    // === Serialization ===
    [[nodiscard]] QJsonObject ToJson() const 
    {
        QJsonObject Obj;
        Obj["outerRadius"]       = OuterRadius;
        Obj["innerRadius"]       = InnerRadius;
        Obj["iconSize"]          = IconSize;
        Obj["gapAngle"]          = GapAngle;
        Obj["backgroundColor"]   = BackgroundColor.name(QColor::HexArgb);
        Obj["sectorColor"]       = SectorColor.name(QColor::HexArgb);
        Obj["hoverColor"]        = HoverColor.name(QColor::HexArgb);
        Obj["borderColor"]       = BorderColor.name(QColor::HexArgb);
        Obj["textColor"]         = TextColor.name(QColor::HexArgb);
        Obj["centerColor"]       = CenterColor.name(QColor::HexArgb);
        Obj["centerDotColor"]    = CenterDotColor.name(QColor::HexArgb);
        Obj["fontFamily"]        = FontFamily;
        Obj["fontSize"]          = FontSize;
        Obj["animationDuration"] = AnimationDuration;
        Obj["hoverScale"]        = HoverScale;
        Obj["opacity"]           = Opacity;
        Obj["borderWidth"]       = BorderWidth;
        Obj["autoContrast"]      = bAutoContrast;
        Obj["textOutlineThickness"] = TextOutlineThickness;
        return Obj;
    }

    [[nodiscard]] static StyleConfig FromJson(const QJsonObject& Obj) 
    {
        StyleConfig S;
        if (Obj.contains("outerRadius"))       S.OuterRadius       = Obj["outerRadius"].toDouble(S.OuterRadius);
        if (Obj.contains("innerRadius"))       S.InnerRadius       = Obj["innerRadius"].toDouble(S.InnerRadius);
        if (Obj.contains("iconSize"))          S.IconSize          = Obj["iconSize"].toDouble(S.IconSize);
        if (Obj.contains("gapAngle"))          S.GapAngle          = Obj["gapAngle"].toDouble(S.GapAngle);
        // Fallback for old keys
        if (Obj.contains("gapAngleDegrees"))   S.GapAngle          = Obj["gapAngleDegrees"].toDouble(S.GapAngle);
        
        if (Obj.contains("backgroundColor"))   S.BackgroundColor   = QColor(Obj["backgroundColor"].toString());
        if (Obj.contains("sectorColor"))       S.SectorColor       = QColor(Obj["sectorColor"].toString());
        if (Obj.contains("hoverColor"))        S.HoverColor        = QColor(Obj["hoverColor"].toString());
        if (Obj.contains("borderColor"))       S.BorderColor       = QColor(Obj["borderColor"].toString());
        if (Obj.contains("textColor"))         S.TextColor         = QColor(Obj["textColor"].toString());
        if (Obj.contains("centerColor"))       S.CenterColor       = QColor(Obj["centerColor"].toString());
        if (Obj.contains("centerDotColor"))    S.CenterDotColor    = QColor(Obj["centerDotColor"].toString());
        if (Obj.contains("fontFamily"))        S.FontFamily        = Obj["fontFamily"].toString();
        if (Obj.contains("fontSize"))          S.FontSize          = Obj["fontSize"].toDouble(S.FontSize);
        
        if (Obj.contains("animationDuration")) S.AnimationDuration = Obj["animationDuration"].toInt(S.AnimationDuration);
        // Fallback for old keys
        if (Obj.contains("animationDurationMs")) S.AnimationDuration = Obj["animationDurationMs"].toInt(S.AnimationDuration);
        
        if (Obj.contains("hoverScale"))        S.HoverScale        = Obj["hoverScale"].toDouble(S.HoverScale);
        if (Obj.contains("opacity"))           S.Opacity           = Obj["opacity"].toDouble(S.Opacity);
        if (Obj.contains("borderWidth"))       S.BorderWidth       = Obj["borderWidth"].toDouble(S.BorderWidth);
        
        if (Obj.contains("autoContrast"))      S.bAutoContrast     = Obj["autoContrast"].toBool(S.bAutoContrast);
        // Fallback for old keys
        if (Obj.contains("autoContrastText"))  S.bAutoContrast     = Obj["autoContrastText"].toBool(S.bAutoContrast);

        if (Obj.contains("textOutlineThickness")) S.TextOutlineThickness = Obj["textOutlineThickness"].toDouble(S.TextOutlineThickness);
        return S;
    }

    // === Helpers ===
    [[nodiscard]] static StyleConfig Merge(const StyleConfig& Base, const QJsonObject& Overrides) 
    {
        auto Merged = Base.ToJson();
        for (auto It = Overrides.begin(); It != Overrides.end(); ++It) 
        {
            Merged[It.key()] = It.value();
        }
        return FromJson(Merged);
    }
};

} // namespace gpm
