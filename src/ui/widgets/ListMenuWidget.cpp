#include "ListMenuWidget.h"
#include <QGuiApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <cmath>

namespace gpm {
ListMenuWidget::ListMenuWidget(IconService* icons, QWidget* parent)
    : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus),
      Icons(icons), OpenAnimation(this, "AnimProgress"), CloseAnimation(this, "AnimProgress") {
    setAttribute(Qt::WA_TranslucentBackground); setAttribute(Qt::WA_ShowWithoutActivating);
    setMouseTracking(true); setFocusPolicy(Qt::NoFocus);
    connect(&CloseAnimation, &QPropertyAnimation::finished, this, [this] { hide(); emit MenuClosed(); });
    connect(Icons, &IconService::imageReady, this, [this](const QString& key, const QImage& image) {
        for (const auto& request : IconRequests.value(key)) Scene.setIcon(request.first, image, request.second);
        if (IconRequests.contains(key)) update();
    });
    connect(Icons, &IconService::invalidated, this, [this] { if (Open) requestIcons(); });
    EdgeScrollTimer.setInterval(180);
    connect(&EdgeScrollTimer, &QTimer::timeout, this, [this] { Scroll(-EdgeDirection * 120); });
}
void ListMenuWidget::Reset() {
    Open = false; OpenAnimation.stop(); CloseAnimation.stop(); EdgeScrollTimer.stop();
    Hovered = -1; hide();
}
void ListMenuWidget::ShowAt(const QPoint& pos, const std::vector<PieItem>& items, const StyleConfig& style) {
    ShowAtDir(pos, 0, items, style);
}
void ListMenuWidget::ShowAtDir(const QPoint& pos, double angle, const std::vector<PieItem>& items, const StyleConfig& style) {
    Reset();
    if (items.empty()) return;
    Scene.build(items, style); First = 0;
    auto* screen = QGuiApplication::screenAt(pos);
    if (!screen) screen = QGuiApplication::primaryScreen();
    const auto available = screen ? screen->availableGeometry() : QRect(0, 0, 800, 600);
    Rows = Scene.rowsForHeight(available.height() - 24);
    const auto size = (Scene.size(Rows) + QSizeF(20, 20)).toSize();
    int x = std::cos(angle) >= 0 ? pos.x() + 10 : pos.x() - size.width() - 10;
    if (x + size.width() > available.x() + available.width()) x = pos.x() - size.width() - 10;
    if (x < available.left()) x = pos.x() + 10;
    x = std::clamp(x, available.left(), std::max(available.left(), available.x() + available.width() - size.width()));
    const auto y = std::clamp(pos.y() - size.height() / 2, available.top(),
        std::max(available.top(), available.y() + available.height() - size.height()));
    setGeometry(x, y, size.width(), size.height());
    requestIcons(); Open = true; Progress = .01;
    OpenAnimation.setDuration(Scene.style().AnimationDuration);
    OpenAnimation.setStartValue(.01); OpenAnimation.setEndValue(1.0); OpenAnimation.setEasingCurve(QEasingCurve::OutCubic);
    show(); raise(); OpenAnimation.start(); update();
}
void ListMenuWidget::requestIcons() {
    IconRequests.clear();
    Scene.retainIcons(First, Rows);
    const int pixels = int(std::ceil(std::min(Scene.style().IconSize * .8, Scene.rowHeight() - 10) * devicePixelRatioF()));
    for (int i = First; i < std::min(First + Rows, Scene.count()); ++i) for (bool hover : {false, true}) {
        const auto key = Icons->request(Scene.items()[i].Icon, pixels, Scene.iconColor(i, hover));
        IconRequests[key].append({i, hover}); Scene.setIcon(i, Icons->cached(key), hover);
    }
}
void ListMenuWidget::HideMenu() {
    if (!Open) return;
    Open = false; EdgeScrollTimer.stop(); OpenAnimation.stop(); CloseAnimation.stop();
    CloseAnimation.setDuration(std::max(40, Scene.style().AnimationDuration / 2));
    CloseAnimation.setStartValue(Progress); CloseAnimation.setEndValue(0.0); CloseAnimation.start();
}
void ListMenuWidget::UpdateMousePos(const QPoint& pos) {
    if (!Open) return;
    LastMouse = pos;
    const QPointF point = mapFromGlobal(pos) - QPoint(10, 10);
    const int next = Scene.hitTest(point, First, Rows);
    if (next != Hovered) { Hovered = next; update(); }
    EdgeDirection = 0;
    if (point.x() >= 0 && point.x() <= Scene.size(Rows).width()) {
        if (point.y() >= 0 && point.y() < 8 && First > 0) EdgeDirection = -1;
        if (point.y() > Scene.size(Rows).height() - 8 && point.y() <= Scene.size(Rows).height() && First + Rows < Scene.count()) EdgeDirection = 1;
    }
    if (EdgeDirection && !EdgeScrollTimer.isActive()) EdgeScrollTimer.start();
    if (!EdgeDirection) EdgeScrollTimer.stop();
}
void ListMenuWidget::Scroll(int delta) {
    if (!Open || delta == 0) return;
    const int next = std::clamp(First - (delta > 0 ? 1 : -1), 0, std::max(0, Scene.count() - Rows));
    if (next == First) return;
    First = next; Hovered = -1; requestIcons(); UpdateMousePos(LastMouse); update();
}
int ListMenuWidget::ConfirmSelection() {
    if (!Open) return -1;
    const int selected = Hovered;
    std::optional<PieItem> item;
    if (selected >= 0 && selected < Scene.count()) item = Scene.items()[selected];
    HideMenu();
    if (item) emit ItemSelected(selected, *item);
    return item ? selected : -1;
}
void ListMenuWidget::SetAnimProgress(qreal value) { Progress = value; update(); }
bool ListMenuWidget::event(QEvent* event) {
    const bool handled = QWidget::event(event);
    if (event->type() == QEvent::DevicePixelRatioChange && Open) { requestIcons(); update(); }
    return handled;
}
void ListMenuWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this); painter.translate(10, 10);
    Scene.draw(painter, First, Rows, Hovered, Progress);
}
void ListMenuWidget::mouseMoveEvent(QMouseEvent* event) { UpdateMousePos(event->globalPosition().toPoint()); }
void ListMenuWidget::wheelEvent(QWheelEvent* event) { Scroll(event->angleDelta().y()); event->accept(); }
void ListMenuWidget::keyPressEvent(QKeyEvent* event) { if (event->key() == Qt::Key_Escape) HideMenu(); }
}
