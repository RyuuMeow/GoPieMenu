#pragma once

// =============================================================================
// GoPieMenu - List Menu Widget
// =============================================================================

#include "models/PieItem.h"
#include "models/StyleConfig.h"

#include <QWidget>
#include <QPropertyAnimation>
#include <vector>

namespace gpm 
{

class ListMenuWidget : public QWidget 
{
    Q_OBJECT
    Q_PROPERTY(qreal AnimProgress READ GetAnimProgress WRITE SetAnimProgress)

public:
    explicit ListMenuWidget(QWidget* Parent = nullptr);
    virtual ~ListMenuWidget() override;

    /** Open the list menu at a screen position */
    void ShowAt(const QPoint& InScreenPos, const std::vector<PieItem>& InItems, const StyleConfig& InStyle);
    
    /** Open the list menu projecting in a specific direction from an origin */
    void ShowAtDir(const QPoint& InOriginPos, double InAngle, const std::vector<PieItem>& InItems, const StyleConfig& InStyle);
    
    /** Close the menu with animation */
    void HideMenu();
    
    /** Update selection based on mouse movement */
    void UpdateMousePos(const QPoint& InScreenPos);
    
    /** Finalize selection and trigger action */
    int ConfirmSelection();

    // === Property Accessors ===
    [[nodiscard]] qreal GetAnimProgress() const { return AnimProgress; }
    void SetAnimProgress(qreal InValue);

    [[nodiscard]] bool IsOpen() const { return bIsOpen; }
    [[nodiscard]] int GetHoveredIndex() const { return HoveredIndex; }

signals:
    void ItemSelected(int Index, const PieItem& Item);
    void MenuClosed();

protected:
    virtual void paintEvent(QPaintEvent* Event) override;
    virtual void mouseMoveEvent(QMouseEvent* Event) override;
    virtual void mouseReleaseEvent(QMouseEvent* Event) override;
    virtual void keyPressEvent(QKeyEvent* Event) override;

private:
    // === Animation ===
    void StartOpenAnimation();
    void StartCloseAnimation();

    /** Calculate which item is at the given local position */
    [[nodiscard]] int GetItemAtPos(const QPoint& Pos) const;

    // === Data ===
    std::vector<PieItem> Items;
    StyleConfig          Style;

    // === State ===
    bool                 bIsOpen       = false;
    int                  HoveredIndex  = -1;
    qreal                AnimProgress  = 0.0;

    // === Configuration ===
    int                  ItemHeight    = 36;
    int                  MenuWidth     = 200;

    // === Widgets ===
    QPropertyAnimation*  OpenAnim      = nullptr;
    QPropertyAnimation*  CloseAnim     = nullptr;
};

} // namespace gpm
