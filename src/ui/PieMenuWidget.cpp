#include "PieMenuWidget.h"
#include <QMouseEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QKeyEvent>
#include <cmath>

namespace gpm {
PieMenuWidget::PieMenuWidget(IconService* icons, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus),
      Icons(icons), OpenAnimation(this, "AnimProgress"), CloseAnimation(this, "AnimProgress"),
      List(new ListMenuWidget(icons, this)) {
    setAttribute(Qt::WA_TranslucentBackground); setAttribute(Qt::WA_ShowWithoutActivating);
    setMouseTracking(true); setFocusPolicy(Qt::NoFocus);
    ListHideTimer.setSingleShot(true); ListHideTimer.setInterval(200);
    connect(&ListHideTimer, &QTimer::timeout, List, &ListMenuWidget::HideMenu);
    connect(&CloseAnimation, &QPropertyAnimation::finished, this, [this] {
        hide(); emit MenuClosed();
    });
    connect(List, &ListMenuWidget::ItemSelected, this, [this](int, const PieItem& item) {
        if (!Open) return;
        const auto copy = item;
        HideMenu(); // Consume the selection before dispatching external actions.
        emit ItemSelected(-1, copy);
    });
    connect(Icons, &IconService::imageReady, this, [this](const QString& key, const QImage& image) {
        for (const auto& request : IconRequests.value(key)) {
            if (request.second) Scene.setHoverIcon(request.first, image);
            else Scene.setIcon(request.first, image);
        }
        if (IconRequests.contains(key)) update();
    });
    connect(Icons, &IconService::invalidated, this, [this] { if (Open) requestIcons(); });
}
void PieMenuWidget::requestIcons() {
    IconRequests.clear();
    const auto& config = Scene.config();
    const int pixels = int(std::ceil(Scene.style().IconSize * devicePixelRatioF()));
    for (int i = 0; i < int(config.Items.size()); ++i) {
        for (bool hover : {false, true}) {
            const auto key = Icons->request(config.Items[i].Icon, pixels, Scene.iconColor(i, hover));
            IconRequests[key].append({i, hover});
            const auto image = Icons->cached(key);
            if (hover) Scene.setHoverIcon(i, image); else Scene.setIcon(i, image);
        }
    }
}
void PieMenuWidget::ShowAt(const QPoint& pos, const PieMenuConfig& config, const StyleConfig& global) {
    OpenAnimation.stop(); CloseAnimation.stop(); ListHideTimer.stop(); List->Reset();
    if (config.Items.empty()) { Open = false; hide(); return; }
    Scene.build(config, global);
    ScreenOrigin = pos; LastMouse = pos; Hovered = -1;
    auto* screen = QGuiApplication::screenAt(pos);
    if (!screen) screen = QGuiApplication::primaryScreen();
    Placement = PlaceMenu(Scene.bounds(), pos, screen ? screen->availableGeometry() : QRect(0, 0, 800, 600));
    setGeometry(Placement.Window);
    requestIcons();
    Open = true; Progress = .01;
    OpenAnimation.setDuration(Scene.style().AnimationDuration);
    OpenAnimation.setStartValue(.01); OpenAnimation.setEndValue(1.0);
    OpenAnimation.setEasingCurve(QEasingCurve::OutCubic);
    show(); raise(); OpenAnimation.start(); update();
}
void PieMenuWidget::HideMenu() {
    if (!Open) return;
    Open = false; // Closing windows no longer accept input.
    ListHideTimer.stop(); List->HideMenu(); OpenAnimation.stop();
    CloseAnimation.stop(); CloseAnimation.setDuration(std::max(40, Scene.style().AnimationDuration / 2));
    CloseAnimation.setStartValue(Progress); CloseAnimation.setEndValue(0.0);
    CloseAnimation.setEasingCurve(QEasingCurve::InCubic); CloseAnimation.start();
}
void PieMenuWidget::UpdateMousePos(const QPoint& screenPos) {
    if (!Open) return;
    LastMouse = screenPos;
    if (List->IsOpen()) {
        List->UpdateMousePos(screenPos);
        if (List->geometry().contains(screenPos)) { ListHideTimer.stop(); return; }
    }
    const auto relative = (QPointF(mapFromGlobal(screenPos)) - Placement.Origin) / Placement.Scale;
    const auto cursorDelta = screenPos - ScreenOrigin;
    const int next = std::hypot(cursorDelta.x(), cursorDelta.y()) < SafeZoneRadius ? -1 : Scene.hitTest(relative);
    if (next == Hovered) return;
    Hovered = next;
    if (Hovered >= 0 && Scene.config().Items[Hovered].Action == ActionType::ListMenu) {
        ListHideTimer.stop();
        const auto angle = Scene.angle(Hovered);
        const QPointF edge(std::cos(angle) * Scene.style().OuterRadius, -std::sin(angle) * Scene.style().OuterRadius);
        const auto position = (QPointF(pos()) + Placement.Origin + edge * Placement.Scale).toPoint();
        List->ShowAtDir(position, angle, Scene.config().Items[Hovered].SubItems, Scene.style());
    } else if (List->IsOpen()) ListHideTimer.start();
    update();
}
void PieMenuWidget::Scroll(int delta) { if (Open && List->IsOpen()) List->Scroll(delta); }
int PieMenuWidget::ConfirmSelection() {
    if (!Open) return -1;
    if (List->IsOpen() && List->GetHoveredIndex() >= 0) return List->ConfirmSelection();
    const auto delta = LastMouse - ScreenOrigin;
    const int selected = std::hypot(delta.x(), delta.y()) < SafeZoneRadius ? -1 : Hovered;
    std::optional<PieItem> action;
    if (selected >= 0 && selected < int(Scene.config().Items.size()) &&
        Scene.config().Items[selected].Action != ActionType::ListMenu) action = Scene.config().Items[selected];
    HideMenu();
    if (action) emit ItemSelected(selected, *action);
    return action ? selected : -1;
}
void PieMenuWidget::SetAnimProgress(qreal value) { Progress = value; update(); }
bool PieMenuWidget::event(QEvent* event) {
    const bool handled = QWidget::event(event);
    if (event->type() == QEvent::DevicePixelRatioChange && Open) { requestIcons(); update(); }
    return handled;
}
void PieMenuWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.translate(Placement.Origin);
    const auto scale = Placement.Scale * std::max(.01, Progress);
    painter.scale(scale, scale);
    Scene.draw(painter, Hovered, -1, Progress);
}
void PieMenuWidget::mouseMoveEvent(QMouseEvent* event) { UpdateMousePos(event->globalPosition().toPoint()); }
void PieMenuWidget::keyPressEvent(QKeyEvent* event) { if (event->key() == Qt::Key_Escape) HideMenu(); }
}
