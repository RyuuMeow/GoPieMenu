#include "MenuScene.h"
#include <QFontMetricsF>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace gpm {
namespace {
constexpr auto Pi = std::numbers::pi;
double finiteClamp(double value, double low, double high, double fallback) {
    return std::isfinite(value) ? std::clamp(value, low, high) : fallback;
}
QColor outline(const QColor& text) { return text.lightnessF() > .5 ? QColor(0, 0, 0, 175) : QColor(255, 255, 255, 210); }
QPainterPath labelPath(const QString& text, const QFont& font, const QRectF& rect, Qt::Alignment alignment) {
    const QFontMetricsF metrics(font);
    const auto label = metrics.elidedText(text, Qt::ElideRight, rect.width());
    auto x = rect.left();
    if (alignment & Qt::AlignHCenter) x = rect.center().x() - metrics.horizontalAdvance(label) / 2;
    else if (alignment & Qt::AlignRight) x = rect.right() - metrics.horizontalAdvance(label);
    QPainterPath result;
    result.addText(x, rect.center().y() + (metrics.ascent() - metrics.descent()) / 2, font, label);
    return result;
}
}
StyleConfig SafeStyle(const StyleConfig& input) {
    auto s = input;
    s.OuterRadius = finiteClamp(s.OuterRadius, 80, 400, 150);
    s.InnerRadius = finiteClamp(s.InnerRadius, 10, s.OuterRadius - 9, 45);
    s.IconSize = finiteClamp(s.IconSize, 12, 96, 28);
    s.FontSize = finiteClamp(s.FontSize, 8, 32, 11);
    s.GapAngle = finiteClamp(s.GapAngle, 0, 30, 3);
    s.Opacity = finiteClamp(s.Opacity, .1, 1, .95);
    s.BorderWidth = finiteClamp(s.BorderWidth, 0, 8, 1);
    s.TextOutlineThickness = finiteClamp(s.TextOutlineThickness, 0, 10, 2);
    s.HoverScale = finiteClamp(s.HoverScale, 1, 1.25, 1.05);
    s.AnimationDuration = std::clamp(s.AnimationDuration, 0, 1000);
    if (!s.BackgroundColor.isValid()) s.BackgroundColor = StyleConfig::Frost().BackgroundColor;
    if (!s.SectorColor.isValid()) s.SectorColor = StyleConfig::Frost().SectorColor;
    if (!s.HoverColor.isValid()) s.HoverColor = StyleConfig::Frost().HoverColor;
    if (!s.BorderColor.isValid()) s.BorderColor = StyleConfig::Frost().BorderColor;
    if (!s.TextColor.isValid()) s.TextColor = StyleConfig::Frost().TextColor;
    if (!s.CenterColor.isValid()) s.CenterColor = StyleConfig::Frost().CenterColor;
    if (!s.CenterDotColor.isValid()) s.CenterDotColor = StyleConfig::Frost().CenterDotColor;
    return s;
}
QColor MenuTextColor(const StyleConfig& style, const QColor& background, bool hovered) {
    if (!style.bAutoContrast) return hovered ? QColor(Qt::white) : style.TextColor;
    const auto luminance = background.redF() * .299 + background.greenF() * .587 + background.blueF() * .114;
    return luminance > .52 ? QColor("#233044") : QColor("#f7f9fc");
}
MenuPlacement PlaceMenu(const QRectF& bounds, const QPointF& desiredCenter, const QRect& available) {
    const auto area = available.isEmpty() ? QRect(0, 0, 800, 600) : available;
    const auto safeBounds = bounds.isEmpty() ? QRectF(-40, -40, 80, 80) : bounds;
    const auto scale = std::min({1.0, std::max(1, area.width() - 8) / safeBounds.width(),
                                    std::max(1, area.height() - 8) / safeBounds.height()});
    QSize size(std::min(area.width(), int(std::ceil(safeBounds.width() * scale))),
               std::min(area.height(), int(std::ceil(safeBounds.height() * scale))));
    QPointF origin(-safeBounds.left() * scale, -safeBounds.top() * scale);
    auto pos = (desiredCenter - origin).toPoint();
    pos.setX(std::clamp(pos.x(), area.left(), std::max(area.left(), area.x() + area.width() - size.width())));
    pos.setY(std::clamp(pos.y(), area.top(), std::max(area.top(), area.y() + area.height() - size.height())));
    return {QRect(pos, size), origin, scale};
}
void PieScene::build(const PieMenuConfig& config, const StyleConfig& globalStyle) {
    Config = config; Style = SafeStyle(config.StyleOverride.value_or(globalStyle));
    Sectors.clear(); Sectors.resize(int(Config.Items.size()));
    const auto outer = Style.OuterRadius;
    const auto inner = Style.InnerRadius;
    const auto extent = outer * Style.HoverScale + Style.BorderWidth + 6;
    Bounds = QRectF(-extent, -extent, extent * 2, extent * 2);
    PreviewBounds = Bounds;
    if (Config.Items.empty()) return;
    const auto span = 2 * Pi / Config.Items.size();
    const auto gap = std::min(Style.GapAngle * Pi / 180, span * .8);
    QFont font(Style.FontFamily); font.setPointSizeF(Style.FontSize); font.setWeight(QFont::Medium);
    auto bold = font; bold.setWeight(QFont::DemiBold);
    for (int i = 0; i < Sectors.size(); ++i) {
        auto& sector = Sectors[i];
        const auto theta = angle(i);
        const auto start = (theta - span / 2 + gap / 2) * 180 / Pi;
        const auto sweep = (span - gap) * 180 / Pi;
        const QRectF outerRect(-outer, -outer, outer * 2, outer * 2), innerRect(-inner, -inner, inner * 2, inner * 2);
        sector.Shape.arcMoveTo(outerRect, start);
        sector.Shape.arcTo(outerRect, start, sweep);
        sector.Shape.arcTo(innerRect, start + sweep, -sweep);
        sector.Shape.closeSubpath();
        const auto iconRadius = (inner + outer) / 2;
        const QPointF iconPos(std::cos(theta) * iconRadius, -std::sin(theta) * iconRadius);
        sector.IconRect = QRectF(iconPos.x() - Style.IconSize / 2, iconPos.y() - Style.IconSize / 2, Style.IconSize, Style.IconSize);
        const QPointF labelPos(std::cos(theta) * (outer + 22), -std::sin(theta) * (outer + 22));
        const auto height = QFontMetricsF(bold).height() + 6;
        QRectF labelRect(labelPos.x() - 90, labelPos.y() - height / 2, 180, height);
        Qt::Alignment alignment = Qt::AlignHCenter;
        if (std::cos(theta) > .3) { labelRect.moveLeft(labelPos.x() + 6); alignment = Qt::AlignLeft; }
        else if (std::cos(theta) < -.3) { labelRect.moveRight(labelPos.x() - 6); alignment = Qt::AlignRight; }
        sector.Label = labelPath(Config.Items[i].Name, font, labelRect, alignment);
        sector.HoverLabel = labelPath(Config.Items[i].Name, bold, labelRect, alignment);
        sector.Fill = Config.Items[i].Color.value_or(Style.SectorColor);
        sector.Text = MenuTextColor(Style, sector.Fill);
        sector.HoverText = MenuTextColor(Style, Style.HoverColor, true);
        Bounds = Bounds.united(sector.HoverLabel.boundingRect().adjusted(-6, -6, 6, 6));
        // Reserve label slots rather than glyph extents so editing names cannot move or resize the preview.
        PreviewBounds = PreviewBounds.united(labelRect.adjusted(-6, -6, 6, 6));
    }
}
double PieScene::angle(int index) const { return Pi / 2 - index * 2 * Pi / std::max(size_t(1), Config.Items.size()); }
int PieScene::hitTest(const QPointF& point, bool extendDirections) const {
    if (Config.Items.empty()) return -1;
    const auto distance = std::hypot(point.x(), point.y());
    if (distance < Style.InnerRadius || (!extendDirections && distance > Style.OuterRadius * Style.HoverScale)) return -1;
    const auto theta = std::atan2(-point.y(), point.x());
    const auto norm = std::fmod(Pi / 2 - theta + 2 * Pi, 2 * Pi);
    const auto span = 2 * Pi / Config.Items.size();
    return int(std::floor(norm / span + .5)) % int(Config.Items.size());
}
QColor PieScene::iconColor(int index, bool hovered) const {
    if (index < 0 || index >= Sectors.size()) return Style.TextColor;
    return hovered ? Sectors[index].HoverText : Sectors[index].Text;
}
void PieScene::setIcon(int index, const QImage& image) { if (index >= 0 && index < Sectors.size()) Sectors[index].Icon = image; }
void PieScene::setHoverIcon(int index, const QImage& image) { if (index >= 0 && index < Sectors.size()) Sectors[index].HoverIcon = image; }
void PieScene::draw(QPainter& p, int hovered, int selected, qreal progress) const {
    p.save(); p.setRenderHint(QPainter::Antialiasing); p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setOpacity(p.opacity() * Style.Opacity * std::clamp(progress, 0.0, 1.0));
    for (int i = 0; i < Sectors.size(); ++i) {
        const auto& s = Sectors[i];
        const bool hover = i == hovered;
        p.save();
        if (hover) p.scale(Style.HoverScale, Style.HoverScale);
        p.setPen(QPen(i == selected ? QColor("#568bda") : Style.BorderColor, i == selected ? 2.2 : Style.BorderWidth));
        p.setBrush(hover ? Style.HoverColor : s.Fill);
        p.drawPath(s.Shape);
        const auto& image = hover && !s.HoverIcon.isNull() ? s.HoverIcon : s.Icon;
        if (!image.isNull()) p.drawImage(s.IconRect, image);
        p.restore();
        const auto& path = hover ? s.HoverLabel : s.Label;
        const auto text = hover ? s.HoverText : s.Text;
        if (Style.TextOutlineThickness > 0) {
            p.setPen(QPen(outline(text), Style.TextOutlineThickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            p.setBrush(Qt::NoBrush); p.drawPath(path);
        }
        p.setPen(Qt::NoPen); p.setBrush(text); p.drawPath(path);
    }
    const auto centerRadius = Style.InnerRadius - 4;
    p.setPen(Qt::NoPen); p.setBrush(Style.CenterColor);
    p.drawEllipse(QPointF(), centerRadius, centerRadius);
    if (hovered >= 0 && hovered < Sectors.size()) {
        const auto theta = angle(hovered);
        p.setBrush(Style.CenterDotColor);
        p.drawEllipse(QPointF(std::cos(theta) * centerRadius * .6, -std::sin(theta) * centerRadius * .6), 4.0, 4.0);
    }
    p.restore();
}

void ListScene::build(const std::vector<PieItem>& items, const StyleConfig& style) {
    Items = items; Style = SafeStyle(style);
    RowHeight = std::max(36.0, Style.FontSize * 2 + 12);
    Width = 252; Icons = QVector<QImage>(int(items.size())); HoverIcons = Icons;
}
QSizeF ListScene::size(int visibleRows) const { return {Width, std::max(0, visibleRows) * RowHeight + 16}; }
int ListScene::rowsForHeight(qreal height) const { return std::clamp(int((height - 16) / RowHeight), 1, std::max(1, count())); }
QColor ListScene::iconColor(int index, bool hovered) const {
    if (!Style.bAutoContrast && !hovered && index >= 0 && index < count()) return Items[index].Color.value_or(Style.TextColor);
    return MenuTextColor(Style, hovered ? Style.HoverColor : Style.BackgroundColor, hovered);
}
void ListScene::setIcon(int index, const QImage& image, bool hover) {
    if (index >= 0 && index < count()) (hover ? HoverIcons : Icons)[index] = image;
}
void ListScene::retainIcons(int first, int rows) {
    for (int i = 0; i < Icons.size(); ++i) if (i < first || i >= first + rows) {
        Icons[i] = {}; HoverIcons[i] = {};
    }
}
int ListScene::hitTest(const QPointF& point, int first, int visibleRows) const {
    if (point.x() < 4 || point.x() > Width - 4 || point.y() < 8 || point.y() >= 8 + visibleRows * RowHeight) return -1;
    const auto index = first + int((point.y() - 8) / RowHeight);
    return index >= 0 && index < count() ? index : -1;
}
void ListScene::draw(QPainter& p, int first, int visibleRows, int hovered, qreal progress) const {
    p.save(); p.setRenderHint(QPainter::Antialiasing); p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setOpacity(p.opacity() * Style.Opacity * std::clamp(progress, 0.0, 1.0));
    const QRectF bounds(QPointF(), size(visibleRows));
    p.setPen(QPen(Style.BorderColor, Style.BorderWidth)); p.setBrush(Style.BackgroundColor);
    p.drawRoundedRect(bounds, 10, 10);
    QFont font(Style.FontFamily); font.setPointSizeF(Style.FontSize); font.setWeight(QFont::Medium);
    for (int row = 0; row < visibleRows; ++row) {
        const int index = first + row;
        if (index < 0 || index >= count()) break;
        const bool hover = index == hovered;
        const QRectF rect(4, 8 + row * RowHeight, Width - 8, RowHeight);
        if (hover) {
            p.setPen(Qt::NoPen); p.setBrush(Style.HoverColor); p.drawRoundedRect(rect, 6, 6);
        }
        const auto iconSize = std::min(Style.IconSize * .8, RowHeight - 10);
        const QRectF iconRect(rect.left() + 10, rect.center().y() - iconSize / 2, iconSize, iconSize);
        const auto& image = hover && !HoverIcons[index].isNull() ? HoverIcons[index] : Icons[index];
        if (!image.isNull()) p.drawImage(iconRect, image);
        p.setFont(font); p.setPen(iconColor(index, hover));
        const QRectF textRect(iconRect.right() + 10, rect.top(), rect.right() - iconRect.right() - 20, rect.height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, QFontMetricsF(font).elidedText(Items[index].Name, Qt::ElideRight, textRect.width()));
    }
    // Scroll affordances remain inside the menu's existing padding.
    p.setPen(Qt::NoPen); p.setBrush(Style.CenterDotColor);
    if (first > 0) p.drawPolygon(QPolygonF({{Width / 2 - 4, 6}, {Width / 2 + 4, 6}, {Width / 2, 2}}));
    if (first + visibleRows < count()) {
        const auto bottom = bounds.bottom();
        p.drawPolygon(QPolygonF({{Width / 2 - 4, bottom - 6}, {Width / 2 + 4, bottom - 6}, {Width / 2, bottom - 2}}));
    }
    p.restore();
}
}
