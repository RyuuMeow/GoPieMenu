// =============================================================================
// GoPieMenu - Pie Menu Widget Implementation
// =============================================================================

#include "PieMenuWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>

#include <cmath>
#include <numbers>

namespace gpm 
{

namespace 
{
    /** Conversion constants */
    constexpr double PI             = std::numbers::pi;
    constexpr double TWO_PI          = 2.0 * std::numbers::pi;
    constexpr double RAD_TO_DEG      = 180.0 / std::numbers::pi;
    constexpr double DEG_TO_RAD      = std::numbers::pi / 180.0;
    
    /** Physics constants */
    constexpr double DEADZONE_RADIUS = 20.0;
}

PieMenuWidget::PieMenuWidget(QWidget* Parent)
    : QWidget(Parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
{
    // === Initialization ===
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setMouseTracking(true);
    setFocusPolicy(Qt::NoFocus);

    // === Animation Setup ===
    OpenAnim = new QPropertyAnimation(this, "AnimProgress", this);
    CloseAnim = new QPropertyAnimation(this, "AnimProgress", this);
    CloseAnim->setEasingCurve(QEasingCurve::InBack);

    // === Hover List Setup ===
    ListMenu = new ListMenuWidget(this);
    connect(ListMenu, &ListMenuWidget::ItemSelected, this, [this](int, const PieItem& Item) 
    {
        emit ItemSelected(-1, Item);
        HideMenu();
    });
    
    ListHideTimer = new QTimer(this);
    ListHideTimer->setSingleShot(true);
    ListHideTimer->setInterval(200);
    connect(ListHideTimer, &QTimer::timeout, this, [this]() 
    {
        ListMenu->HideMenu();
    });

    connect(CloseAnim, &QPropertyAnimation::finished, this, [this]() 
    {
        hide();
        bIsOpen = false;
        emit MenuClosed();
    });
}

void PieMenuWidget::ShowAt(const QPoint& InScreenPos, const PieMenuConfig& InConfig, const StyleConfig& InGlobalStyle)
{
    // === Data Binding ===
    Config      = InConfig;
    Style       = InConfig.StyleOverride.value_or(InGlobalStyle);
    ScreenOrigin = InScreenPos;
    HoveredIndex = -1;
    bIsOpen      = true;

    // === Geometry Calculation ===
    int Diameter = static_cast<int>(Style.OuterRadius * 2 + 450);
    resize(Diameter, Diameter);
    Origin = QPoint(Diameter / 2, Diameter / 2);

    QPoint TopLeft(InScreenPos.x() - Diameter / 2, InScreenPos.y() - Diameter / 2);
    if (auto* Screen = QGuiApplication::screenAt(InScreenPos)) 
    {
        QRect ScreenRect = Screen->availableGeometry();
        TopLeft.setX(std::clamp(TopLeft.x(), ScreenRect.left(), ScreenRect.right() - Diameter));
        TopLeft.setY(std::clamp(TopLeft.y(), ScreenRect.top(), ScreenRect.bottom() - Diameter));
        Origin = InScreenPos - TopLeft;
    }

    move(TopLeft);
    AnimProgress = 0.01;
    
    // === Execution ===
    StartOpenAnimation();
    show();
    raise();
    update();
}

void PieMenuWidget::HideMenu()
{
    if (!bIsOpen) 
    {
        return;
    }
    
    ListMenu->HideMenu();
    StartCloseAnimation();
}

void PieMenuWidget::UpdateMousePos(const QPoint& InScreenPos)
{
    if (!bIsOpen) 
    {
        return;
    }

    if (ListMenu->IsOpen()) 
    {
        ListMenu->UpdateMousePos(InScreenPos);
        QPoint LocalListPos = ListMenu->mapFromGlobal(InScreenPos);
        if (ListMenu->rect().contains(LocalListPos)) 
        {
            ListHideTimer->stop(); // Cancel hide if mouse enters the ListMenu bounding box
            return;
        }
    }

    QPoint Local = mapFromGlobal(InScreenPos);
    int NewHover = GetSectorAtPos(Local);

    if (NewHover != HoveredIndex) 
    {
        HoveredIndex = NewHover;

        if (HoveredIndex >= 0 && Config.Items[HoveredIndex].Action == ActionType::ListMenu) 
        {
            ListHideTimer->stop();
            double Angle = GetAngleForSector(HoveredIndex);
            QPointF Edge(std::cos(Angle) * Style.OuterRadius, -std::sin(Angle) * Style.OuterRadius);
            QPoint ShowPos = (Origin + Edge).toPoint() + pos();
            ListMenu->ShowAtDir(ShowPos, Angle, Config.Items[HoveredIndex].SubItems, Style);
        } 
        else if (ListMenu->IsOpen()) 
        {
            if (!ListHideTimer->isActive()) 
            {
                ListHideTimer->start();
            }
        }

        update();
    }
}

int PieMenuWidget::ConfirmSelection()
{
    if (!bIsOpen) 
    {
        return -1;
    }

    if (ListMenu->IsOpen() && ListMenu->GetHoveredIndex() >= 0) 
    {
        int SubSelected = ListMenu->ConfirmSelection();
        if (SubSelected >= 0) 
        {
            HideMenu();
            return HoveredIndex;
        }
    }

    int Selected = HoveredIndex;

    // If mouse is still inside safe zone, cancel without selecting
    if (bIsInsideSafeZone(QCursor::pos())) 
    {
        Selected = -1;
    }

    if (Selected >= 0 && Selected < static_cast<int>(Config.Items.size())) 
    {
        if (Config.Items[Selected].Action != ActionType::ListMenu) 
        {
            emit ItemSelected(Selected, Config.Items[Selected]);
        }
    }
    
    HideMenu();
    return Selected;
}

bool PieMenuWidget::bIsInsideSafeZone(const QPoint& InScreenPos) const
{
    QPointF Delta = InScreenPos - ScreenOrigin;
    return (Delta.x() * Delta.x() + Delta.y() * Delta.y()) < SafeZoneRadius * SafeZoneRadius;
}

void PieMenuWidget::SetAnimProgress(qreal InValue)
{
    AnimProgress = InValue;
    update();
}

// === Rendering ===

void PieMenuWidget::paintEvent(QPaintEvent*)
{
    if (Config.Items.empty()) 
    {
        return;
    }

    QPainter Painter(this);
    Painter.setRenderHint(QPainter::Antialiasing);

    double Scale = std::max(AnimProgress, 0.01);
    double Alpha = std::clamp(AnimProgress, 0.0, 1.0);

    Painter.translate(Origin);
    Painter.scale(Scale, Scale);
    Painter.setOpacity(Alpha);

    DrawSectors(Painter);
    DrawSafeZone(Painter);
    DrawCenter(Painter);
    DrawIcons(Painter);
    DrawLabels(Painter);
}

void PieMenuWidget::DrawSectors(QPainter& Painter)
{
    int Count = static_cast<int>(Config.Items.size());
    if (Count == 0) 
    {
        return;
    }

    double SpanAngle = GetSectorSpanAngle();
    double GapRad    = Style.GapAngle * DEG_TO_RAD;
    double InnerR    = Style.InnerRadius;
    double OuterR    = Style.OuterRadius;

    for (int i = 0; i < Count; ++i) 
    {
        double StartAngle = GetAngleForSector(i) - SpanAngle / 2.0 + GapRad / 2.0;
        double EndAngle   = StartAngle + SpanAngle - GapRad;

        QPainterPath Path;
        QRectF OuterRect(-OuterR, -OuterR, OuterR * 2, OuterR * 2);
        QRectF InnerRect(-InnerR, -InnerR, InnerR * 2, InnerR * 2);

        double StartDeg = StartAngle * RAD_TO_DEG;
        double SpanDeg  = (EndAngle - StartAngle) * RAD_TO_DEG;

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
    }
}

void PieMenuWidget::DrawSafeZone(QPainter& Painter)
{
    double R = SafeZoneRadius;
    Painter.setPen(QPen(QColor(255, 255, 255, 20), 1.0, Qt::DashLine));
    Painter.setBrush(Qt::NoBrush);
    Painter.drawEllipse(QPointF(0, 0), R, R);
}

void PieMenuWidget::DrawCenter(QPainter& Painter)
{
    double R = Style.InnerRadius - 4;
    if (R <= 0) 
    {
        return;
    }

    Painter.setPen(Qt::NoPen);
    Painter.setBrush(Style.CenterColor);
    Painter.drawEllipse(QPointF(0, 0), R, R);

    if (HoveredIndex >= 0) 
    {
        double Angle = GetAngleForSector(HoveredIndex);
        double DotR   = 5.0;
        double DotDist = R * 0.6;
        QPointF DotPos(std::cos(Angle) * DotDist, -std::sin(Angle) * DotDist);

        Painter.setBrush(Style.CenterDotColor);
        Painter.drawEllipse(DotPos, DotR, DotR);
    }
}

void PieMenuWidget::DrawLabels(QPainter& Painter)
{
    int Count = static_cast<int>(Config.Items.size());
    if (Count == 0) 
    {
        return;
    }

    QFont Font(Style.FontFamily, static_cast<int>(Style.FontSize));
    Font.setWeight(QFont::Medium);
    Painter.setFont(Font);

    double LabelR = Style.OuterRadius + 22;

    for (int i = 0; i < Count; ++i) 
    {
        double Angle = GetAngleForSector(i);
        QPointF LabelPos(std::cos(Angle) * LabelR, -std::sin(Angle) * LabelR);

        QRectF TextRect(LabelPos.x() - 90, LabelPos.y() - 14, 180, 28);
        Qt::Alignment Align = Qt::AlignCenter;

        if (std::cos(Angle) > 0.3) 
        {
            TextRect.moveLeft(LabelPos.x() + 6);
            Align = Qt::AlignLeft | Qt::AlignVCenter;
        } 
        else if (std::cos(Angle) < -0.3) 
        {
            TextRect.moveRight(LabelPos.x() - 6);
            Align = Qt::AlignRight | Qt::AlignVCenter;
        }

        QColor TextColorData;
        if (i == HoveredIndex) 
        {
            QFont BoldFont = Font;
            BoldFont.setWeight(QFont::Bold);
            Painter.setFont(BoldFont);
            
            if (Style.bAutoContrast) 
            {
                int Br = (Style.HoverColor.red() * 299 + Style.HoverColor.green() * 587 + Style.HoverColor.blue() * 114) / 1000;
                TextColorData = (Br > 125) ? QColor(0, 0, 0) : QColor(255, 255, 255);
            } 
            else 
            {
                TextColorData = QColor(255, 255, 255);
            }
        } 
        else 
        {
            Painter.setFont(Font);
            
            if (Style.bAutoContrast) 
            {
                QColor SectorBg = Config.Items[i].Color.value_or(Style.SectorColor);
                int Br = (SectorBg.red() * 299 + SectorBg.green() * 587 + SectorBg.blue() * 114) / 1000;
                TextColorData = (Br > 125) ? QColor(0, 0, 0) : QColor(255, 255, 255);
            } 
            else 
            {
                TextColorData = Style.TextColor;
            }
        }

        int TextBr = (TextColorData.red() * 299 + TextColorData.green() * 587 + TextColorData.blue() * 114) / 1000;
        QColor OutlineColor = (TextBr > 125) ? QColor(0, 0, 0, 180) : QColor(255, 255, 255, 180);
        
        QPainterPath TextPath;
        QFontMetricsF FM(Painter.font());
        
        QString TextString = Config.Items[i].Name;
        double TextWidth = FM.horizontalAdvance(TextString);
        double TextX = TextRect.left();
        double TextY = TextRect.center().y() + FM.ascent() / 2.0 - FM.descent() / 2.0;
        
        if (Align & Qt::AlignHCenter) 
        {
            TextX = TextRect.center().x() - TextWidth / 2.0;
        } 
        else if (Align & Qt::AlignRight) 
        {
            TextX = TextRect.right() - TextWidth;
        }
        
        TextPath.addText(TextX, TextY, Painter.font(), TextString);
        
        if (Style.TextOutlineThickness > 0.0) 
        {
            Painter.setPen(QPen(OutlineColor, Style.TextOutlineThickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            Painter.setBrush(Qt::NoBrush);
            Painter.drawPath(TextPath);
        }
        
        Painter.setPen(Qt::NoPen);
        Painter.setBrush(TextColorData);
        Painter.drawPath(TextPath);
        
        Painter.setBrush(Qt::NoBrush);
    }
}

void PieMenuWidget::DrawIcons(QPainter& Painter)
{
    int Count = static_cast<int>(Config.Items.size());
    if (Count == 0) 
    {
        return;
    }

    double IconR  = (Style.InnerRadius + Style.OuterRadius) / 2.0;
    double IconSz = Style.IconSize;

    for (int i = 0; i < Count; ++i) 
    {
        if (Config.Items[i].Icon.isEmpty()) 
        {
            continue;
        }

        double Angle = GetAngleForSector(i);
        QPointF IconCenter(std::cos(Angle) * IconR, -std::sin(Angle) * IconR);
        QRectF IconRect(IconCenter.x() - IconSz / 2, IconCenter.y() - IconSz / 2, IconSz, IconSz);

        QIcon IconObj;
        if (Config.Items[i].Icon.contains('/') || Config.Items[i].Icon.contains('\\')) 
        {
            // Legacy/Absolute path
            IconObj = QIcon(Config.Items[i].Icon);
        } 
        else 
        {
            // Relative path (resolve via app root icons folder)
            QString IconFolder = qApp->applicationDirPath() + QStringLiteral("/icons");
            IconObj = QIcon(IconFolder + "/" + Config.Items[i].Icon);
        }
        
        if (!IconObj.isNull()) 
        {
            IconObj.paint(&Painter, IconRect.toRect());
        }
    }
}

// === Calculations ===

double PieMenuWidget::GetSectorSpanAngle() const
{
    int Count = static_cast<int>(Config.Items.size());
    return (Count > 0) ? TWO_PI / Count : TWO_PI;
}

double PieMenuWidget::GetAngleForSector(int Index) const
{
    double Span = GetSectorSpanAngle();
    return (PI / 2.0) - Index * Span;
}

QPointF PieMenuWidget::GetSectorCenter(int Index, double Radius) const
{
    double Angle = GetAngleForSector(Index);
    return QPointF(std::cos(Angle) * Radius, -std::sin(Angle) * Radius);
}

int PieMenuWidget::GetSectorAtAngle(double Angle) const
{
    int Count = static_cast<int>(Config.Items.size());
    if (Count == 0) 
    {
        return -1;
    }

    double Norm = std::fmod(PI / 2.0 - Angle + TWO_PI, TWO_PI);
    double Span = GetSectorSpanAngle();
    int Index = static_cast<int>(std::floor(Norm / Span + 0.5)) % Count;
    return Index;
}

int PieMenuWidget::GetSectorAtPos(const QPoint& Pos) const
{
    if (bIsInsideDeadzone(Pos) || bIsInsideSafeZone(Pos + this->pos())) 
    {
        return -1;
    }

    QPointF Delta = Pos - Origin;
    double Angle = std::atan2(-Delta.y(), Delta.x());
    return GetSectorAtAngle(Angle);
}

bool PieMenuWidget::bIsInsideDeadzone(const QPoint& Pos) const
{
    QPointF Delta = Pos - Origin;
    return (Delta.x() * Delta.x() + Delta.y() * Delta.y()) < DEADZONE_RADIUS * DEADZONE_RADIUS;
}

// === Input Events ===

void PieMenuWidget::mouseMoveEvent(QMouseEvent* Event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    UpdateMousePos(Event->globalPosition().toPoint());
#else
    UpdateMousePos(Event->globalPos());
#endif
}

void PieMenuWidget::mouseReleaseEvent(QMouseEvent*)
{
    ConfirmSelection();
}

void PieMenuWidget::keyPressEvent(QKeyEvent* Event)
{
    if (Event->key() == Qt::Key_Escape) 
    {
        HoveredIndex = -1;
        HideMenu();
    }
}

// === Animation ===

void PieMenuWidget::StartOpenAnimation()
{
    CloseAnim->stop();
    OpenAnim->stop();
    OpenAnim->setDuration(Style.AnimationDuration);
    OpenAnim->setStartValue(0.01);
    OpenAnim->setEndValue(1.0);
    OpenAnim->setEasingCurve(QEasingCurve::OutCubic);
    OpenAnim->start();
}

void PieMenuWidget::StartCloseAnimation()
{
    OpenAnim->stop();
    CloseAnim->stop();
    CloseAnim->setDuration(std::max(Style.AnimationDuration / 2, 50));
    CloseAnim->setStartValue(AnimProgress);
    CloseAnim->setEndValue(0.0);
    CloseAnim->setEasingCurve(QEasingCurve::InCubic);
    CloseAnim->start();
}

} // namespace gpm
