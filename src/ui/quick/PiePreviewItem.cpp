#include "PiePreviewItem.h"
#include <QQuickWindow>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>

namespace gpm {
PiePreviewItem::PiePreviewItem(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true); setAcceptHoverEvents(true); setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow* window) {
        if (window) connect(window, &QWindow::screenChanged, this, [this] { requestIcons(); update(); });
    });
}
void PiePreviewItem::setSession(EditorSession* session) {
    if (Session == session) return;
    if (Session) disconnect(Session, nullptr, this, nullptr);
    Session = session; Signature.clear();
    if (Session) connect(Session, &EditorSession::changed, this, &PiePreviewItem::rebuild);
    rebuild(); emit sessionChanged();
}
void PiePreviewItem::setIcons(IconService* icons) {
    if (Icons == icons) return;
    if (Icons) disconnect(Icons, nullptr, this, nullptr);
    Icons = icons;
    if (Icons) {
        connect(Icons, &IconService::imageReady, this, [this](const QString& key, const QImage& image) {
            for (const auto& r : Requests.value(key)) {
                if (ListMode) List.setIcon(r.first, image, r.second);
                else if (r.second) Pie.setHoverIcon(r.first, image);
                else Pie.setIcon(r.first, image);
            }
            if (Requests.contains(key)) update();
        });
        connect(Icons, &IconService::invalidated, this, [this] { requestIcons(); update(); });
    }
    requestIcons(); emit iconsChanged();
}
void PiePreviewItem::rebuild() {
    if (!Session || !Session->currentProfile()) { update(); return; }
    const auto* profile = Session->currentProfile();
    const auto signature = QJsonDocument(profile->ToJson()).toJson(QJsonDocument::Compact) +
        QJsonDocument(Session->effectiveStyle().ToJson()).toJson(QJsonDocument::Compact) + Session->folderId().toUtf8();
    if (signature == Signature) { update(); return; }
    Signature = signature;
    const bool wasList = ListMode;
    ListMode = !Session->folderId().isEmpty();
    Hovered = -1;
    if (ListMode) {
        const auto* folder = Session->findItem(Session->folderId());
        List.build(folder ? folder->SubItems : std::vector<PieItem>(), Session->effectiveStyle());
        Rows = List.rowsForHeight(std::max(80.0, height() - 64));
        First = wasList ? std::clamp(First, 0, std::max(0, List.count() - Rows)) : 0;
    } else { Pie.build(*profile, Session->effectiveStyle()); First = 0; }
    requestIcons(); emit hoveredNameChanged(); update();
}
void PiePreviewItem::requestIcons() {
    if (!Icons) return;
    Requests.clear();
    const auto dpr = window() ? window()->devicePixelRatio() : 1.0;
    if (ListMode) {
        List.retainIcons(First, Rows);
        const int size = int(std::ceil(std::min(List.style().IconSize * .8, List.rowHeight() - 10) * dpr));
        for (int i = First; i < std::min(First + Rows, List.count()); ++i) for (bool hover : {false, true}) {
            auto key = Icons->request(List.items()[i].Icon, size, List.iconColor(i, hover));
            Requests[key].append({i, hover}); List.setIcon(i, Icons->cached(key), hover);
        }
    } else {
        const int size = int(std::ceil(Pie.style().IconSize * dpr * 1.25));
        for (int i = 0; i < int(Pie.config().Items.size()); ++i) for (bool hover : {false, true}) {
            auto key = Icons->request(Pie.config().Items[i].Icon, size, Pie.iconColor(i, hover));
            Requests[key].append({i, hover});
            if (hover) Pie.setHoverIcon(i, Icons->cached(key)); else Pie.setIcon(i, Icons->cached(key));
        }
    }
}
QTransform PiePreviewItem::transform() const {
    QRectF bounds = ListMode ? QRectF(QPointF(), List.size(Rows)) : Pie.previewBounds();
    if (!ListMode) {
        const auto halfWidth = std::max(std::abs(bounds.left()), std::abs(bounds.right()));
        const auto halfHeight = std::max(std::abs(bounds.top()), std::abs(bounds.bottom()));
        bounds = QRectF(-halfWidth, -halfHeight, halfWidth * 2, halfHeight * 2);
    }
    if (bounds.isEmpty()) return {};
    const double scale = std::max(.05, std::min({ListMode ? 1.0 : 1.25,
        std::max(1.0, width() - 56) / bounds.width(), std::max(1.0, height() - 56) / bounds.height()}));
    QTransform t; t.translate(width() / 2, height() / 2);
    t.scale(scale, scale); t.translate(-bounds.center().x(), -bounds.center().y());
    return t;
}
void PiePreviewItem::paint(QPainter* p) {
    if (!Session || !Session->currentProfile()) return;
    // Preserve QQuickPaintedItem's device-pixel transform when adding our logical canvas transform.
    p->setTransform(transform(), true);
    if (ListMode) {
        int selected = Hovered;
        if (selected < 0) for (int i = 0; i < List.count(); ++i) if (List.items()[i].Id == Session->selectedId()) selected = i;
        List.draw(*p, First, Rows, selected);
    } else {
        int selected = -1;
        for (int i = 0; i < int(Pie.config().Items.size()); ++i) if (Pie.config().Items[i].Id == Session->selectedId()) selected = i;
        Pie.draw(*p, Hovered, selected);
    }
}
void PiePreviewItem::geometryChange(const QRectF& next, const QRectF& previous) {
    QQuickPaintedItem::geometryChange(next, previous);
    if (ListMode) {
        Rows = List.rowsForHeight(std::max(80.0, next.height() - 64));
        First = std::clamp(First, 0, std::max(0, List.count() - Rows)); requestIcons();
    }
    update();
}
int PiePreviewItem::hit(const QPointF& point) const {
    const auto logical = transform().inverted().map(point);
    // Editing selects visible sectors; the runtime uses the same hit test with directional extension.
    return ListMode ? List.hitTest(logical, First, Rows) : Pie.hitTest(logical, false);
}
void PiePreviewItem::hoverMoveEvent(QHoverEvent* event) {
    const auto next = hit(event->position());
    if (next != Hovered) { Hovered = next; emit hoveredNameChanged(); update(); }
    setCursor(Hovered >= 0 ? Qt::PointingHandCursor : Qt::ArrowCursor);
}
void PiePreviewItem::hoverLeaveEvent(QHoverEvent*) { Hovered = -1; emit hoveredNameChanged(); update(); }
QString PiePreviewItem::hoveredName() const {
    if (Hovered < 0) return {};
    if (ListMode) return Hovered < List.count() ? List.items()[Hovered].Name : QString();
    return Hovered < int(Pie.config().Items.size()) ? Pie.config().Items[Hovered].Name : QString();
}
void PiePreviewItem::mousePressEvent(QMouseEvent* event) {
    if (!Session) return;
    const auto index = hit(event->position());
    const auto id = index < 0 ? QString() : (ListMode ? List.items()[index].Id : Pie.config().Items[index].Id);
    if (event->button() == Qt::RightButton) {
        if (!id.isEmpty()) emit contextMenuRequested(id, event->position());
    } else Session->selectItem(id);
    event->accept();
}
void PiePreviewItem::mouseDoubleClickEvent(QMouseEvent* event) {
    if (!Session || ListMode || event->button() != Qt::LeftButton) return;
    const int index = hit(event->position());
    if (index >= 0 && Pie.config().Items[index].Action == ActionType::ListMenu) Session->enterFolder(Pie.config().Items[index].Id);
    event->accept();
}
void PiePreviewItem::wheelEvent(QWheelEvent* event) {
    if (!ListMode || !event->angleDelta().y()) return;
    First = std::clamp(First - (event->angleDelta().y() > 0 ? 1 : -1), 0, std::max(0, List.count() - Rows));
    Hovered = -1; requestIcons(); update(); event->accept();
}
QPointF PiePreviewItem::itemCenter(int index) const {
    if (ListMode) return transform().map(QPointF(List.size(Rows).width() / 2, 8 + (index - First + .5) * List.rowHeight()));
    const auto radius = (Pie.style().InnerRadius + Pie.style().OuterRadius) / 2;
    return transform().map(QPointF(std::cos(Pie.angle(index)) * radius, -std::sin(Pie.angle(index)) * radius));
}
}
