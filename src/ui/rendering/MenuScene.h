#pragma once
#include "models/PieMenuConfig.h"
#include <QPainter>
#include <QPainterPath>
#include <QImage>

namespace gpm {

StyleConfig SafeStyle(const StyleConfig& style);
QColor MenuTextColor(const StyleConfig& style, const QColor& background, bool hovered = false);

struct MenuPlacement {
    QRect Window;
    QPointF Origin;
    qreal Scale = 1;
};
// Works with negative monitor coordinates and a work area smaller than the menu.
MenuPlacement PlaceMenu(const QRectF& bounds, const QPointF& desiredCenter, const QRect& available);

class PieScene {
public:
    void build(const PieMenuConfig& config, const StyleConfig& globalStyle);
    void draw(QPainter& painter, int hovered = -1, int selected = -1, qreal progress = 1) const;
    int hitTest(const QPointF& point, bool extendDirections = true) const;
    double angle(int index) const;
    const QRectF& bounds() const { return Bounds; }
    const PieMenuConfig& config() const { return Config; }
    const StyleConfig& style() const { return Style; }
    void setIcon(int index, const QImage& image);
    QColor iconColor(int index, bool hovered = false) const;
private:
    struct Sector {
        QPainterPath Shape, Label, HoverLabel;
        QRectF IconRect;
        QImage Icon, HoverIcon;
        QColor Fill, Text, HoverText;
    };
    PieMenuConfig Config;
    StyleConfig Style;
    QVector<Sector> Sectors;
    QRectF Bounds;
public:
    void setHoverIcon(int index, const QImage& image);
};

class ListScene {
public:
    void build(const std::vector<PieItem>& items, const StyleConfig& style);
    QSizeF size(int visibleRows) const;
    void draw(QPainter& painter, int first, int visibleRows, int hovered = -1, qreal progress = 1) const;
    int hitTest(const QPointF& point, int first, int visibleRows) const;
    int rowsForHeight(qreal height) const;
    qreal rowHeight() const { return RowHeight; }
    int count() const { return int(Items.size()); }
    const std::vector<PieItem>& items() const { return Items; }
    const StyleConfig& style() const { return Style; }
    void setIcon(int index, const QImage& image, bool hover = false);
    void retainIcons(int first, int rows);
    QColor iconColor(int index, bool hovered = false) const;
private:
    StyleConfig Style;
    std::vector<PieItem> Items;
    QVector<QImage> Icons, HoverIcons;
    qreal RowHeight = 36, Width = 240;
};
}
