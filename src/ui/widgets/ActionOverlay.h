#pragma once
#include "NonActivatingWidget.h"
#include "models/ActionOverlayConfig.h"
#include "ui/icons/IconService.h"
#include <QPropertyAnimation>
#include <QTimer>

namespace gpm {
class ActionOverlay : public NonActivatingWidget {
public:
    explicit ActionOverlay(IconService* icons);
    void present(QScreen* screen, const ActionOverlayConfig& settings, const QString& name, const QString& icon);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    IconService* Icons;
    QTimer Hold;
    QPropertyAnimation Fade;
    QString Text, IconKey;
    QImage Image;
    int IconSize = 0, Padding = 0, Alpha = 0;
};
}
