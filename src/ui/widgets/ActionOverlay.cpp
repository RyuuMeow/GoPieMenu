#include "ActionOverlay.h"
#include <QPainter>
#include <QScreen>

namespace gpm {
ActionOverlay::ActionOverlay(IconService* icons) : Icons(icons), Fade(this, "windowOpacity") {
    setWindowFlag(Qt::WindowTransparentForInput);
    setAttribute(Qt::WA_QuitOnClose, false);
    Hold.setSingleShot(true);
    Fade.setDuration(180);
    Fade.setStartValue(1.0);
    Fade.setEndValue(0.0);
    connect(&Hold, &QTimer::timeout, this, [this] { Fade.start(); });
    connect(&Fade, &QPropertyAnimation::finished, this, &QWidget::hide);
    connect(Icons, &IconService::imageReady, this, [this](const QString& key, const QImage& image) {
        if (key == IconKey) { Image = image; update(); }
    });
}

void ActionOverlay::present(QScreen* screen, const ActionOverlayConfig& settings, const QString& name, const QString& icon) {
    if (!screen) return;
    Hold.stop();
    Fade.stop();
    setScreen(screen);
    setWindowOpacity(1.0);
    const QRect area = screen->availableGeometry();
    const int margin = qMin(settings.margin, qMin(area.width(), area.height()) / 4);
    const QRect bounds = area.adjusted(margin, margin, -margin, -margin);
    QFont f = font();
    f.setPixelSize(15 + settings.size * 4);
    setFont(f);
    Padding = 10 + settings.size * 3;
    IconSize = icon.isEmpty() ? 0 : f.pixelSize() + 6;
    IconKey = icon.isEmpty() ? QString() : Icons->request(icon, qRound(IconSize * screen->devicePixelRatio()), Qt::white);
    Image = Icons->cached(IconKey);
    const int fixedWidth = 2 * Padding + (IconSize ? IconSize + Padding : 0);
    Text = fontMetrics().elidedText(name.simplified(), Qt::ElideRight, qMax(0, bounds.width() - fixedWidth));
    resize(qMin(bounds.width(), fixedWidth + fontMetrics().horizontalAdvance(Text)),
           qMax(fontMetrics().height(), IconSize) + 2 * Padding);
    Alpha = qRound(settings.opacity * 2.55);
    const int column = settings.position % 3;
    move(bounds.left() + column * (bounds.width() - width()) / 2,
         settings.position < 3 ? bounds.top() : bounds.bottom() + 1 - height());
    update();
    show();
    Hold.start(settings.durationMs);
}

void ActionOverlay::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, Alpha));
    p.drawRoundedRect(rect(), 10, 10);
    if (!Image.isNull()) p.drawImage(QRect(Padding, (height()-IconSize)/2, IconSize, IconSize), Image);
    const int left = Padding + (IconSize ? IconSize + Padding : 0);
    p.setPen(Qt::white);
    p.drawText(rect().adjusted(left, 0, -Padding, 0), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, Text);
}
}
