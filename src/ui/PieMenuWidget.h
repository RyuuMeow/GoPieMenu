#pragma once

// =============================================================================
// GoPieMenu - Pie Menu Widget
// =============================================================================

#include "models/PieMenuConfig.h"
#include "models/StyleConfig.h"
#include "widgets/ListMenuWidget.h"

#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>
#include <QPoint>

#include <optional>

namespace gpm 
{

class PieMenuWidget : public QWidget 
{
    Q_OBJECT
    Q_PROPERTY(qreal AnimProgress READ GetAnimProgress WRITE SetAnimProgress)

public:
    explicit PieMenuWidget(QWidget* Parent = nullptr);
    virtual ~PieMenuWidget() override = default;

    /** Open the menu at a screen position with specified configuration and style */
    void ShowAt(const QPoint& InScreenPos, const PieMenuConfig& InConfig, const StyleConfig& InGlobalStyle);
    
    /** Close the menu with animation */
    void HideMenu();
    
    /** Update selection based on mouse movement */
    void UpdateMousePos(const QPoint& InScreenPos);
    
    /** Finalize selection and trigger action */
    int ConfirmSelection();

    // === Property Accessors ===
    [[nodiscard]] qreal GetAnimProgress() const { return AnimProgress; }
    void SetAnimProgress(qreal InValue);

    // === Configuration ===
    void SetSafeZoneRadius(double InRadius) { SafeZoneRadius = InRadius; }
    [[nodiscard]] double GetSafeZoneRadius() const { return SafeZoneRadius; }

signals:
    void ItemSelected(int Index, const PieItem& Item);
    void MenuClosed();

protected:
    virtual void paintEvent(QPaintEvent* Event) override;
    virtual void mouseMoveEvent(QMouseEvent* Event) override;
    virtual void mouseReleaseEvent(QMouseEvent* Event) override;
    virtual void keyPressEvent(QKeyEvent* Event) override;

private:
    // === Rendering ===
    void DrawSectors(QPainter& Painter);
    void DrawCenter(QPainter& Painter);
    void DrawLabels(QPainter& Painter);
    void DrawIcons(QPainter& Painter);
    void DrawSafeZone(QPainter& Painter);

    // === Calculations ===
    [[nodiscard]] int GetSectorAtAngle(double Angle) const;
    [[nodiscard]] int GetSectorAtPos(const QPoint& Pos) const;
    [[nodiscard]] double GetAngleForSector(int Index) const;
    [[nodiscard]] double GetSectorSpanAngle() const;
    [[nodiscard]] QPointF GetSectorCenter(int Index, double Radius) const;
    [[nodiscard]] bool bIsInsideDeadzone(const QPoint& Pos) const;
    [[nodiscard]] bool bIsInsideSafeZone(const QPoint& ScreenPos) const;

    // === Animation ===
    void StartOpenAnimation();
    void StartCloseAnimation();

    // === Data ===
    PieMenuConfig       Config;
    StyleConfig         Style;
    QPoint              Origin;
    QPoint              ScreenOrigin;

    // === State ===
    int                 HoveredIndex = -1;
    bool                bIsOpen      = false;
    qreal               AnimProgress = 0.0;
    double              SafeZoneRadius = 35.0;

    // === Widgets ===
    QPropertyAnimation* OpenAnim  = nullptr;
    QPropertyAnimation* CloseAnim = nullptr;
    ListMenuWidget*     ListMenu  = nullptr;
    QTimer*             ListHideTimer = nullptr;
};

} // namespace gpm
