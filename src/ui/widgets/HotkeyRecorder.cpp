// =============================================================================
// GoPieMenu - Hotkey Recorder Implementation
// =============================================================================

#include "HotkeyRecorder.h"

#include <QKeySequence>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace gpm 
{

HotkeyRecorder::HotkeyRecorder(QWidget* Parent)
    : QWidget(Parent)
{
    setFocusPolicy(Qt::StrongFocus);
    SetupUI();
    UpdateVisibilities();
}

void HotkeyRecorder::SetupUI()
{
    auto* RootLayoutPointer = new QVBoxLayout(this);
    RootLayoutPointer->setContentsMargins(0, 0, 0, 0);
    RootLayoutPointer->setSpacing(8);

    // === Mode Selection Row ===
    ModeRow = new QHBoxLayout;
    ModeGroup = new QButtonGroup(this);
    ModeGroup->setExclusive(true);

    auto CreateModeBtn = [](const QString& InText, int InId, QButtonGroup* InGroup) 
    {
        auto* NewBtn = new QPushButton(InText);
        NewBtn->setCheckable(true);
        NewBtn->setCursor(Qt::PointingHandCursor);
        NewBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #131320; color: #8888aa; border: 1px solid #1e1e30; border-radius: 6px; padding: 6px 12px; font-weight: 600; }"
            "QPushButton:checked { background: #4f46e5; color: #ffffff; border-color: #5b52f0; }"
            "QPushButton:hover:!checked { background: #1a1a2e; color: #a0a0c8; }"
        ));
        InGroup->addButton(NewBtn, InId);
        return NewBtn;
    };

    BtnMouseHold = CreateModeBtn(QStringLiteral("Mouse Hold"), 0, ModeGroup);
    BtnKeyHold   = CreateModeBtn(QStringLiteral("Key Hold"),   1, ModeGroup);
    BtnKeyToggle = CreateModeBtn(QStringLiteral("Key Toggle"), 2, ModeGroup);
    
    BtnMouseHold->setChecked(true);
    ModeRow->addWidget(BtnMouseHold);
    ModeRow->addWidget(BtnKeyHold);
    ModeRow->addWidget(BtnKeyToggle);
    ModeRow->addStretch();
    RootLayoutPointer->addLayout(ModeRow);

    // === Input Area Row ===
    auto* InputRow = new QHBoxLayout;
    InputRow->setSpacing(12);

    // 1. Mouse Input
    MouseInputWidget = new QWidget(this);
    auto* MouseBox = new QHBoxLayout(MouseInputWidget);
    MouseBox->setContentsMargins(0, 0, 0, 0);
    CtrlCheck  = new QCheckBox(QStringLiteral("Ctrl"), this);
    ShiftCheck = new QCheckBox(QStringLiteral("Shift"), this);
    AltCheck   = new QCheckBox(QStringLiteral("Alt"), this);
    WinCheck   = new QCheckBox(QStringLiteral("Win"), this);
    MouseCombo = new QComboBox(this);
    MouseCombo->addItem(QStringLiteral("Right Click"),  static_cast<int>(MouseButton::Right));
    MouseCombo->addItem(QStringLiteral("Middle Click"), static_cast<int>(MouseButton::Middle));
    MouseCombo->addItem(QStringLiteral("X1 Click"),     static_cast<int>(MouseButton::X1));
    MouseCombo->addItem(QStringLiteral("X2 Click"),     static_cast<int>(MouseButton::X2));
    MouseBox->addWidget(CtrlCheck);
    MouseBox->addWidget(ShiftCheck);
    MouseBox->addWidget(AltCheck);
    MouseBox->addWidget(WinCheck);
    MouseBox->addWidget(new QLabel(QStringLiteral("+"), this));
    MouseBox->addWidget(MouseCombo);

    // 2. Keyboard Input (Custom Key Record)
    KeyInputWidget = new QWidget(this);
    auto* KeyBox = new QHBoxLayout(KeyInputWidget);
    KeyBox->setContentsMargins(0, 0, 0, 0);
    RecordBtn = new QPushButton(QStringLiteral("Click to record shortcut..."), this);
    RecordBtn->setCursor(Qt::PointingHandCursor);
    RecordBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1e1e35; color: #c0c0d8; border: 1px dashed #4f46e5; border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
        "QPushButton:hover { background: #2a2a40; }"
    ));
    connect(RecordBtn, &QPushButton::clicked, this, &HotkeyRecorder::StartRecording);
    KeyBox->addWidget(RecordBtn);

    InputRow->addWidget(MouseInputWidget);
    InputRow->addWidget(KeyInputWidget);

    // Display Label (e.g. "Ctrl+Right")
    DisplayLabel = new QLabel(this);
    DisplayLabel->setStyleSheet(QStringLiteral(
        "color: #89b4fa; font-weight: bold; padding: 4px 12px; "
        "background: #313150; border-radius: 4px; min-width: 80px; text-align: center;"));
    DisplayLabel->setAlignment(Qt::AlignCenter);
    InputRow->addWidget(DisplayLabel);
    InputRow->addStretch();
    RootLayoutPointer->addLayout(InputRow);

    // === Description Row ===
    DescLabel = new QLabel(this);
    DescLabel->setStyleSheet(QStringLiteral("color: #5a5a7a; font-size: 11px; margin-top: 2px;"));
    RootLayoutPointer->addWidget(DescLabel);

    // Signals
    auto OnChange = [this]() 
    {
        UpdateVisibilities();
        UpdateDisplay();
        emit TriggerChanged(GetTrigger());
    };

    connect(ModeGroup, &QButtonGroup::idToggled, this, [OnChange](int, bool bChecked) 
    {
        if (bChecked) 
        {
            OnChange();
        }
    });
    connect(CtrlCheck,  &QCheckBox::toggled, this, OnChange);
    connect(ShiftCheck, &QCheckBox::toggled, this, OnChange);
    connect(AltCheck,   &QCheckBox::toggled, this, OnChange);
    connect(WinCheck,   &QCheckBox::toggled, this, OnChange);
    connect(MouseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, OnChange);
}

void HotkeyRecorder::UpdateVisibilities()
{
    if (bIsKeyboardOnly) 
    {
        if (ModeRow) 
        {
            for (int i = 0; i < ModeRow->count(); ++i) 
            {
                if (auto* Item = ModeRow->itemAt(i)) 
                {
                    if (Item->widget()) 
                    {
                        Item->widget()->hide();
                    }
                }
            }
        }
        MouseInputWidget->hide();
        KeyInputWidget->show();
        DescLabel->hide();
        return;
    }
    
    if (ModeRow) 
    {
        for (int i = 0; i < ModeRow->count(); ++i) 
        {
            if (auto* Item = ModeRow->itemAt(i)) 
            {
                if (Item->widget()) 
                {
                    Item->widget()->show();
                }
            }
        }
    }
    DescLabel->show();

    int CurrentModeId = ModeGroup->checkedId();
    if (CurrentModeId == 0) 
    {
        MouseInputWidget->show();
        KeyInputWidget->hide();
        DescLabel->setText(QStringLiteral("Hold the assigned mouse button to open the menu, move cursor, and release to execute."));
    } 
    else 
    {
        MouseInputWidget->hide();
        KeyInputWidget->show();
        if (CurrentModeId == 1) 
        {
            DescLabel->setText(QStringLiteral("Hold the assigned keyboard key to open the menu, move cursor, and release the key to execute."));
        } 
        else 
        {
            DescLabel->setText(QStringLiteral("Press the assigned keyboard key to open the menu. Click on an item to execute."));
        }
    }
}

void HotkeyRecorder::SetKeyboardOnlyMode(bool bInKeyboardOnly)
{
    bIsKeyboardOnly = bInKeyboardOnly;
    // Force mode to 2 (KeyToggle) logic so it acts naturally like a normal key recorder without hold/release context
    if (bIsKeyboardOnly) 
    {
        BtnKeyToggle->setChecked(true);
    }
    UpdateVisibilities();
}

TriggerDef HotkeyRecorder::GetTrigger() const
{
    TriggerDef NewTrigger;
    int CurrentModeId = ModeGroup->checkedId();
    if (CurrentModeId == 1) 
    {
        NewTrigger.Mode = ActivationMode::KeyHold;
    }
    else if (CurrentModeId == 2) 
    {
        NewTrigger.Mode = ActivationMode::KeyToggle;
    }
    else 
    {
        NewTrigger.Mode = ActivationMode::MouseHold;
    }

    if (NewTrigger.Mode == ActivationMode::MouseHold) 
    {
        ModifierKey LocalMods = ModifierKey::None;
        if (CtrlCheck->isChecked())  LocalMods = LocalMods | ModifierKey::Ctrl;
        if (ShiftCheck->isChecked()) LocalMods = LocalMods | ModifierKey::Shift;
        if (AltCheck->isChecked())   LocalMods = LocalMods | ModifierKey::Alt;
        if (WinCheck->isChecked())   LocalMods = LocalMods | ModifierKey::Win;
        NewTrigger.Modifiers = LocalMods;
        NewTrigger.Button    = static_cast<MouseButton>(MouseCombo->currentData().toInt());
        NewTrigger.VKCode    = 0;
    } 
    else 
    {
        NewTrigger.Modifiers = CurrentMods;
        NewTrigger.VKCode    = CurrentVkCode;
        NewTrigger.Button    = MouseButton::Right; // fallback
    }

    return NewTrigger;
}

QString HotkeyRecorder::GetShortcutString() const
{
    QStringList StringParts;
    if (HasModifier(CurrentMods, ModifierKey::Ctrl))  StringParts << QStringLiteral("Ctrl");
    if (HasModifier(CurrentMods, ModifierKey::Win))   StringParts << QStringLiteral("Win");
    if (HasModifier(CurrentMods, ModifierKey::Alt))   StringParts << QStringLiteral("Alt");
    if (HasModifier(CurrentMods, ModifierKey::Shift)) StringParts << QStringLiteral("Shift");
    if (!CurrentKeyString.isEmpty()) 
    {
        StringParts << CurrentKeyString;
    }
    return StringParts.join('+');
}

void HotkeyRecorder::SetShortcutString(const QString& InString)
{
    // A primitive parser to map string back to UI state
    CurrentMods = ModifierKey::None;
    CurrentKeyString.clear();
    CurrentVkCode = 0;
    
    auto StringParts = InString.split('+', Qt::SkipEmptyParts);
    for (auto& Part : StringParts) 
    {
        auto TrimmedPart = Part.trimmed();
        auto LowerPart = TrimmedPart.toLower();
        if (LowerPart == "ctrl" || LowerPart == "control")      CurrentMods = CurrentMods | ModifierKey::Ctrl;
        else if (LowerPart == "shift")                          CurrentMods = CurrentMods | ModifierKey::Shift;
        else if (LowerPart == "alt")                            CurrentMods = CurrentMods | ModifierKey::Alt;
        else if (LowerPart == "win")                            CurrentMods = CurrentMods | ModifierKey::Win;
        else 
        {
            CurrentKeyString = TrimmedPart;
            // Best effort vkCode mapping so GetTrigger() still returns something
            QKeySequence Sequence(TrimmedPart);
            if (Sequence.count() > 0) 
            {
                CurrentVkCode = Sequence[0].key() & ~Qt::KeyboardModifierMask;
            }
        }
    }
    UpdateDisplay();
}

void HotkeyRecorder::SetTrigger(const TriggerDef& InNewTrigger)
{
    if (InNewTrigger.Mode == ActivationMode::KeyHold)       BtnKeyHold->setChecked(true);
    else if (InNewTrigger.Mode == ActivationMode::KeyToggle) BtnKeyToggle->setChecked(true);
    else                                          BtnMouseHold->setChecked(true);

    CtrlCheck->setChecked(HasModifier(InNewTrigger.Modifiers, ModifierKey::Ctrl));
    ShiftCheck->setChecked(HasModifier(InNewTrigger.Modifiers, ModifierKey::Shift));
    AltCheck->setChecked(HasModifier(InNewTrigger.Modifiers, ModifierKey::Alt));
    WinCheck->setChecked(HasModifier(InNewTrigger.Modifiers, ModifierKey::Win));

    int CurrentIndex = MouseCombo->findData(static_cast<int>(InNewTrigger.Button));
    if (CurrentIndex >= 0) 
    {
        MouseCombo->setCurrentIndex(CurrentIndex);
    }

    CurrentMods   = InNewTrigger.Modifiers;
    CurrentVkCode = InNewTrigger.VKCode;

    UpdateVisibilities();
    UpdateDisplay();
}

void HotkeyRecorder::UpdateDisplay()
{
    auto CurrentTrigger = GetTrigger();
    QString DisplayText = ModifierKeyToString(CurrentTrigger.Modifiers);
    
    if (CurrentTrigger.Mode == ActivationMode::MouseHold) 
    {
        if (!DisplayText.isEmpty()) 
        {
            DisplayText += '+';
        }
        DisplayText += MouseButtonToString(CurrentTrigger.Button);
    } 
    else 
    {
        if (CurrentTrigger.VKCode != 0) 
        {
            if (!DisplayText.isEmpty()) 
            {
                DisplayText += '+';
            }
            // Simple mapping for display
            if (!CurrentKeyString.isEmpty()) 
            {
                DisplayText += CurrentKeyString;
            } 
            else 
            {
#ifdef Q_OS_WIN
                UINT LocalScanCode = MapVirtualKey(CurrentTrigger.VKCode, MAPVK_VK_TO_VSC);
                WCHAR LocalName[128];
                int Result = GetKeyNameTextW(LocalScanCode << 16, LocalName, 128);
                if (Result > 0) 
                {
                    DisplayText += QString::fromWCharArray(LocalName).toUpper();
                } 
                else 
                {
                    DisplayText += QStringLiteral("VK_") + QString::number(CurrentTrigger.VKCode);
                }
#else
                DisplayText += QStringLiteral("Key ") + QString::number(CurrentTrigger.VKCode);
#endif
            }
        } 
        else 
        {
            DisplayText = QStringLiteral("Unset");
        }
    }

    if (DisplayText.isEmpty()) 
    {
        DisplayText = QStringLiteral("None");
    }
    DisplayLabel->setText(DisplayText);
}

void HotkeyRecorder::StartRecording()
{
    bIsRecording = true;
    RecordBtn->setText(QStringLiteral("Press any key combination..."));
    RecordBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #313150; color: #ffffff; border: 1px solid #4f46e5; border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
    ));
    setFocus();
}

void HotkeyRecorder::StopRecording(bool bInCanceled)
{
    if (!bIsRecording) 
    {
        return;
    }
    bIsRecording = false;
    RecordBtn->setText(QStringLiteral("Click to record shortcut..."));
    RecordBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1e1e35; color: #c0c0d8; border: 1px dashed #4f46e5; border-radius: 6px; padding: 6px 16px; font-weight: 600; }"
        "QPushButton:hover { background: #2a2a40; }"
    ));

    if (!bInCanceled) 
    {
        UpdateDisplay();
        emit TriggerChanged(GetTrigger());
    }
}

void HotkeyRecorder::keyPressEvent(QKeyEvent* Event)
{
    if (!bIsRecording) 
    {
        QWidget::keyPressEvent(Event);
        return;
    }

    int Key = Event->key();
    // Ignore lone modifier presses
    if (Key == Qt::Key_Control || Key == Qt::Key_Shift || Key == Qt::Key_Alt || Key == Qt::Key_Meta) 
    {
        return; 
    }

    if (Key == Qt::Key_Escape) 
    {
        StopRecording(true);
        return;
    }

    ModifierKey LocalMods = ModifierKey::None;
    if (Event->modifiers() & Qt::ControlModifier) LocalMods = LocalMods | ModifierKey::Ctrl;
    if (Event->modifiers() & Qt::ShiftModifier)   LocalMods = LocalMods | ModifierKey::Shift;
    if (Event->modifiers() & Qt::AltModifier)     LocalMods = LocalMods | ModifierKey::Alt;
    if (Event->modifiers() & Qt::MetaModifier)    LocalMods = LocalMods | ModifierKey::Win;

    CurrentMods   = LocalMods;
    CurrentVkCode = Event->nativeVirtualKey();
    
    int RawKey = Key & ~Qt::KeyboardModifierMask;
    CurrentKeyString = QKeySequence(RawKey).toString();

    StopRecording(false);
}

void HotkeyRecorder::focusOutEvent(QFocusEvent* Event)
{
    if (bIsRecording) 
    {
        StopRecording(true);
    }
    QWidget::focusOutEvent(Event);
}

} // namespace gpm
