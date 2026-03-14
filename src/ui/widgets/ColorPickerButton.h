#pragma once

// =============================================================================
// GoPieMenu - Color Picker Button
// =============================================================================

#include <QPushButton>
#include <QColor>
#include <QColorDialog>

namespace gpm 
{

class ColorPickerButton : public QPushButton 
{
    Q_OBJECT

public:
    explicit ColorPickerButton(QWidget* Parent = nullptr)
        : QPushButton(Parent)
        , Color(Qt::gray)
    {
        setFixedSize(80, 28);
        UpdateStyle();
        
        connect(this, &QPushButton::clicked, this, [this]() 
        {
            QColor StartColor = (Color.alpha() == 0) ? QColor(255, 255, 255, 255) : Color;
            QColorDialog Dlg(StartColor, this);
            Dlg.setOption(QColorDialog::ShowAlphaChannel);
            Dlg.setOption(QColorDialog::DontUseNativeDialog);
            Dlg.setWindowTitle(QStringLiteral("Select Color"));
            Dlg.setStyleSheet(QStringLiteral(
                "QDialog { background: #1a1a28; color: #d0d0e8; }"
                "QLabel { color: #d0d0e8; }"
                "QPushButton { background: #2a2a40; color: #d0d0e8; border: 1px solid #3a3a50; border-radius: 4px; padding: 4px 12px; min-width: 60px; }"
                "QPushButton:hover { background: #3a3a50; border-color: #4f46e5; }"
                "QLineEdit { background: #131320; color: #e0e0f0; border: 1px solid #2a2a40; border-radius: 4px; padding: 2px 4px; }"
                "QSpinBox { background: #131320; color: #e0e0f0; border: 1px solid #2a2a40; border-radius: 4px; padding: 2px 4px; }"
            ));
            
            if (Dlg.exec() == QDialog::Accepted) 
            {
                QColor NewColor = Dlg.currentColor();
                if (NewColor.isValid()) 
                {
                    Color = NewColor;
                    UpdateStyle();
                    emit ColorChanged(Color);
                }
            }
        });
    }

    [[nodiscard]] QColor GetColor() const 
    { 
        return Color; 
    }

    void SetColor(const QColor& InColor) 
    {
        Color = InColor;
        UpdateStyle();
    }

signals:
    void ColorChanged(const QColor& NewColor);

private:
    void UpdateStyle() 
    {
        if (Color.alpha() == 0) 
        {
            setStyleSheet(QStringLiteral(
                "QPushButton { background: transparent; border: 2px dashed #555; border-radius: 4px; color: #888; }"
                "QPushButton:hover { border-color: #89b4fa; color: #d0d0e8; }"
            ));
            setText(QStringLiteral("Default"));
        } 
        else 
        {
            // Determine text color based on background brightness for readability
            int Brightness = (Color.red() * 299 + Color.green() * 587 + Color.blue() * 114) / 1000;
            QString TextColorHex = (Brightness > 125) ? "#000" : "#fff";

            setStyleSheet(QStringLiteral(
                "QPushButton { background: %1; border: 2px solid #555; border-radius: 4px; color: %2; }"
                "QPushButton:hover { border-color: #89b4fa; }"
            ).arg(Color.name(QColor::HexArgb), TextColorHex));
            setText(Color.name(QColor::HexArgb));
        }
    }

    QColor Color;
};

} // namespace gpm
