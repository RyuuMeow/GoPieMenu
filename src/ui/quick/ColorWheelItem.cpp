#include "ColorWheelItem.h"
#include <QPainter>
#include <QMouseEvent>
#include <QCursor>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>
#include <numbers>

namespace gpm {
namespace { constexpr qreal Tau = std::numbers::pi * 2; }
ColorWheelItem::ColorWheelItem(QQuickItem* parent) : QQuickPaintedItem(parent) {
    setAntialiasing(true); setAcceptedMouseButtons(Qt::LeftButton);
    setCursor(Qt::CrossCursor);
}
QColor ColorWheelItem::color() const { return QColor::fromHsvF(Hue, Saturation, Value, Alpha); }
void ColorWheelItem::changed() { update(); emit colorChanged(); }
void ColorWheelItem::setColor(const QColor& color) {
    if (!color.isValid()) return;
    const auto h = color.hsvHueF();
    if (h >= 0) Hue = h; // Keep the chosen hue when passing through gray or black.
    Saturation = color.hsvSaturationF(); Value = color.valueF(); Alpha = color.alphaF();
    changed();
}
void ColorWheelItem::setHue(qreal value) { if (std::isfinite(value)) { Hue = std::fmod(std::max(0.0, value), 1.0); changed(); } }
void ColorWheelItem::setSaturation(qreal value) { if (std::isfinite(value)) { Saturation = std::clamp(value, 0.0, 1.0); changed(); } }
void ColorWheelItem::setValue(qreal value) { if (std::isfinite(value)) { Value = std::clamp(value, 0.0, 1.0); changed(); } }
void ColorWheelItem::setAlpha(qreal value) { if (std::isfinite(value)) { Alpha = std::clamp(value, 0.0, 1.0); changed(); } }
QString ColorWheelItem::hex() const {
    const auto c = color();
    auto text = c.name(QColor::HexRgb);
    if (c.alpha() < 255) text += QString::number(c.alpha(), 16).rightJustified(2, '0');
    return text.toUpper();
}
bool ColorWheelItem::setHex(const QString& text) {
    const auto value = text.trimmed();
    static const QRegularExpression pattern("^#?([0-9a-fA-F]{6})([0-9a-fA-F]{2})?$");
    const auto match = pattern.match(value);
    if (!match.hasMatch()) return false;
    QColor c("#" + match.captured(1));
    if (!match.captured(2).isEmpty()) c.setAlpha(match.captured(2).toInt(nullptr, 16));
    setColor(c); return true;
}
QRectF ColorWheelItem::square() const {
    const auto side = std::max(1.0, (std::min(width(), height()) / 2 - 40) * std::sqrt(2.0));
    return QRectF((width() - side) / 2, (height() - side) / 2, side, side);
}
void ColorWheelItem::paint(QPainter* painter) {
    painter->setRenderHint(QPainter::Antialiasing);
    const QPointF center(width()/2, height()/2);
    const auto radius = std::max(1.0, std::min(width(), height()) / 2 - 17);
    QConicalGradient hues(center, 0);
    for (int i = 0; i <= 12; ++i) hues.setColorAt(i / 12.0, QColor::fromHsvF((i % 12) / 12.0, 1, 1));
    painter->setPen(QPen(QBrush(hues), 22)); painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(center, radius, radius);
    const auto rect = square();
    painter->fillRect(rect, QColor::fromHsvF(Hue, 1, 1));
    QLinearGradient saturation(rect.topLeft(), rect.topRight());
    saturation.setColorAt(0, Qt::white); saturation.setColorAt(1, QColor(255,255,255,0));
    painter->fillRect(rect, saturation);
    QLinearGradient brightness(rect.topLeft(), rect.bottomLeft());
    brightness.setColorAt(0, QColor(0,0,0,0)); brightness.setColorAt(1, Qt::black);
    painter->fillRect(rect, brightness);
    painter->setPen(QPen(QColor("#c9d3e1"), 1)); painter->setBrush(Qt::NoBrush); painter->drawRect(rect);
    for (const auto& point : {center + QPointF(std::cos(Hue*Tau)*radius, -std::sin(Hue*Tau)*radius),
        QPointF(rect.left() + Saturation*rect.width(), rect.top() + (1-Value)*rect.height())}) {
        painter->setPen(QPen(QColor(0,0,0,170), 4)); painter->drawEllipse(point, 5, 5);
        painter->setPen(QPen(Qt::white, 2)); painter->drawEllipse(point, 5, 5);
    }
}
void ColorWheelItem::editAt(const QPointF& point) {
    if (Editing == Part::Hue) {
        Hue = std::atan2(height()/2 - point.y(), point.x() - width()/2) / Tau;
        if (Hue < 0) Hue += 1;
    } else if (Editing == Part::Square) {
        const auto rect = square();
        Saturation = std::clamp((point.x() - rect.left()) / rect.width(), 0.0, 1.0);
        Value = 1 - std::clamp((point.y() - rect.top()) / rect.height(), 0.0, 1.0);
    }
    changed();
}
void ColorWheelItem::mousePressEvent(QMouseEvent* event) {
    const auto point = event->position();
    const auto radius = std::hypot(point.x() - width()/2, point.y() - height()/2);
    const auto ringRadius = std::max(1.0, std::min(width(), height()) / 2 - 17);
    Editing = square().contains(point) ? Part::Square : std::abs(radius-ringRadius) <= 16 ? Part::Hue : Part::None;
    if (Editing == Part::None) { event->ignore(); return; }
    editAt(point); event->accept();
}
void ColorWheelItem::mouseMoveEvent(QMouseEvent* event) { if (Editing != Part::None) editAt(event->position()); event->accept(); }
void ColorWheelItem::mouseReleaseEvent(QMouseEvent* event) { if (Editing != Part::None) editAt(event->position()); Editing = Part::None; event->accept(); }
}
