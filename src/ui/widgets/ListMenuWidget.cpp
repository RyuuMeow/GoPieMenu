// =============================================================================
// GoPieMenu - List Menu Widget Implementation
// =============================================================================

#include "ListMenuWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QIcon>
#include <QGuiApplication>

namespace gpm 
{

ListMenuWidget::ListMenuWidget(QWidget* Parent)
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

    connect(CloseAnim, &QPropertyAnimation::finished, this, [this]() 
    {
        hide();
        bIsOpen = false;
        emit MenuClosed();
    });
}

ListMenuWidget::~ListMenuWidget() = default;

void ListMenuWidget::ShowAt(const QPoint& InScreenPos, const std::vector<PieItem>& InItems, const StyleConfig& InStyle)
{
    if (InItems.empty()) 
    {
        return;
    }

    // === Data Binding ===
    Items        = InItems;
    Style        = InStyle;
    bIsOpen      = true;
    HoveredIndex = -1;

    // === Geometry Calculation ===
    ItemHeight = static_cast<int>(Style.FontSize) * 2 + 10;
    MenuWidth  = 220;

    int TotalHeight = ItemHeight * static_cast<int>(Items.size()) + 16;
    int W = MenuWidth + 40; 
    int H = TotalHeight + 40;

    resize(W, H);

    QPoint TopLeft(InScreenPos.x() + 30, InScreenPos.y() - TotalHeight / 2);

    if (auto* Screen = QGuiApplication::screenAt(InScreenPos)) 
    {
        QRect ScreenRect = Screen->availableGeometry();
        if (TopLeft.x() + W > ScreenRect.right()) 
        {
            TopLeft.setX(InScreenPos.x() - W - 30);
        }
        TopLeft.setY(std::clamp(TopLeft.y(), ScreenRect.top(), ScreenRect.bottom() - H));
    }

    move(TopLeft);

    // === Execution ===
    AnimProgress = 0.01;
    StartOpenAnimation();
    show();
    raise();
    update();
}

void ListMenuWidget::ShowAtDir(const QPoint& InOriginPos, double InAngle, const std::vector<PieItem>& InItems, const StyleConfig& InStyle)
{
    if (InItems.empty()) 
    {
        return;
    }

    // === Data Binding ===
    Items        = InItems;
    Style        = InStyle;
    bIsOpen      = true;
    HoveredIndex = -1;

    // === Geometry Calculation ===
    ItemHeight = static_cast<int>(Style.FontSize) * 2 + 10;
    MenuWidth  = 220;

    int TotalHeight = ItemHeight * static_cast<int>(Items.size()) + 16;
    int W = MenuWidth + 40; 
    int H = TotalHeight + 40;
    resize(W, H);

    QPoint TopLeft;
    int Gap = 15;

    if (std::cos(InAngle) >= -0.01) 
    {
        TopLeft = QPoint(InOriginPos.x() - 20 + Gap, InOriginPos.y() - 20 - TotalHeight / 2);
    } 
    else 
    {
        TopLeft = QPoint(InOriginPos.x() - W + 20 - Gap, InOriginPos.y() - 20 - TotalHeight / 2);
    }

    if (auto* Screen = QGuiApplication::screenAt(InOriginPos)) 
    {
        QRect ScreenRect = Screen->availableGeometry();
        if (std::cos(InAngle) >= -0.01 && TopLeft.x() + W > ScreenRect.right()) 
        {
            TopLeft.setX(InOriginPos.x() - W + 20 - Gap);
        } 
        else if (std::cos(InAngle) < -0.01 && TopLeft.x() < ScreenRect.left()) 
        {
            TopLeft.setX(InOriginPos.x() - 20 + Gap);
        }
        TopLeft.setY(std::clamp(TopLeft.y(), ScreenRect.top(), ScreenRect.bottom() - H));
    }

    move(TopLeft);

    // === Execution ===
    AnimProgress = 0.01;
    StartOpenAnimation();
    show();
    raise();
    update();
}

void ListMenuWidget::HideMenu()
{
    if (!bIsOpen) 
    {
        return;
    }
    StartCloseAnimation();
}

void ListMenuWidget::UpdateMousePos(const QPoint& InScreenPos)
{
    if (!bIsOpen) 
    {
        return;
    }

    QPoint Local = mapFromGlobal(InScreenPos);
    int NewHover = GetItemAtPos(Local);

    if (NewHover != HoveredIndex) 
    {
        HoveredIndex = NewHover;
        update();
    }
}

int ListMenuWidget::ConfirmSelection()
{
    if (!bIsOpen) 
    {
        return -1;
    }

    int Selected = HoveredIndex;
    if (Selected >= 0 && Selected < static_cast<int>(Items.size())) 
    {
        emit ItemSelected(Selected, Items[Selected]);
    }
    
    HideMenu();
    return Selected;
}

void ListMenuWidget::SetAnimProgress(qreal InValue)
{
    AnimProgress = InValue;
    update();
}

// === Rendering ===

void ListMenuWidget::paintEvent(QPaintEvent*)
{
    if (Items.empty()) 
    {
        return;
    }

    QPainter Painter(this);
    Painter.setRenderHint(QPainter::Antialiasing);

    double Alpha = std::clamp(AnimProgress, 0.0, 1.0);
    Painter.setOpacity(Alpha);

    double SlideOffset = 10.0 * (1.0 - Alpha);
    Painter.translate(20, 20 + SlideOffset);

    int TotalHeight = ItemHeight * static_cast<int>(Items.size()) + 16;
    QRectF BGRect(0, 0, MenuWidth, TotalHeight);

    // Draw background
    QPainterPath BGPath;
    BGPath.addRoundedRect(BGRect, 8.0, 8.0);
    
    QColor BGColor = Style.BackgroundColor;
    Painter.setPen(QPen(Style.BorderColor, Style.BorderWidth));
    Painter.setBrush(BGColor);
    Painter.drawPath(BGPath);

    // Draw items
    QFont TextFont(Style.FontFamily, static_cast<int>(Style.FontSize));
    TextFont.setWeight(QFont::Medium);
    Painter.setFont(TextFont);

    int Count = static_cast<int>(Items.size());
    for (int i = 0; i < Count; ++i) 
    {
        QRectF ItemRect(4, 8 + i * ItemHeight, MenuWidth - 8, ItemHeight);

        // Hover background
        if (i == HoveredIndex) 
        {
            QPainterPath HoverPath;
            HoverPath.addRoundedRect(ItemRect, 6.0, 6.0);
            Painter.setPen(Qt::NoPen);
            Painter.setBrush(Style.HoverColor);
            Painter.drawPath(HoverPath);
        }

        const auto& CurrentItem = Items[i];
        
        // Icon
        double IconSz = std::min(Style.IconSize * 0.8, static_cast<double>(ItemHeight - 8));
        QRectF IconRect(ItemRect.left() + 8, ItemRect.center().y() - IconSz / 2, IconSz, IconSz);
        
        if (!CurrentItem.Icon.isEmpty()) 
        {
            QIcon IconObj = QIcon(CurrentItem.Icon);
            if (IconObj.isNull()) 
            {
                IconObj = QIcon::fromTheme(CurrentItem.Icon);
            }
            
            if (!IconObj.isNull()) 
            {
                IconObj.paint(&Painter, IconRect.toRect());
            }
        }

        // Text
        QRectF TextRect(IconRect.right() + 8, ItemRect.top(), ItemRect.width() - IconSz - 24, ItemRect.height());
        
        if (i == HoveredIndex) 
        {
            QFont BoldFont = TextFont;
            BoldFont.setWeight(QFont::Bold);
            Painter.setFont(BoldFont);
            
            if (Style.bAutoContrast) 
            {
                int Br = (Style.HoverColor.red() * 299 + Style.HoverColor.green() * 587 + Style.HoverColor.blue() * 114) / 1000;
                Painter.setPen(Br > 125 ? QColor(0, 0, 0) : QColor(255, 255, 255));
            } 
            else 
            {
                Painter.setPen(QColor(255, 255, 255));
            }
        } 
        else 
        {
            Painter.setFont(TextFont);
            if (Style.bAutoContrast) 
            {
                int Br = (Style.BackgroundColor.red() * 299 + Style.BackgroundColor.green() * 587 + Style.BackgroundColor.blue() * 114) / 1000;
                Painter.setPen(Br > 125 ? QColor(0, 0, 0) : QColor(255, 255, 255));
            } 
            else 
            {
                Painter.setPen(CurrentItem.Color.value_or(Style.TextColor));
            }
        }

        Painter.drawText(TextRect, Qt::AlignLeft | Qt::AlignVCenter, CurrentItem.Name);
    }
}

int ListMenuWidget::GetItemAtPos(const QPoint& Pos) const
{
    int LocalX = Pos.x() - 20; 
    int LocalY = Pos.y() - 20;

    int TotalHeight = ItemHeight * static_cast<int>(Items.size()) + 16;
    if (LocalX < 0 || LocalX > MenuWidth || LocalY < 0 || LocalY > TotalHeight) 
    {
        return -1;
    }

    int Index = (LocalY - 8) / ItemHeight;
    if (Index >= 0 && Index < static_cast<int>(Items.size())) 
    {
        return Index;
    }
    return -1;
}

// === Input Events ===

void ListMenuWidget::mouseMoveEvent(QMouseEvent* Event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    UpdateMousePos(Event->globalPosition().toPoint());
#else
    UpdateMousePos(Event->globalPos());
#endif
}

void ListMenuWidget::mouseReleaseEvent(QMouseEvent*)
{
    ConfirmSelection();
}

void ListMenuWidget::keyPressEvent(QKeyEvent* Event)
{
    if (Event->key() == Qt::Key_Escape) 
    {
        HoveredIndex = -1;
        HideMenu();
    }
}

// === Animation ===

void ListMenuWidget::StartOpenAnimation()
{
    CloseAnim->stop();
    OpenAnim->stop();
    OpenAnim->setDuration(Style.AnimationDuration);
    OpenAnim->setStartValue(0.01);
    OpenAnim->setEndValue(Style.Opacity);
    OpenAnim->setEasingCurve(QEasingCurve::OutCubic);
    OpenAnim->start();
}

void ListMenuWidget::StartCloseAnimation()
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
