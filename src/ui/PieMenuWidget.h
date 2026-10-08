#pragma once
#include "rendering/MenuScene.h"
#include "icons/IconService.h"
#include "widgets/ListMenuWidget.h"
#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>

namespace gpm {
class PieMenuWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal AnimProgress READ GetAnimProgress WRITE SetAnimProgress)
public:
    explicit PieMenuWidget(IconService* icons, QWidget* parent = nullptr);
    void ShowAt(const QPoint& screenPos, const PieMenuConfig& config, const StyleConfig& globalStyle);
    void HideMenu();
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
protected:
    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    void requestIcons();
    IconService* Icons;
    PieScene Scene;
    QHash<QString, QList<QPair<int, bool>>> IconRequests;
    MenuPlacement Placement;
    QPoint ScreenOrigin, LastMouse;
    QPropertyAnimation OpenAnimation, CloseAnimation;
    ListMenuWidget* List;
    QTimer ListHideTimer;
    int Hovered = -1;
    bool Open = false;
    qreal Progress = 0;
    double SafeZoneRadius = 35;
};
}
