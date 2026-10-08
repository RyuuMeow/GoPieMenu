#pragma once
#include <QQuickPaintedItem>
#include <QColor>

namespace gpm {
class ColorWheelItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(qreal hue READ hue WRITE setHue NOTIFY colorChanged)
    Q_PROPERTY(qreal saturation READ saturation WRITE setSaturation NOTIFY colorChanged)
    Q_PROPERTY(qreal value READ value WRITE setValue NOTIFY colorChanged)
    Q_PROPERTY(qreal alpha READ alpha WRITE setAlpha NOTIFY colorChanged)
    Q_PROPERTY(QString hex READ hex NOTIFY colorChanged)
public:
    explicit ColorWheelItem(QQuickItem* parent = nullptr);
    QColor color() const;
    void setColor(const QColor& color);
    qreal hue() const { return Hue; }
    qreal saturation() const { return Saturation; }
    qreal value() const { return Value; }
    qreal alpha() const { return Alpha; }
    void setHue(qreal value);
    void setSaturation(qreal value);
    void setValue(qreal value);
    void setAlpha(qreal value);
    QString hex() const;
    Q_INVOKABLE bool setHex(const QString& text);
    void paint(QPainter* painter) override;
signals:
    void colorChanged();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
private:
    QRectF square() const;
    void editAt(const QPointF& point);
    void changed();
    qreal Hue = .6, Saturation = .65, Value = .8, Alpha = 1;
    enum class Part { None, Hue, Square } Editing = Part::None;
};
}
