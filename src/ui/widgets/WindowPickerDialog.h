#pragma once

// =============================================================================
// GoPieMenu - Window Picker Dialog
// =============================================================================

#include <QDialog>
#include <QListWidget>
#include <QString>
#include <QLineEdit>

namespace gpm 
{

class WindowPickerDialog : public QDialog 
{
    Q_OBJECT

public:
    explicit WindowPickerDialog(QWidget* Parent = nullptr);

    /** Returns the executable name of the selected window (e.g., "chrome.exe") */
    [[nodiscard]] QString GetSelectedProcessName() const 
    { 
        return SelectedProcessName; 
    }

private:
    void SetupUI();
    void ScanWindows();
    void FilterWindows(const QString& InText);

    QString      SelectedProcessName;
    QLineEdit*   SearchEdit = nullptr;
    QListWidget* ListWidget = nullptr;
};

} // namespace gpm
