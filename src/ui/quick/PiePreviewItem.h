#pragma once
#include <QQuickPaintedItem>
#include <QPointer>
#include "core/EditorSession.h"
#include "ui/icons/IconService.h"
#include "ui/rendering/MenuScene.h"

namespace gpm {
class PiePreviewItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(gpm::EditorSession* session READ session WRITE setSession NOTIFY sessionChanged)
    Q_PROPERTY(gpm::IconService* icons READ icons WRITE setIcons NOTIFY iconsChanged)
    Q_PROPERTY(QString hoveredName READ hoveredName NOTIFY hoveredNameChanged)
public:
    explicit PiePreviewItem(QQuickItem* parent = nullptr);
    EditorSession* session() const { return Session; }
    IconService* icons() const { return Icons; }
    void setSession(EditorSession* session);
    void setIcons(IconService* icons);
    QString hoveredName() const;
    void paint(QPainter* painter) override;
    Q_INVOKABLE QPointF itemCenter(int index) const;
signals:
    void sessionChanged();
    void iconsChanged();
    void hoveredNameChanged();
    void contextMenuRequested(const QString& itemId, const QPointF& position);
protected:
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;
    void hoverMoveEvent(QHoverEvent* event) override;
    void hoverLeaveEvent(QHoverEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
private:
    void rebuild();
    void requestIcons();
    QTransform transform() const;
    int hit(const QPointF& point) const;
    QPointer<EditorSession> Session;
    QPointer<IconService> Icons;
    PieScene Pie;
    ListScene List;
    QHash<QString, QList<QPair<int, bool>>> Requests;
    QByteArray Signature;
    bool ListMode = false;
    int First = 0, Rows = 1, Hovered = -1;
};
}
