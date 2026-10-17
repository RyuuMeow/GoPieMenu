#pragma once
#include "rendering/MenuScene.h"
#include "icons/IconService.h"
#include "widgets/ListMenuWidget.h"
#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>

namespace gpm {
class PieMenuWidget : public NonActivatingWidget {
    Q_OBJECT
    Q_PROPERTY(qreal AnimProgress READ GetAnimProgress WRITE SetAnimProgress)
public:
    explicit PieMenuWidget(IconService* icons, QWidget* parent = nullptr);
    void ShowAt(const QPoint& screenPos, const PieMenuConfig& config, const StyleConfig& globalStyle, bool persistent = false);
    void HideMenu();
    void ResetMenu();
    void RestoreSubmenu(const QString& id);
    void DismissContextOutside(const QPoint& global);
    QPoint MenuCenter() const { return pos() + Placement.Origin.toPoint(); }
    bool IsPersistent() const { return Persistent; }
    void UpdateMousePos(const QPoint& screenPos);
    void Scroll(int delta);
    int ConfirmSelection();
    qreal GetAnimProgress() const { return Progress; }
    void SetAnimProgress(qreal value);
    bool IsOpen() const { return Open; }
    void SetSafeZoneRadius(double radius) { SafeZoneRadius = radius; }
    double GetSafeZoneRadius() const { return SafeZoneRadius; }
signals:
    void ItemSelected(int index, const PieItem& item);
    void MenuClosed();
    void PositionChanged(const QPoint& center);
    void SubmenuChanged(const QString& id);
    void HidePersistentRequested();
protected:
    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent*) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    void requestIcons();
    void showSubmenu(int index, bool moveOnly = false);
    bool inCenter(const QPoint& global) const;
    void showContext(const QPoint& global);
    IconService* Icons;
    PieScene Scene;
    QHash<QString, QList<QPair<int, bool>>> IconRequests;
    MenuPlacement Placement;
    QPoint ScreenOrigin, LastMouse;
    QPropertyAnimation OpenAnimation, CloseAnimation;
    ListMenuWidget* List;
    NonActivatingWidget* Context;
    QTimer ListHideTimer;
    int Hovered = -1;
    bool Open = false;
    bool Persistent = false, CenterPressed = false, Dragging = false;
    int Pressed = -1, Expanded = -1;
    QPoint PressPosition, DragOrigin;
    qreal Progress = 0;
    double SafeZoneRadius = 35;
};
}
