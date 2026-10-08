#pragma once
#include "ui/rendering/MenuScene.h"
#include "ui/icons/IconService.h"
#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>

namespace gpm {
class ListMenuWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal AnimProgress READ GetAnimProgress WRITE SetAnimProgress)
public:
    explicit ListMenuWidget(IconService* icons, QWidget* parent = nullptr);
    void ShowAt(const QPoint& pos, const std::vector<PieItem>& items, const StyleConfig& style);
    void ShowAtDir(const QPoint& pos, double angle, const std::vector<PieItem>& items, const StyleConfig& style);
    void HideMenu();
    void Reset();
    void UpdateMousePos(const QPoint& pos);
    void Scroll(int delta);
    int ConfirmSelection();
    bool IsOpen() const { return Open; }
    int GetHoveredIndex() const { return Hovered; }
    qreal GetAnimProgress() const { return Progress; }
    void SetAnimProgress(qreal value);
signals:
    void ItemSelected(int index, const PieItem& item);
    void MenuClosed();
protected:
    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
private:
    void requestIcons();
    IconService* Icons;
    ListScene Scene;
    QHash<QString, QList<QPair<int, bool>>> IconRequests;
    QPropertyAnimation OpenAnimation, CloseAnimation;
    QTimer EdgeScrollTimer;
    QPoint LastMouse;
    int First = 0, Rows = 0, Hovered = -1, EdgeDirection = 0;
    bool Open = false;
    qreal Progress = 0;
};
}
