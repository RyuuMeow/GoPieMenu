#pragma once

// =============================================================================
// GoPieMenu - Pie Preview Widget (for Settings)
// =============================================================================

#include "models/PieMenuConfig.h"
#include "models/StyleConfig.h"

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

#include <cmath>
#include <numbers>

namespace gpm 
{

class PiePreviewWidget : public QWidget 
{
    Q_OBJECT

public:
    explicit PiePreviewWidget(QWidget* Parent = nullptr)
        : QWidget(Parent)
    {
        setMinimumSize(320, 320);
        setFixedHeight(360);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setMouseTracking(true);
        setStyleSheet(QStringLiteral(
            "background: #0e0e16; border: 1px solid #1e1e30; border-radius: 8px;"));
    }

    /** Update the preview with the current configuration and global style */
    void SetConfig(const PieMenuConfig& InConfig, const StyleConfig& InGlobalStyle) 
    {
        Config       = InConfig;
        Style        = InConfig.StyleOverride.value_or(InGlobalStyle);
        HoveredIndex = -1;
        update();
    }

protected:
    virtual void paintEvent(QPaintEvent*) override 
    {
        if (Config.Items.empty()) 
        {
            return;
        }

        QPainter Painter(this);
        Painter.setRenderHint(QPainter::Antialiasing);

        double MaxRadius = Style.OuterRadius + 40;
        double AvailableSize = std::min(width(), height()) - 20.0;
        double ScaleFactor = AvailableSize / (MaxRadius * 2.0);

        QPointF CenterPos(width() / 2.0, height() / 2.0);
        Center = CenterPos;
        Scale  = ScaleFactor;

        Painter.translate(CenterPos);
        Painter.scale(ScaleFactor, ScaleFactor);

        int Count = static_cast<int>(Config.Items.size());
        double SpanAngleRad = (2.0 * std::numbers::pi) / Count;
        double GapRad       = Style.GapAngle * std::numbers::pi / 180.0;
        double InnerR       = Style.InnerRadius;
        double OuterR       = Style.OuterRadius;
        double RadToDeg     = 180.0 / std::numbers::pi;

        for (int i = 0; i < Count; ++i) 
        {
            double CAngle = (std::numbers::pi / 2.0) - i * SpanAngleRad;
            double StartAngle = CAngle - SpanAngleRad / 2.0 + GapRad / 2.0;
            double EndAngle   = StartAngle + SpanAngleRad - GapRad;

            QPainterPath Path;
            QRectF OuterRect(-OuterR, -OuterR, OuterR * 2, OuterR * 2);
            QRectF InnerRect(-InnerR, -InnerR, InnerR * 2, InnerR * 2);

            double StartDeg = StartAngle * RadToDeg;
            double SpanDeg  = (EndAngle - StartAngle) * RadToDeg;

            Path.arcMoveTo(OuterRect, StartDeg);
            Path.arcTo(OuterRect, StartDeg, SpanDeg);
            Path.arcTo(InnerRect, StartDeg + SpanDeg, -SpanDeg);
            Path.closeSubpath();

            QColor FillColor = (i == HoveredIndex) ? Style.HoverColor : Style.SectorColor;
            if (Config.Items[i].Color.has_value() && i != HoveredIndex) 
            {
                FillColor = Config.Items[i].Color.value();
            }

            Painter.setPen(QPen(Style.BorderColor, Style.BorderWidth));
            Painter.setBrush(FillColor);
            Painter.drawPath(Path);

            // Icon
            if (!Config.Items[i].Icon.isEmpty()) 
            {
                double IconR = (InnerR + OuterR) / 2.0;
                if (!Config.Items[i].Name.isEmpty()) 
                {
                    IconR -= 10; // Shift icon up if name exists
                }
                QPointF IconPos(std::cos(CAngle) * IconR, -std::sin(CAngle) * IconR);
                double IconSz = Style.IconSize * 0.8;
                QRectF IconRect(IconPos.x() - IconSz / 2.0, IconPos.y() - IconSz / 2.0, IconSz, IconSz);

                QIcon IconObj;
                if (Config.Items[i].Icon.contains('/') || Config.Items[i].Icon.contains('\\')) 
                {
                    IconObj = QIcon(Config.Items[i].Icon);
                } 
                else 
                {
                    QString IconFolder = qApp->applicationDirPath() + QStringLiteral("/icons");
                    IconObj = QIcon(IconFolder + "/" + Config.Items[i].Icon);
                }

                if (!IconObj.isNull()) 
                {
                    IconObj.paint(&Painter, IconRect.toRect());
                }
            }

            // Label
            double LabelR = (InnerR + OuterR) / 2.0;
            if (!Config.Items[i].Icon.isEmpty()) 
            {
                LabelR += 12; // Shift name down if icon exists
            }
            QPointF LabelPos(std::cos(CAngle) * LabelR, -std::sin(CAngle) * LabelR);

            QFont LabelFont(Style.FontFamily, static_cast<int>(Style.FontSize * 0.85));
            if (i == HoveredIndex) 
            {
                LabelFont.setWeight(QFont::Bold);
            }
            
            Painter.setFont(LabelFont);
            Painter.setPen(Style.TextColor);
            QRectF TextRect(LabelPos.x() - 55, LabelPos.y() - 10, 110, 20);
            Painter.drawText(TextRect, Qt::AlignCenter, Config.Items[i].Name);
        }

        // Center
        double CenterR = InnerR - 4;
        if (CenterR > 0) 
        {
            Painter.setPen(Qt::NoPen);
            Painter.setBrush(Style.CenterColor);
            Painter.drawEllipse(QPointF(0, 0), CenterR, CenterR);

            if (HoveredIndex >= 0) 
            {
                double Angle = (std::numbers::pi / 2.0) - HoveredIndex * SpanAngleRad;
                double DotRadius = 4.0;
                double DotDistance = CenterR * 0.55;
                QPointF DotPos(std::cos(Angle) * DotDistance, -std::sin(Angle) * DotDistance);
                Painter.setBrush(Style.CenterDotColor);
                Painter.drawEllipse(DotPos, DotRadius, DotRadius);
            }
        }
    }

    virtual void mouseMoveEvent(QMouseEvent* Event) override 
    {
        int Count = static_cast<int>(Config.Items.size());
        if (Count == 0) 
        {
            return;
        }

        QPointF Delta = (Event->position() - Center) / Scale;
        double Distance = std::sqrt(Delta.x() * Delta.x() + Delta.y() * Delta.y());

        if (Distance < Style.InnerRadius || Distance > Style.OuterRadius + 30) 
        {
            if (HoveredIndex != -1) 
            {
                HoveredIndex = -1; 
                update(); 
            }
            return;
        }

        double Angle = std::atan2(-Delta.y(), Delta.x());
        double SpanAngleRad = (2.0 * std::numbers::pi) / Count;
        double NormAngle = std::fmod(
            std::numbers::pi / 2.0 - Angle + 2.0 * std::numbers::pi,
            2.0 * std::numbers::pi);
        int Index = static_cast<int>(std::floor(NormAngle / SpanAngleRad + 0.5)) % Count;

        if (Index != HoveredIndex) 
        {
            HoveredIndex = Index;
            update();
        }
    }

    virtual void leaveEvent(QEvent*) override 
    {
        HoveredIndex = -1;
        update();
    }

private:
    PieMenuConfig Config;
    StyleConfig   Style;
    int           HoveredIndex = -1;
    QPointF       Center;
    double        Scale = 1.0;
};

} // namespace gpm
