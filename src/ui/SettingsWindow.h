#pragma once

// =============================================================================
// GoPieMenu - Settings Window
// =============================================================================

#include "core/ConfigManager.h"
#include "core/ActionExecutor.h"

#include <QMainWindow>
#include <QListWidget>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>

namespace gpm 
{

class PiePreviewWidget;
class HotkeyRecorder;
class ColorPickerButton;

class SettingsWindow : public QMainWindow 
{
    Q_OBJECT

public:
    explicit SettingsWindow(ConfigManager* InConfigMgr, ActionExecutor* InExecutor, QWidget* Parent = nullptr);

signals:
    void ConfigUpdated();

private:
    // === Page Builders ===
    QWidget* CreateProfilesPage();
    QWidget* CreateStylePage();
    QWidget* CreateGeneralPage();

    // === Save Bar (Discord-Style) ===
    QWidget* CreateSaveBar();
    void ShowSaveBar();
    void HideSaveBar();
    void MarkDirty();

    // === Profile Management ===
    void PopulateProfileList();
    void LoadProfile(int Index);
    void SaveCurrentProfile();
    void OnAddProfile();
    void OnRemoveProfile();
    void OnDuplicateProfile();

    // === Item Editing ===
    QWidget* CreateItemEditor();
    void PopulateItemList();
    void LoadItem(QTreeWidgetItem* Item, int Column);
    void SaveCurrentItem();
    void OnAddItem();
    void OnAddSubItem();
    void OnRemoveItem();
    void UpdateItemEditorState();
    
    PieItem* FindItemById(const QString& Id);
    void SyncTreeToItems();

    // === Icon System ===
    void ScanIconFolder();
    void PopulateIconCombo();
    QString GetIconFolderPath() const;

    // === Style Management ===
    void LoadStyleSettings();
    void SaveStyleSettings();

    // === General Settings ===
    void LoadGeneralSettings();
    void SaveGeneralSettings();

    // === Apply / Discard ===
    void ApplyChanges();
    void DiscardChanges();

    // === UI Helpers ===
    QPushButton* CreateNavButton(const QString& Text, bool bIsActive = false);
    QWidget* CreateCard(const QString& Title, QLayout* Content);

    // === Core ===
    ConfigManager*   ConfigMgr;
    ActionExecutor*  Executor;

    // === Navigation ===
    QStackedWidget*  PageStack          = nullptr;
    QPushButton*     NavProfileBtn      = nullptr;
    QPushButton*     NavStyleBtn        = nullptr;
    QPushButton*     NavGeneralBtn      = nullptr;

    // === Save Bar ===
    QWidget*         SaveBar            = nullptr;
    bool             bIsDirty           = false;

    // === Profile Page Widgets ===
    QListWidget*     ProfileList        = nullptr;
    QLineEdit*       ProfileNameEdit    = nullptr;
    QCheckBox*       ProfileEnabledCheck= nullptr;
    HotkeyRecorder*  ProfileHotkeyRecorder = nullptr;
    QLineEdit*       AppFilterEdit      = nullptr;
    QTreeWidget*     ItemTree           = nullptr;
    PiePreviewWidget* PiePreview        = nullptr;

    // === Item Editor Widgets ===
    QWidget*           ItemEditorPanel    = nullptr;
    QLineEdit*         ItemNameEdit       = nullptr;
    QPushButton*       ItemIconButton     = nullptr;
    QComboBox*         ItemActionTypeCombo= nullptr;
    QLineEdit*         ItemActionDataEdit = nullptr;
    QLineEdit*         ItemActionArgsEdit = nullptr;
    ColorPickerButton* ItemColorPicker    = nullptr;
    QString            CurrentIconPath;

    // === Style Page Widgets ===
    QDoubleSpinBox*  OuterRadiusSpin    = nullptr;
    QDoubleSpinBox*  InnerRadiusSpin    = nullptr;
    QDoubleSpinBox*  IconSizeSpin       = nullptr;
    QDoubleSpinBox*  GapAngleSpin       = nullptr;
    QSpinBox*        AnimDurationSpin   = nullptr;
    QDoubleSpinBox*  OpacitySpin        = nullptr;
    QDoubleSpinBox*  FontSizeSpin       = nullptr;
    QDoubleSpinBox*  TextOutlineThicknessSpin = nullptr;
    ColorPickerButton* BGColorPicker     = nullptr;
    ColorPickerButton* SectorColorPicker = nullptr;
    ColorPickerButton* HoverColorPicker  = nullptr;
    ColorPickerButton* TextColorPicker   = nullptr;
    ColorPickerButton* CenterColorPicker = nullptr;
    QCheckBox*         AutoContrastCheck  = nullptr;

    // === General Page Widgets ===
    QCheckBox*       AutoStartCheck     = nullptr;
    QLabel*          IconFolderLabel    = nullptr;

    // === State ===
    int              CurrentProfileIdx  = -1;
    QTreeWidgetItem* CurrentItemNode    = nullptr;
    bool             bIsUpdatingUI      = false;
    
    QLabel*          ActionDataLabel    = nullptr;
    QLabel*          ActionArgsLabel    = nullptr;
    QPushButton*     BrowseBtn          = nullptr;
    HotkeyRecorder*  ItemHotkeyRecorder = nullptr;
};

} // namespace gpm
