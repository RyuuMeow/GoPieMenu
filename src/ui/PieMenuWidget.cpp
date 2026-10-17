#include "PieMenuWidget.h"
#include <QMouseEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <cmath>

namespace gpm {
PieMenuWidget::PieMenuWidget(IconService* icons, QWidget* parent)
    : NonActivatingWidget(parent),
      Icons(icons), OpenAnimation(this, "AnimProgress"), CloseAnimation(this, "AnimProgress"),
      List(new ListMenuWidget(icons, this)), Context(new NonActivatingWidget(this)) {
    List->setObjectName("runtimeSubmenu");
    Context->setObjectName("persistentContextMenu");
    auto* layout = new QVBoxLayout(Context);
    layout->setContentsMargins(6, 6, 6, 6);
    auto* hideButton = new QPushButton(tr("Hide all persistent menus"), Context);
    hideButton->setObjectName("hidePersistentButton");
    hideButton->setFocusPolicy(Qt::NoFocus);
    hideButton->setStyleSheet("QPushButton { background: #ffffff; color: #29374b; border: 1px solid #e2e7ef; border-radius: 6px; padding: 10px 16px; } QPushButton:hover { background: #e2ebf8; }");
    layout->addWidget(hideButton);
    connect(hideButton, &QPushButton::clicked, this, [this] { Context->hide(); emit HidePersistentRequested(); });
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
        if (!Persistent) HideMenu(); // Transient selections are consumed before dispatch.
        else if (Scene.config().CollapseSubmenuOnClick) {
            List->Reset(); Expanded = -1;
            emit SubmenuChanged(QString()); update();
        }
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
void PieMenuWidget::ShowAt(const QPoint& pos, const PieMenuConfig& config, const StyleConfig& global, bool persistent) {
    ResetMenu();
    Persistent = persistent; List->SetPersistent(persistent);
    if (config.Items.empty()) { Open = false; hide(); return; }
    Scene.build(config, global);
    ScreenOrigin = pos; LastMouse = pos; Hovered = -1;
    auto* screen = QGuiApplication::screenAt(pos);
    if (!screen) screen = QGuiApplication::primaryScreen();
    Placement = PlaceMenu(Scene.bounds(), pos, screen ? screen->availableGeometry() : QRect(0, 0, 800, 600));
    setGeometry(Placement.Window);
    requestIcons();
    if (Persistent) ScreenOrigin = MenuCenter();
    Open = true; Progress = .01;
    OpenAnimation.setDuration(Scene.style().AnimationDuration);
    OpenAnimation.setStartValue(.01); OpenAnimation.setEndValue(1.0);
    OpenAnimation.setEasingCurve(QEasingCurve::OutCubic);
    show(); raise(); OpenAnimation.start(); update();
}
void PieMenuWidget::ResetMenu() {
    Open = false; OpenAnimation.stop(); CloseAnimation.stop(); ListHideTimer.stop(); List->Reset(); Context->hide();
    Pressed = -1; Expanded = -1; CenterPressed = false; Dragging = false; unsetCursor(); hide();
}
bool PieMenuWidget::inCenter(const QPoint& global) const {
    const auto point = (QPointF(mapFromGlobal(global)) - Placement.Origin) / Placement.Scale;
    return std::hypot(point.x(), point.y()) < Scene.style().InnerRadius;
}
void PieMenuWidget::showSubmenu(int index, bool moveOnly) {
    if (index < 0 || index >= int(Scene.config().Items.size())) return;
    const auto& item = Scene.config().Items[index];
    if (item.Action != ActionType::ListMenu) return;
    const auto angle = Scene.angle(index);
    const QPointF edge(std::cos(angle) * Scene.style().OuterRadius, -std::sin(angle) * Scene.style().OuterRadius);
    const auto position = (QPointF(pos()) + Placement.Origin + edge * Placement.Scale).toPoint();
    if (moveOnly) List->MoveToAnchor(position, angle);
    else List->ShowAtDir(position, angle, item.SubItems, Scene.style());
    Expanded = index;
}
void PieMenuWidget::RestoreSubmenu(const QString& id) {
    if (!Persistent || !Open || id.isEmpty()) return;
    for (int i = 0; i < int(Scene.config().Items.size()); ++i)
        if (Scene.config().Items[i].Id == id) { showSubmenu(i); update(); break; }
}
void PieMenuWidget::showContext(const QPoint& global) {
    Context->adjustSize();
    auto* screen = QGuiApplication::screenAt(global);
    if (!screen) screen = QGuiApplication::primaryScreen();
    Context->setGeometry(PlaceMenu(QRectF(QPointF(), Context->size()), global,
        screen->availableGeometry()).Window);
    Context->show(); Context->raise();
}
void PieMenuWidget::DismissContextOutside(const QPoint& global) {
    if (Context->isVisible() && !Context->geometry().contains(global)) Context->hide();
}
void PieMenuWidget::HideMenu() {
    if (!Open) return;
    Open = false; // Closing windows no longer accept input.
    Context->hide(); ListHideTimer.stop(); List->HideMenu(); OpenAnimation.stop();
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
    const int next = Persistent ? Scene.hitTest(relative, false) :
        (std::hypot(cursorDelta.x(), cursorDelta.y()) < SafeZoneRadius ? -1 : Scene.hitTest(relative));
    if (next == Hovered) return;
    Hovered = next;
    if (Persistent) { update(); return; }
    if (Hovered >= 0 && Scene.config().Items[Hovered].Action == ActionType::ListMenu) {
        ListHideTimer.stop();
        showSubmenu(Hovered);
    } else if (List->IsOpen()) ListHideTimer.start();
    update();
}
void PieMenuWidget::Scroll(int delta) { if (Open && List->IsOpen()) List->Scroll(delta); }
int PieMenuWidget::ConfirmSelection() {
    if (!Open || Persistent) return -1;
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
    const bool handled = NonActivatingWidget::event(event);
    if (event->type() == QEvent::DevicePixelRatioChange && Open) { requestIcons(); update(); }
    return handled;
}
void PieMenuWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.translate(Placement.Origin);
    const auto scale = Placement.Scale * std::max(.01, Progress);
    painter.scale(scale, scale);
    Scene.draw(painter, Hovered, Persistent ? Expanded : -1, Progress);
    if (Persistent) {
        painter.setPen(QPen(Scene.style().CenterDotColor, 2, Qt::SolidLine, Qt::RoundCap));
        for (int y : {-5, 0, 5}) painter.drawLine(QPointF(-7, y), QPointF(7, y));
    }
}
void PieMenuWidget::mouseMoveEvent(QMouseEvent* event) {
    const auto global = event->globalPosition().toPoint();
    if (Persistent && Open && CenterPressed) {
        const auto delta = global - PressPosition;
        if (!Dragging && delta.manhattanLength() < QApplication::startDragDistance()) return;
        Dragging = true; setCursor(Qt::ClosedHandCursor);
        auto* screen = QGuiApplication::screenAt(global);
        if (!screen) screen = QGuiApplication::primaryScreen();
        Placement = PlaceMenu(Scene.bounds(), DragOrigin + delta, screen->availableGeometry());
        setGeometry(Placement.Window); ScreenOrigin = MenuCenter();
        if (Expanded >= 0) showSubmenu(Expanded, true);
        return;
    }
    if (Persistent) setCursor(inCenter(global) ? Qt::OpenHandCursor : Qt::ArrowCursor);
    UpdateMousePos(global);
}
void PieMenuWidget::mousePressEvent(QMouseEvent* event) {
    if (!Open || !Persistent) return;
    Context->hide();
    const auto global = event->globalPosition().toPoint();
    if (event->button() == Qt::RightButton && inCenter(global)) { showContext(global); event->accept(); return; }
    if (event->button() != Qt::LeftButton) return;
    CenterPressed = inCenter(global); Dragging = false; PressPosition = global; DragOrigin = MenuCenter();
    UpdateMousePos(global); Pressed = CenterPressed ? -1 : Hovered; event->accept();
}
void PieMenuWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (!Open || !Persistent || event->button() != Qt::LeftButton) return;
    const bool center = CenterPressed, dragged = Dragging;
    CenterPressed = false; Dragging = false;
    const int pressed = Pressed; Pressed = -1;
    if (center) { if (dragged) emit PositionChanged(MenuCenter()); unsetCursor(); return; }
    UpdateMousePos(event->globalPosition().toPoint());
    if (pressed < 0 || pressed != Hovered) return;
    const auto item = Scene.config().Items[pressed];
    if (item.Action == ActionType::ListMenu) {
        if (Expanded == pressed) { List->Reset(); Expanded = -1; }
        else showSubmenu(pressed);
        emit SubmenuChanged(Expanded < 0 ? QString() : item.Id); update();
    } else emit ItemSelected(pressed, item);
    event->accept();
}
void PieMenuWidget::leaveEvent(QEvent*) { if (Persistent && !CenterPressed) { Hovered = -1; update(); } }
void PieMenuWidget::keyPressEvent(QKeyEvent* event) { if (!Persistent && event->key() == Qt::Key_Escape) HideMenu(); }
}
