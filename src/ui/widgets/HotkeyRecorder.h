#pragma once

// =============================================================================
// GoPieMenu - Hotkey Recorder Widget
// =============================================================================

#include "models/Types.h"

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QKeyEvent>

namespace gpm 
{

/**
 * Custom widget that captures a modifier key + mouse button or keyboard key.
 */
class HotkeyRecorder : public QWidget 
{
    Q_OBJECT

public:
    explicit HotkeyRecorder(QWidget* Parent = nullptr);

    // === Trigger API ===
    [[nodiscard]] TriggerDef GetTrigger() const;
    void SetTrigger(const TriggerDef& InTrigger);
    
    // === Raw Shortcut API ===
    [[nodiscard]] QString GetShortcutString() const;
    void SetShortcutString(const QString& InString);
    
    /** Restricts recorder to keyboard keys only (e.g. for action items) */
    void SetKeyboardOnlyMode(bool bInKeyboardOnly);

signals:
    void TriggerChanged(const TriggerDef& NewTrigger);

protected:
    virtual void keyPressEvent(QKeyEvent* Event) override;
    virtual void focusOutEvent(QFocusEvent* Event) override;

private:
    // === UI Logic ===
    void SetupUI();
    void UpdateVisibilities();
    void UpdateDisplay();
    
    // === Recording Lifecycle ===
    void StartRecording();
    void StopRecording(bool bInCanceled = false);

    // === State ===
    bool        bIsRecording = false;
    bool        bIsKeyboardOnly = false;
    uint32_t    CurrentVkCode = 0;
    ModifierKey CurrentMods = ModifierKey::None;
    QString     CurrentKeyString;

    // === UI Elements ===
    QHBoxLayout* ModeRow = nullptr;
    QButtonGroup* ModeGroup = nullptr;
    QPushButton*  BtnMouseHold = nullptr;
    QPushButton*  BtnKeyHold   = nullptr;
    QPushButton*  BtnKeyToggle = nullptr;

    QWidget*    MouseInputWidget = nullptr;
    QCheckBox*  CtrlCheck  = nullptr;
    QCheckBox*  ShiftCheck = nullptr;
    QCheckBox*  AltCheck   = nullptr;
    QCheckBox*  WinCheck   = nullptr;
    QComboBox*  MouseCombo = nullptr;

    QWidget*     KeyInputWidget = nullptr;
    QPushButton* RecordBtn      = nullptr;

    QLabel* DisplayLabel = nullptr;
    QLabel* DescLabel    = nullptr;
};

} // namespace gpm
