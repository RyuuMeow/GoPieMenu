#pragma once

// =============================================================================
// GoPieMenu - Icon Picker Dialog
// =============================================================================

#include <QDialog>
#include <QListWidget>
#include <QString>
#include <QPushButton>
#include <QLineEdit>
#include <QImage>

namespace gpm 
{

class IconPickerDialog : public QDialog 
{
    Q_OBJECT

public:
    explicit IconPickerDialog(const QString& InIconFolderPath, QWidget* Parent = nullptr);

    /** Returns the absolute path to the selected icon file, or empty if none */
    [[nodiscard]] QString GetSelectedIconPath() const 
    { 
        return SelectedIconPath; 
    }

private:
    void SetupUI();
    void ScanIcons();
    void FilterIcons(const QString& InText);

    QString      IconDir;
    QString      SelectedIconPath;
    QLineEdit*   SearchEdit = nullptr;
    QListWidget* ListWidget = nullptr;
};

} // namespace gpm
