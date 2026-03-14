// =============================================================================
// GoPieMenu - Settings Window Implementation (Modern UI v2)
// =============================================================================

#include "SettingsWindow.h"
#include "widgets/HotkeyRecorder.h"
#include "widgets/ColorPickerButton.h"
#include "widgets/PiePreviewWidget.h"
#include "widgets/IconPickerDialog.h"
#include "widgets/WindowPickerDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QStyle>
#include <QScrollArea>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDesktopServices>
#include <QGraphicsOpacityEffect>

namespace gpm 
{

// =============================================================================
// Stylesheet
// =============================================================================

static const char* StyleSystemText = R"(
    * { font-family: "Segoe UI", sans-serif; }
    QMainWindow { background: #0f0f17; }

    QPushButton#navBtn {
        background: transparent; color: #6b6b8a; border: none;
        border-radius: 8px; padding: 10px 20px;
        text-align: left; font-size: 13px; font-weight: 500;
    }
    QPushButton#navBtn:hover { background: #1a1a2e; color: #a0a0c8; }
    QPushButton#navBtnActive {
        background: #1e1e35; color: #c8c8f0; border: none;
        border-radius: 8px; padding: 10px 20px;
        text-align: left; font-size: 13px; font-weight: 600;
        border-left: 3px solid #6366f1;
    }

    QGroupBox {
        background: #16161e; border: 1px solid #1e1e30; border-radius: 10px;
        margin-top: 8px; padding: 20px 16px 16px 16px;
        font-size: 12px; font-weight: 600; color: #8888aa;
    }
    QGroupBox::title {
        subcontrol-origin: margin; left: 16px; padding: 0 8px;
        color: #8888aa; font-size: 11px; font-weight: 600;
    }

    QLabel { color: #b0b0cc; font-size: 12px; }
    QLabel#sectionTitle { color: #e0e0f0; font-size: 18px; font-weight: 700; padding-bottom: 4px; }
    QLabel#sectionSubtitle { color: #6b6b8a; font-size: 12px; padding-bottom: 12px; }

    QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
        background: #1a1a28; color: #d0d0e8; border: 1px solid #2a2a40;
        border-radius: 6px; padding: 7px 10px; font-size: 12px;
        selection-background-color: #4f46e5;
    }
    QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus { border-color: #4f46e5; }
    QComboBox::drop-down { border: none; padding-right: 8px; }
    QComboBox QAbstractItemView {
        background: #1a1a28; color: #d0d0e8; border: 1px solid #2a2a40;
        selection-background-color: #4f46e5;
    }

    QListWidget {
        background: #131320; color: #c0c0d8; border: 1px solid #1e1e30;
        border-radius: 8px; padding: 4px; font-size: 12px; outline: none;
    }
    QListWidget::item { padding: 8px 12px; border-radius: 6px; margin: 1px 2px; }
    QListWidget::item:selected { background: #2a2a50; color: #e0e0ff; }
    QListWidget::item:hover:!selected { background: #1a1a30; }

    QPushButton {
        background: #1e1e35; color: #c0c0d8; border: 1px solid #2a2a45;
        border-radius: 6px; padding: 7px 16px; font-size: 12px; font-weight: 500;
    }
    QPushButton:hover { background: #2a2a50; border-color: #4f46e5; }
    QPushButton:pressed { background: #3a3a60; }

    QPushButton#primaryBtn {
        background: #4f46e5; color: #ffffff; border: none;
        font-weight: 600; padding: 8px 24px;
    }
    QPushButton#primaryBtn:hover { background: #5b52f0; }

    QPushButton#dangerBtn { background: transparent; color: #f87171; border: 1px solid #7f1d1d; }
    QPushButton#dangerBtn:hover { background: #1c0a0a; border-color: #dc2626; }

    QPushButton#ghostBtn {
        background: transparent; color: #a0a0c0; border: 1px solid #3a3a55;
        font-weight: 500;
    }
    QPushButton#ghostBtn:hover { background: #1a1a2e; color: #d0d0e8; border-color: #5a5a7a; }

    QPushButton#iconBtn {
        background: transparent; border: 1px solid #2a2a45;
        border-radius: 6px; padding: 5px; min-width: 32px; max-width: 32px;
    }
    QPushButton#iconBtn:hover { background: #1e1e35; border-color: #4f46e5; }

    QCheckBox { color: #b0b0cc; font-size: 12px; spacing: 8px; }
    QCheckBox::indicator { width: 18px; height: 18px; border: 2px solid #3a3a5c; border-radius: 4px; background: #1a1a28; }
    QCheckBox::indicator:checked { background: #4f46e5; border-color: #4f46e5; }

    QScrollArea { background: transparent; border: none; }
    QScrollBar:vertical { background: transparent; width: 6px; margin: 0; }
    QScrollBar::handle:vertical { background: #2a2a40; border-radius: 3px; min-height: 30px; }
    QScrollBar::handle:vertical:hover { background: #3a3a60; }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
)";

// =============================================================================
// Constructor
// =============================================================================

SettingsWindow::SettingsWindow(ConfigManager* InConfigMgr, ActionExecutor* InExecutor, QWidget* Parent)
    : QMainWindow(Parent)
    , ConfigMgr(InConfigMgr)
    , Executor(InExecutor)
{
    setWindowTitle(QStringLiteral("GoPieMenu"));
    setMinimumSize(1060, 720);

    auto* Central = new QWidget;
    auto* RootLayout = new QVBoxLayout(Central);
    RootLayout->setContentsMargins(0, 0, 0, 0);
    RootLayout->setSpacing(0);

    auto* MainRow = new QHBoxLayout;
    MainRow->setContentsMargins(0, 0, 0, 0);
    MainRow->setSpacing(0);

    // --- Sidebar ---
    auto* Sidebar = new QWidget;
    Sidebar->setFixedWidth(200);
    Sidebar->setStyleSheet(QStringLiteral("background: #101018; border-right: 1px solid #1a1a2e;"));
    auto* SideLayout = new QVBoxLayout(Sidebar);
    SideLayout->setContentsMargins(12, 20, 12, 12);
    SideLayout->setSpacing(4);

    auto* LogoLabel = new QLabel(QStringLiteral("GoPieMenu"));
    LogoLabel->setStyleSheet(QStringLiteral(
        "color: #e0e0f0; font-size: 16px; font-weight: 700; "
        "padding: 4px 8px 4px 8px; border: none; background: transparent;"));
    SideLayout->addWidget(LogoLabel);

    auto* VersionLabel = new QLabel(QStringLiteral("v" APP_VERSION));
    VersionLabel->setStyleSheet(QStringLiteral(
        "color: #4a4a6a; font-size: 10px; padding: 0 8px 20px 8px; "
        "border: none; background: transparent;"));
    SideLayout->addWidget(VersionLabel);

    NavProfileBtn = CreateNavButton(QStringLiteral("Profiles"), true);
    NavStyleBtn   = CreateNavButton(QStringLiteral("Appearance"));
    NavGeneralBtn = CreateNavButton(QStringLiteral("General"));

    SideLayout->addWidget(NavProfileBtn);
    SideLayout->addWidget(NavStyleBtn);
    SideLayout->addWidget(NavGeneralBtn);
    SideLayout->addStretch();

    // --- Save bar (Discord-style, initially hidden) ---
    SaveBar = CreateSaveBar();
    SaveBar->setMaximumHeight(0);
    SaveBar->setVisible(false);

    // Content
    PageStack = new QStackedWidget;
    PageStack->addWidget(CreateProfilesPage());
    PageStack->addWidget(CreateStylePage());
    PageStack->addWidget(CreateGeneralPage());

    MainRow->addWidget(Sidebar);
    MainRow->addWidget(PageStack, 1);
    RootLayout->addLayout(MainRow, 1);
    RootLayout->addWidget(SaveBar);

    setCentralWidget(Central);

    // Navigation
    auto SwitchPage = [this](int InIndex) 
    {
        PageStack->setCurrentIndex(InIndex);
        for (auto* Btn : {NavProfileBtn, NavStyleBtn, NavGeneralBtn}) 
        {
            Btn->setObjectName(QStringLiteral("navBtn"));
        }
        QList<QPushButton*> Btns = {NavProfileBtn, NavStyleBtn, NavGeneralBtn};
        Btns[InIndex]->setObjectName(QStringLiteral("navBtnActive"));
        for (auto* Btn : Btns) 
        { 
            Btn->style()->unpolish(Btn); 
            Btn->style()->polish(Btn); 
        }
    };
    connect(NavProfileBtn, &QPushButton::clicked, this, [=]() { SwitchPage(0); });
    connect(NavStyleBtn,   &QPushButton::clicked, this, [=]() { SwitchPage(1); });
    connect(NavGeneralBtn, &QPushButton::clicked, this, [=]() { SwitchPage(2); });

    setStyleSheet(QString::fromUtf8(StyleSystemText));

    ScanIconFolder();
    
    bIsUpdatingUI = true;
    PopulateProfileList();
    if (!ConfigMgr->GetConfig().Profiles.empty()) 
    {
        ProfileList->setCurrentRow(0);
        LoadProfile(0);
    }
    bIsUpdatingUI = false;
    bIsDirty      = false;
    SaveBar->setVisible(false);
    SaveBar->setMaximumHeight(0);
}

QPushButton* SettingsWindow::CreateNavButton(const QString& InText, bool bInActive)
{
    auto* Btn = new QPushButton(InText);
    Btn->setObjectName(bInActive ? QStringLiteral("navBtnActive") : QStringLiteral("navBtn"));
    Btn->setCursor(Qt::PointingHandCursor);
    return Btn;
}

QWidget* SettingsWindow::CreateCard(const QString& InTitle, QLayout* InContent)
{
    auto* Group = new QGroupBox(InTitle);
    Group->setLayout(InContent);
    return Group;
}

// =============================================================================
// DISCORD-STYLE SAVE BAR
// =============================================================================

QWidget* SettingsWindow::CreateSaveBar()
{
    auto* Bar = new QWidget;
    Bar->setStyleSheet(QStringLiteral(
        "background: #1e1e35; border-top: 1px solid #2a2a50;"));
    Bar->setFixedHeight(56);

    auto* Layout = new QHBoxLayout(Bar);
    Layout->setContentsMargins(24, 8, 24, 8);
    Layout->setSpacing(12);

    auto* InfoLabel = new QLabel(QStringLiteral("You have unsaved changes"));
    InfoLabel->setStyleSheet(QStringLiteral(
        "color: #e0e0f0; font-size: 13px; font-weight: 500; background: transparent; border: none;"));
    Layout->addWidget(InfoLabel);
    Layout->addStretch();

    auto* DiscardBtn = new QPushButton(QStringLiteral("Discard"));
    DiscardBtn->setObjectName(QStringLiteral("ghostBtn"));
    DiscardBtn->setCursor(Qt::PointingHandCursor);
    connect(DiscardBtn, &QPushButton::clicked, this, &SettingsWindow::DiscardChanges);
    Layout->addWidget(DiscardBtn);

    auto* SaveBtnLocal = new QPushButton(QStringLiteral("Save Changes"));
    SaveBtnLocal->setObjectName(QStringLiteral("primaryBtn"));
    SaveBtnLocal->setCursor(Qt::PointingHandCursor);
    connect(SaveBtnLocal, &QPushButton::clicked, this, &SettingsWindow::ApplyChanges);
    Layout->addWidget(SaveBtnLocal);

    return Bar;
}

void SettingsWindow::ShowSaveBar()
{
    if (SaveBar->isVisible() && SaveBar->maximumHeight() > 0) 
    {
        return;
    }
    SaveBar->setVisible(true);
    auto* Anim = new QPropertyAnimation(SaveBar, "maximumHeight", this);
    Anim->setDuration(200);
    Anim->setStartValue(0);
    Anim->setEndValue(56);
    Anim->setEasingCurve(QEasingCurve::OutCubic);
    Anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void SettingsWindow::HideSaveBar()
{
    auto* Anim = new QPropertyAnimation(SaveBar, "maximumHeight", this);
    Anim->setDuration(150);
    Anim->setStartValue(56);
    Anim->setEndValue(0);
    Anim->setEasingCurve(QEasingCurve::InCubic);
    connect(Anim, &QPropertyAnimation::finished, this, [this]() 
    {
        SaveBar->setVisible(false);
    });
    Anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void SettingsWindow::MarkDirty()
{
    if (bIsUpdatingUI) 
    {
        return;
    }
    if (!bIsDirty) 
    {
        bIsDirty = true;
        ShowSaveBar();
    }
}

// =============================================================================
// ICON SYSTEM
// =============================================================================

QString SettingsWindow::GetIconFolderPath() const
{
    return qApp->applicationDirPath() + QStringLiteral("/icons");
}

void SettingsWindow::ScanIconFolder()
{
    QDir Directory(GetIconFolderPath());
    if (!Directory.exists()) 
    {
        Directory.mkpath(".");
    }
}

// =============================================================================
// PROFILES PAGE
// =============================================================================

QWidget* SettingsWindow::CreateProfilesPage()
{
    auto* Page = new QWidget;
    auto* PageLayout = new QHBoxLayout(Page);
    PageLayout->setContentsMargins(24, 24, 24, 24);
    PageLayout->setSpacing(16);

    // --- Left column: profile list (wider) ---
    auto* LeftCol = new QWidget;
    LeftCol->setFixedWidth(260);
    auto* LeftLayout = new QVBoxLayout(LeftCol);
    LeftLayout->setContentsMargins(0, 0, 0, 0);
    LeftLayout->setSpacing(8);

    auto* ListTitle = new QLabel(QStringLiteral("Profiles"));
    ListTitle->setObjectName(QStringLiteral("sectionTitle"));
    LeftLayout->addWidget(ListTitle);

    ProfileList = new QListWidget;
    connect(ProfileList, &QListWidget::currentRowChanged, this, &SettingsWindow::LoadProfile);
    LeftLayout->addWidget(ProfileList, 1);

    // Buttons in two rows for readability
    auto* AddBtnLocal = new QPushButton(QStringLiteral("Add Profile"));
    connect(AddBtnLocal, &QPushButton::clicked, this, &SettingsWindow::OnAddProfile);
    LeftLayout->addWidget(AddBtnLocal);

    auto* BtnRow = new QHBoxLayout;
    BtnRow->setSpacing(6);
    auto* DupBtn = new QPushButton(QStringLiteral("Duplicate"));
    auto* RemoveBtnLocal = new QPushButton(QStringLiteral("Remove"));
    RemoveBtnLocal->setObjectName(QStringLiteral("dangerBtn"));
    connect(DupBtn,       &QPushButton::clicked, this, &SettingsWindow::OnDuplicateProfile);
    connect(RemoveBtnLocal, &QPushButton::clicked, this, &SettingsWindow::OnRemoveProfile);
    BtnRow->addWidget(DupBtn);
    BtnRow->addWidget(RemoveBtnLocal);
    LeftLayout->addLayout(BtnRow);

    // --- Right column: profile editor ---
    auto* RightScroll = new QScrollArea;
    RightScroll->setWidgetResizable(true);
    auto* RightPanel = new QWidget;
    auto* RightLayout = new QVBoxLayout(RightPanel);
    RightLayout->setContentsMargins(0, 0, 8, 0);
    RightLayout->setSpacing(12);

    // Profile info card
    auto* InfoForm = new QFormLayout;
    InfoForm->setSpacing(10);
    InfoForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ProfileNameEdit = new QLineEdit;
    ProfileNameEdit->setPlaceholderText(QStringLiteral("Profile name"));
    ProfileEnabledCheck = new QCheckBox(QStringLiteral("Enable this profile"));
    ProfileEnabledCheck->setChecked(true);
    connect(ProfileNameEdit,    &QLineEdit::textEdited, this, [this]() { MarkDirty(); });
    connect(ProfileEnabledCheck, &QCheckBox::toggled,    this, [this]() { MarkDirty(); });
    InfoForm->addRow(QStringLiteral("Name"), ProfileNameEdit);
    InfoForm->addRow(QString(), ProfileEnabledCheck);
    RightLayout->addWidget(CreateCard(QStringLiteral("PROFILE"), InfoForm));

    // Trigger card
    auto* TriggerForm = new QFormLayout;
    TriggerForm->setSpacing(10);
    TriggerForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ProfileHotkeyRecorder = new HotkeyRecorder;
    connect(ProfileHotkeyRecorder, &HotkeyRecorder::TriggerChanged, this, [this]() { MarkDirty(); });
    
    AppFilterEdit = new QLineEdit;
    AppFilterEdit->setPlaceholderText(QStringLiteral("chrome.exe, code.exe"));
    connect(AppFilterEdit, &QLineEdit::textEdited, this, [this]() { MarkDirty(); });
    
    auto* AppFilterLayout = new QHBoxLayout;
    AppFilterLayout->setSpacing(6);
    AppFilterLayout->addWidget(AppFilterEdit, 1);
    
    auto* PickWindowBtn = new QPushButton(QStringLiteral("Target"));
    PickWindowBtn->setObjectName(QStringLiteral("iconBtn"));
    PickWindowBtn->setFixedWidth(60);
    PickWindowBtn->setToolTip(QStringLiteral("Pick from running applications"));
    connect(PickWindowBtn, &QPushButton::clicked, this, [this]() 
    {
        WindowPickerDialog Dialog(this);
        if (Dialog.exec() == QDialog::Accepted) 
        {
            QString ProcName = Dialog.GetSelectedProcessName();
            if (!ProcName.isEmpty()) 
            {
                QString Current = AppFilterEdit->text().trimmed();
                if (Current.isEmpty()) 
                {
                    AppFilterEdit->setText(ProcName);
                }
                else if (!Current.contains(ProcName)) 
                {
                    AppFilterEdit->setText(Current + ", " + ProcName);
                }
                MarkDirty();
            }
        }
    });
    AppFilterLayout->addWidget(PickWindowBtn);
    
    auto* BrowseExeBtn = new QPushButton(QStringLiteral("..."));
    BrowseExeBtn->setObjectName(QStringLiteral("iconBtn"));
    BrowseExeBtn->setToolTip(QStringLiteral("Browse for .exe"));
    connect(BrowseExeBtn, &QPushButton::clicked, this, [this]() 
    {
        QString FullPath = QFileDialog::getOpenFileName(this, QStringLiteral("Select Application"), {}, QStringLiteral("Executables (*.exe)"));
        if (!FullPath.isEmpty()) 
        {
            QFileInfo Info(FullPath);
            QString ProcName = Info.fileName();
            QString Current = AppFilterEdit->text().trimmed();
            if (Current.isEmpty()) 
            {
                AppFilterEdit->setText(ProcName);
            }
            else if (!Current.contains(ProcName)) 
            {
                AppFilterEdit->setText(Current + ", " + ProcName);
            }
            MarkDirty();
        }
    });
    AppFilterLayout->addWidget(BrowseExeBtn);

    TriggerForm->addRow(QStringLiteral("Trigger"), ProfileHotkeyRecorder);
    TriggerForm->addRow(QStringLiteral("App Filter"), AppFilterLayout);
    RightLayout->addWidget(CreateCard(QStringLiteral("ACTIVATION"), TriggerForm));

    // Items card
    auto* ItemsCardGroup = new QGroupBox(QStringLiteral("ITEMS"));
    auto* ContainerLayout = new QVBoxLayout(ItemsCardGroup);
    ContainerLayout->setSpacing(8);

    auto* ItemsTopLayout = new QHBoxLayout;
    ItemTree = new QTreeWidget;
    ItemTree->setHeaderHidden(true);
    ItemTree->setMinimumHeight(220);
    ItemTree->setDragEnabled(true);
    ItemTree->setAcceptDrops(true);
    ItemTree->setDropIndicatorShown(true);
    ItemTree->setDragDropMode(QAbstractItemView::InternalMove);
    
    ItemTree->setStyleSheet(QStringLiteral(
        "QTreeWidget {"
        "  background: #131320; border: 1px solid #2a2a40; border-radius: 6px;"
        "  padding: 4px; outline: none;"
        "}"
        "QTreeWidget::item {"
        "  color: #d0d0e8; font-size: 13px; padding: 4px; border-radius: 4px;"
        "}"
        "QTreeWidget::item:hover {"
        "  background: #2a2a40;"
        "}"
        "QTreeWidget::item:selected {"
        "  background: #4f46e5; color: #ffffff;"
        "}"
        "QTreeView::branch:has-children:!has-siblings:closed,"
        "QTreeView::branch:closed:has-children:has-siblings {"
        "  image: url(none); border-image: none;"
        "  subcontrol-position: center left;"
        "}"
        "QTreeView::branch:has-children {"
        "  margin: 0px;"
        "}"
        "QTreeView::branch:open:has-children {"
        "  image: url(none); border-image: none;"
        "}"
    ));
    connect(ItemTree, &QTreeWidget::itemClicked, this, &SettingsWindow::LoadItem);

    auto* ItemBtnCol = new QVBoxLayout;
    ItemBtnCol->setSpacing(4);
    auto* AddItemBtnLocal = new QPushButton(QStringLiteral("+"));
    AddItemBtnLocal->setObjectName(QStringLiteral("iconBtn"));
    AddItemBtnLocal->setToolTip(QStringLiteral("Add a new action item"));
    auto* AddSubItemBtnLocal = new QPushButton(QStringLiteral("\xF0\x9F\x93\x81")); // folder emoji
    AddSubItemBtnLocal->setObjectName(QStringLiteral("iconBtn"));
    AddSubItemBtnLocal->setToolTip(QStringLiteral("Add a folder (List Menu) to contain sub-actions"));
    auto* RemoveItemBtnLocal = new QPushButton(QStringLiteral("-"));
    RemoveItemBtnLocal->setObjectName(QStringLiteral("iconBtn"));
    connect(AddItemBtnLocal,    &QPushButton::clicked, this, &SettingsWindow::OnAddItem);
    connect(AddSubItemBtnLocal, &QPushButton::clicked, this, &SettingsWindow::OnAddSubItem);
    connect(RemoveItemBtnLocal, &QPushButton::clicked, this, &SettingsWindow::OnRemoveItem);
    ItemBtnCol->addWidget(AddItemBtnLocal);
    ItemBtnCol->addWidget(AddSubItemBtnLocal);
    ItemBtnCol->addWidget(RemoveItemBtnLocal);
    ItemBtnCol->addStretch();

    ItemsTopLayout->addWidget(ItemTree, 1);
    ItemsTopLayout->addLayout(ItemBtnCol);
    ContainerLayout->addLayout(ItemsTopLayout);
    ContainerLayout->addWidget(CreateItemEditor());
    
    // Connect item reordered via drag drop
    auto SyncTreeFn = [this]() 
    {
        if (bIsUpdatingUI) 
        {
            return;
        }
        QTimer::singleShot(0, this, [this]() 
        {
            if (bIsUpdatingUI) 
            {
                return;
            }
            SyncTreeToItems();
            MarkDirty();
            if (CurrentProfileIdx >= 0) 
            {
                PiePreview->SetConfig(ConfigMgr->GetConfig().Profiles[CurrentProfileIdx], ConfigMgr->GetConfig().GlobalStyle);
            }
        });
    };
    connect(ItemTree->model(), &QAbstractItemModel::rowsMoved,    this, SyncTreeFn);
    connect(ItemTree->model(), &QAbstractItemModel::rowsInserted, this, SyncTreeFn);
    connect(ItemTree->model(), &QAbstractItemModel::rowsRemoved,  this, SyncTreeFn);
    
    RightLayout->addWidget(ItemsCardGroup);

    // Preview
    auto* PreviewGroup = new QGroupBox(QStringLiteral("PREVIEW"));
    auto* PreviewLayout = new QVBoxLayout(PreviewGroup);
    PiePreview = new PiePreviewWidget;
    PreviewLayout->addWidget(PiePreview);
    RightLayout->addWidget(PreviewGroup);

    RightLayout->addStretch();
    RightScroll->setWidget(RightPanel);

    PageLayout->addWidget(LeftCol);
    PageLayout->addWidget(RightScroll, 1);
    return Page;
}


QWidget* SettingsWindow::CreateItemEditor()
{
    auto* Panel = new QGroupBox(QStringLiteral("ITEM DETAILS"));
    ItemEditorPanel = Panel;
    auto* Layout = new QFormLayout(Panel);
    Layout->setSpacing(8);
    Layout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    ItemNameEdit = new QLineEdit;
    ItemNameEdit->setPlaceholderText(QStringLiteral("Display name"));
    connect(ItemNameEdit, &QLineEdit::textEdited, this, [this]() { MarkDirty(); });

    // Icon button invoking dialog
    ItemIconButton = new QPushButton(QStringLiteral("Select Icon..."));
    ItemIconButton->setObjectName(QStringLiteral("ghostBtn"));
    ItemIconButton->setCursor(Qt::PointingHandCursor);
    ItemIconButton->setStyleSheet(ItemIconButton->styleSheet() + QStringLiteral("text-align: left; padding: 6px 10px;"));
    
    auto* IconRow = new QHBoxLayout;
    IconRow->setSpacing(6);
    IconRow->addWidget(ItemIconButton, 1);

    connect(ItemIconButton, &QPushButton::clicked, this, [this]() 
    {
        IconPickerDialog Dialog(GetIconFolderPath(), this);
        if (Dialog.exec() == QDialog::Accepted) 
        {
            CurrentIconPath = Dialog.GetSelectedIconPath();
            if (CurrentIconPath.isEmpty()) 
            {
                ItemIconButton->setText(QStringLiteral("Select Icon..."));
                ItemIconButton->setIcon(QIcon());
            } 
            else 
            {
                ItemIconButton->setText(CurrentIconPath);
                ItemIconButton->setIcon(QIcon(GetIconFolderPath() + "/" + CurrentIconPath));
            }
            MarkDirty();
        }
    });

    ItemActionTypeCombo = new QComboBox;
    connect(ItemActionTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() 
            { 
                UpdateItemEditorState();
                MarkDirty(); 
            });

    ItemActionDataEdit = new QLineEdit;
    ItemActionDataEdit->setPlaceholderText(QStringLiteral("Target path, URL, or hotkey combo"));
    connect(ItemActionDataEdit, &QLineEdit::textEdited, this, [this]() { MarkDirty(); });
    
    ItemHotkeyRecorder = new HotkeyRecorder;
    ItemHotkeyRecorder->setMinimumHeight(40);
    ItemHotkeyRecorder->SetKeyboardOnlyMode(true);
    ItemHotkeyRecorder->hide();
    connect(ItemHotkeyRecorder, &HotkeyRecorder::TriggerChanged, this, [this]() { MarkDirty(); });

    ItemActionArgsEdit = new QLineEdit;
    ItemActionArgsEdit->setPlaceholderText(QStringLiteral("Additional arguments (optional)"));
    connect(ItemActionArgsEdit, &QLineEdit::textEdited, this, [this]() { MarkDirty(); });

    ItemColorPicker = new ColorPickerButton;
    connect(ItemColorPicker, &ColorPickerButton::ColorChanged, this, [this]() { MarkDirty(); });

    // Populate action types
    if (Executor) 
    {
        for (auto TypeEnum : Executor->GetRegisteredTypes()) 
        {
            ItemActionTypeCombo->addItem(Executor->GetDisplayNameFor(TypeEnum), static_cast<int>(TypeEnum));
        }
    }

    if (ItemActionTypeCombo->count() == 0) 
    {
        ItemActionTypeCombo->addItem(QStringLiteral("Launch Application"), static_cast<int>(ActionType::LaunchApp));
        ItemActionTypeCombo->addItem(QStringLiteral("Run Command"),        static_cast<int>(ActionType::RunCommand));
        ItemActionTypeCombo->addItem(QStringLiteral("Send Hotkey"),        static_cast<int>(ActionType::SendHotkey));
        ItemActionTypeCombo->addItem(QStringLiteral("Open File / Folder"), static_cast<int>(ActionType::OpenFile));
        ItemActionTypeCombo->addItem(QStringLiteral("Open URL"),           static_cast<int>(ActionType::OpenURL));
    }
    
    // Always ensure List Menu is present regardless of executor's dynamic types
    if (ItemActionTypeCombo->findData(static_cast<int>(ActionType::ListMenu)) == -1) 
    {
        ItemActionTypeCombo->addItem(QStringLiteral("List Menu"), static_cast<int>(ActionType::ListMenu));
    }

    auto* DataLayout = new QHBoxLayout;
    DataLayout->setSpacing(6);

    // Stack the hotkey recorder and data line edit
    auto* StackedDataLayout = new QVBoxLayout;
    StackedDataLayout->setContentsMargins(0, 0, 0, 0);
    StackedDataLayout->addWidget(ItemActionDataEdit);
    StackedDataLayout->addWidget(ItemHotkeyRecorder);
    
    DataLayout->addLayout(StackedDataLayout, 1);
    
    BrowseBtn = new QPushButton(QStringLiteral("..."));
    BrowseBtn->setObjectName(QStringLiteral("iconBtn"));
    connect(BrowseBtn, &QPushButton::clicked, this, [this]() 
    {
        auto TypeEnum = static_cast<ActionType>(ItemActionTypeCombo->currentData().toInt());
        QString PickedPath;
        if (TypeEnum == ActionType::LaunchApp) 
        {
            PickedPath = QFileDialog::getOpenFileName(this, QStringLiteral("Select Application"), {}, QStringLiteral("Executables (*.exe)"));
        }
        else if (TypeEnum == ActionType::OpenFile) 
        {
            PickedPath = QFileDialog::getOpenFileName(this, QStringLiteral("Select File"));
        }

        if (!PickedPath.isEmpty()) 
        { 
            ItemActionDataEdit->setText(PickedPath); 
            MarkDirty(); 
        }
    });
    DataLayout->addWidget(BrowseBtn);

    ActionDataLabel = new QLabel(QStringLiteral("Target"));
    ActionArgsLabel = new QLabel(QStringLiteral("Arguments"));

    Layout->addRow(QStringLiteral("Name"),      ItemNameEdit);
    Layout->addRow(QStringLiteral("Icon"),       IconRow);
    Layout->addRow(QStringLiteral("Action"),     ItemActionTypeCombo);
    Layout->addRow(ActionDataLabel,             DataLayout);
    Layout->addRow(ActionArgsLabel,             ItemActionArgsEdit);
    Layout->addRow(QStringLiteral("Color"),      ItemColorPicker);

    Panel->setEnabled(false);
    return Panel;
}

void SettingsWindow::UpdateItemEditorState()
{
    if (!ActionDataLabel || !ActionArgsLabel || !ItemActionTypeCombo || !ItemActionDataEdit || !ItemActionArgsEdit) 
    {
        return;
    }
    
    auto TypeEnum = static_cast<ActionType>(ItemActionTypeCombo->currentData().toInt());
    
    switch (TypeEnum) 
    {
        case ActionType::LaunchApp:
            ActionDataLabel->setText(QStringLiteral("Application"));
            ItemActionDataEdit->setPlaceholderText(QStringLiteral("Target executable path (.exe, .bat)"));
            ActionArgsLabel->setText(QStringLiteral("Arguments"));
            ItemActionArgsEdit->setPlaceholderText(QStringLiteral("Command line arguments (optional)"));
            
            ActionDataLabel->show();
            ItemActionDataEdit->show();
            ItemHotkeyRecorder->hide();
            BrowseBtn->setVisible(true);
            ActionArgsLabel->show();
            ItemActionArgsEdit->show();
            break;
            
        case ActionType::OpenFile:
        case ActionType::OpenURL:
        case ActionType::RunCommand:
            ActionDataLabel->setText((TypeEnum == ActionType::OpenURL) ? QStringLiteral("URL") : 
                                      (TypeEnum == ActionType::RunCommand) ? QStringLiteral("Command") : QStringLiteral("Path"));
            ItemActionDataEdit->setPlaceholderText(QStringLiteral("Target path or URL"));
            ActionArgsLabel->setText(QStringLiteral("Arguments"));
            ItemActionArgsEdit->setPlaceholderText(QStringLiteral("Additional parameters (optional)"));
            
            ActionDataLabel->show();
            ItemActionDataEdit->show();
            ItemHotkeyRecorder->hide();
            BrowseBtn->setVisible(TypeEnum == ActionType::OpenFile);
            ActionArgsLabel->show();
            ItemActionArgsEdit->show();
            break;
            
        case ActionType::SendHotkey:
            ActionDataLabel->setText(QStringLiteral("Hotkey"));
            
            ActionDataLabel->show();
            ItemActionDataEdit->hide();
            ItemHotkeyRecorder->show();
            BrowseBtn->setVisible(false);
            ActionArgsLabel->hide();
            ItemActionArgsEdit->hide();
            break;
            
        case ActionType::ListMenu:
            ActionDataLabel->hide();
            ItemActionDataEdit->hide();
            ItemHotkeyRecorder->hide();
            BrowseBtn->setVisible(false);
            ActionArgsLabel->hide();
            ItemActionArgsEdit->hide();
            break;
            
        default:
            ActionDataLabel->show();
            ItemActionDataEdit->show();
            ItemHotkeyRecorder->hide();
            BrowseBtn->setVisible(true);
            ActionArgsLabel->show();
            ItemActionArgsEdit->show();
            break;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// APPEARANCE PAGE
// ═══════════════════════════════════════════════════════════════════════════

QWidget* SettingsWindow::CreateStylePage()
{
    auto* ScrollBarArea = new QScrollArea;
    ScrollBarArea->setWidgetResizable(true);
    auto* Page = new QWidget;
    auto* MainLayout = new QVBoxLayout(Page);
    MainLayout->setContentsMargins(32, 24, 32, 24);
    MainLayout->setSpacing(16);

    auto* TitleLabel = new QLabel(QStringLiteral("Appearance"));
    TitleLabel->setObjectName(QStringLiteral("sectionTitle"));
    MainLayout->addWidget(TitleLabel);
    auto* SubtitleLabel = new QLabel(QStringLiteral("Customize the look and feel of your pie menus"));
    SubtitleLabel->setObjectName(QStringLiteral("sectionSubtitle"));
    MainLayout->addWidget(SubtitleLabel);

    // ── Preset Themes ──────────────────────────────────────────────────────
    auto ApplyPreset = [this](
        QColor InBG, QColor InSector, QColor InHover, QColor InBorder,
        QColor InText, QColor InCenter, QColor InCenterDot, bool bInContrast
    ) {
        bIsUpdatingUI = true;
        BGColorPicker->SetColor(InBG);
        SectorColorPicker->SetColor(InSector);
        HoverColorPicker->SetColor(InHover);
        TextColorPicker->SetColor(InText);
        CenterColorPicker->SetColor(InCenter);
        AutoContrastCheck->setChecked(bInContrast);
        bIsUpdatingUI = false;
        MarkDirty();
    };

    auto MakePresetBtn = [](const QString& InLabel, const QColor& InPreviewColor) 
    {
        auto* Btn = new QPushButton(InLabel);
        Btn->setCursor(Qt::PointingHandCursor);
        Btn->setFixedHeight(36);
        int Brightness = (InPreviewColor.red() * 299 + InPreviewColor.green() * 587 + InPreviewColor.blue() * 114) / 1000;
        QString FGColor = (Brightness > 125) ? "#111" : "#eee";
        Btn->setStyleSheet(QStringLiteral(
            "QPushButton { background: %1; color: %2; border: 2px solid %3;"
            "  border-radius: 6px; font-weight: 600; font-size: 12px; padding: 4px 14px; }"
            "QPushButton:hover { border-color: #89b4fa; }"
        ).arg(InPreviewColor.name(), FGColor, InPreviewColor.darker(130).name()));
        return Btn;
    };

    auto* PresetLayout = new QHBoxLayout;
    PresetLayout->setSpacing(8);

    // Obsidian (deep black)
    auto* ObsidianBtn = MakePresetBtn(QStringLiteral("Obsidian"), QColor(20, 20, 25));
    connect(ObsidianBtn, &QPushButton::clicked, this, [=]() 
    {
        ApplyPreset(
            QColor(20, 20, 25, 230),    // bg
            QColor(40, 40, 50, 210),    // sector
            QColor(90, 70, 220, 230),   // hover
            QColor(80, 80, 100, 150),   // border
            QColor(240, 240, 245),      // text
            QColor(30, 30, 38, 245),    // center
            QColor(120, 100, 240),      // centerDot
            false
        );
    });

    // Slate (neutral gray)
    auto* SlateBtn = MakePresetBtn(QStringLiteral("Slate"), QColor(70, 70, 80));
    connect(SlateBtn, &QPushButton::clicked, this, [=]() 
    {
        ApplyPreset(
            QColor(55, 55, 65, 220),    // bg
            QColor(80, 80, 95, 200),    // sector
            QColor(120, 130, 160, 220), // hover
            QColor(110, 110, 130, 150), // border
            QColor(235, 235, 240),      // text
            QColor(65, 65, 75, 240),    // center
            QColor(140, 160, 200),      // centerDot
            false
        );
    });

    // Ocean (gray-blue)
    auto* OceanBtn = MakePresetBtn(QStringLiteral("Ocean"), QColor(35, 55, 85));
    connect(OceanBtn, &QPushButton::clicked, this, [=]() 
    {
        ApplyPreset(
            QColor(25, 40, 65, 225),    // bg
            QColor(40, 60, 90, 210),    // sector
            QColor(60, 120, 200, 225),  // hover
            QColor(70, 100, 150, 150),  // border
            QColor(210, 230, 255),      // text
            QColor(30, 48, 72, 245),    // center
            QColor(80, 160, 240),       // centerDot
            false
        );
    });

    // Frost (light/white)
    auto* FrostBtn = MakePresetBtn(QStringLiteral("Frost"), QColor(230, 235, 245));
    connect(FrostBtn, &QPushButton::clicked, this, [=]() 
    {
        ApplyPreset(
            QColor(240, 242, 248, 230), // bg
            QColor(220, 225, 235, 210), // sector
            QColor(100, 140, 220, 220), // hover
            QColor(180, 185, 200, 150), // border
            QColor(30, 30, 40),         // text
            QColor(245, 247, 252, 245), // center
            QColor(80, 120, 200),       // centerDot
            true                        // auto-contrast on for light theme
        );
    });

    PresetLayout->addWidget(ObsidianBtn);
    PresetLayout->addWidget(SlateBtn);
    PresetLayout->addWidget(OceanBtn);
    PresetLayout->addWidget(FrostBtn);

    auto* PresetCard = CreateCard(QStringLiteral("PRESET THEMES"), PresetLayout);
    MainLayout->addWidget(PresetCard);

    // Geometry card
    auto* GeoForm = new QFormLayout;
    GeoForm->setSpacing(10);
    GeoForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    OuterRadiusSpin = new QDoubleSpinBox; OuterRadiusSpin->setRange(80, 400); OuterRadiusSpin->setSingleStep(5);
    InnerRadiusSpin = new QDoubleSpinBox; InnerRadiusSpin->setRange(20, 100); InnerRadiusSpin->setSingleStep(2);
    IconSizeSpin    = new QDoubleSpinBox; IconSizeSpin->setRange(12, 64);     IconSizeSpin->setSingleStep(2);
    GapAngleSpin    = new QDoubleSpinBox; GapAngleSpin->setRange(0, 20);      GapAngleSpin->setSingleStep(0.5);
    for (auto* Spinner : {OuterRadiusSpin, InnerRadiusSpin, IconSizeSpin, GapAngleSpin})
    {
        connect(Spinner, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { MarkDirty(); });
    }
    GeoForm->addRow(QStringLiteral("Outer Radius"), OuterRadiusSpin);
    GeoForm->addRow(QStringLiteral("Inner Radius"), InnerRadiusSpin);
    GeoForm->addRow(QStringLiteral("Icon Size"),     IconSizeSpin);
    GeoForm->addRow(QStringLiteral("Gap Angle"),     GapAngleSpin);
    MainLayout->addWidget(CreateCard(QStringLiteral("GEOMETRY"), GeoForm));

    // Colors card
    auto* ColorForm = new QFormLayout;
    ColorForm->setSpacing(10);
    ColorForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    BGColorPicker     = new ColorPickerButton;
    SectorColorPicker = new ColorPickerButton;
    HoverColorPicker  = new ColorPickerButton;
    TextColorPicker   = new ColorPickerButton;
    CenterColorPicker = new ColorPickerButton;
    for (auto* Picker : {BGColorPicker, SectorColorPicker, HoverColorPicker, TextColorPicker, CenterColorPicker})
    {
        connect(Picker, &ColorPickerButton::ColorChanged, this, [this]() { MarkDirty(); });
    }
    ColorForm->addRow(QStringLiteral("Background"), BGColorPicker);
    ColorForm->addRow(QStringLiteral("Sector"),      SectorColorPicker);
    ColorForm->addRow(QStringLiteral("Hover"),       HoverColorPicker);
    ColorForm->addRow(QStringLiteral("Text"),        TextColorPicker);
    ColorForm->addRow(QStringLiteral("Center"),      CenterColorPicker);
    MainLayout->addWidget(CreateCard(QStringLiteral("COLORS"), ColorForm));

    // Animation card
    auto* AnimForm = new QFormLayout;
    AnimForm->setSpacing(10);
    AnimForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    AnimDurationSpin = new QSpinBox; AnimDurationSpin->setRange(50, 1000); AnimDurationSpin->setSuffix(QStringLiteral(" ms"));
    OpacitySpin      = new QDoubleSpinBox; OpacitySpin->setRange(0.1, 1.0); OpacitySpin->setSingleStep(0.05);
    FontSizeSpin     = new QDoubleSpinBox; FontSizeSpin->setRange(8, 24);     FontSizeSpin->setSingleStep(0.5);
    TextOutlineThicknessSpin = new QDoubleSpinBox; TextOutlineThicknessSpin->setRange(0.0, 10.0); TextOutlineThicknessSpin->setSingleStep(0.5);
    connect(AnimDurationSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this]() { MarkDirty(); });
    connect(OpacitySpin,      QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { MarkDirty(); });
    connect(FontSizeSpin,     QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { MarkDirty(); });
    connect(TextOutlineThicknessSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this]() { MarkDirty(); });
    AnimForm->addRow(QStringLiteral("Animation"), AnimDurationSpin);
    AnimForm->addRow(QStringLiteral("Opacity"),   OpacitySpin);
    AnimForm->addRow(QStringLiteral("Font Size"), FontSizeSpin);
    AnimForm->addRow(QStringLiteral("Text Outline"), TextOutlineThicknessSpin);
    
    AutoContrastCheck = new QCheckBox(QStringLiteral("Auto-contrast text against sector background"));
    connect(AutoContrastCheck, &QCheckBox::toggled, this, [this]() { MarkDirty(); });
    AnimForm->addRow(QString(), AutoContrastCheck);
    
    MainLayout->addWidget(CreateCard(QStringLiteral("ANIMATION & TEXT"), AnimForm));

    MainLayout->addStretch();
    bIsUpdatingUI = true;
    LoadStyleSettings();
    bIsUpdatingUI = false;
    ScrollBarArea->setWidget(Page);
    return ScrollBarArea;
}

// =============================================================================
// GENERAL PAGE
// =============================================================================

QWidget* SettingsWindow::CreateGeneralPage()
{
    auto* Page = new QWidget;
    auto* Layout = new QVBoxLayout(Page);
    Layout->setContentsMargins(32, 24, 32, 24);
    Layout->setSpacing(16);

    auto* TitleLabel = new QLabel(QStringLiteral("General"));
    TitleLabel->setObjectName(QStringLiteral("sectionTitle"));
    Layout->addWidget(TitleLabel);
    auto* SubtitleLabel = new QLabel(QStringLiteral("Application preferences"));
    SubtitleLabel->setObjectName(QStringLiteral("sectionSubtitle"));
    Layout->addWidget(SubtitleLabel);

    // Startup
    auto* StartupForm = new QFormLayout;
    StartupForm->setSpacing(12);
    AutoStartCheck = new QCheckBox(QStringLiteral("Launch at Windows startup"));
    connect(AutoStartCheck, &QCheckBox::toggled, this, [this]() { MarkDirty(); });
    StartupForm->addRow(QString(), AutoStartCheck);
    Layout->addWidget(CreateCard(QStringLiteral("STARTUP"), StartupForm));

    // Icon folder
    auto* IconForm = new QVBoxLayout;
    IconForm->setSpacing(8);
    IconFolderLabel = new QLabel(QStringLiteral("Icon folder: ") + GetIconFolderPath());
    IconFolderLabel->setWordWrap(true);
    IconForm->addWidget(IconFolderLabel);
    auto* OpenIconFolderBtn = new QPushButton(QStringLiteral("Open Icon Folder"));
    connect(OpenIconFolderBtn, &QPushButton::clicked, this, [this]() 
    {
        QDir Directory(GetIconFolderPath());
        if (!Directory.exists()) 
        {
            Directory.mkpath(".");
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(GetIconFolderPath()));
    });
    IconForm->addWidget(OpenIconFolderBtn);
    auto* HintLabel = new QLabel(QStringLiteral(
        "Place .png, .jpg, .svg, or .ico files in this folder.\n"
        "They will appear in the icon dropdown when editing items."));
    HintLabel->setStyleSheet(QStringLiteral("color: #5a5a7a; font-size: 11px; background: transparent; border: none;"));
    HintLabel->setWordWrap(true);
    IconForm->addWidget(HintLabel);
    Layout->addWidget(CreateCard(QStringLiteral("ICONS"), IconForm));

    // Data Management
    auto* DataForm = new QVBoxLayout;
    DataForm->setSpacing(8);
    
    auto* BtnRow_Config = new QHBoxLayout;
    BtnRow_Config->setSpacing(12);
    
    auto* ExportBtn = new QPushButton(QStringLiteral("Export Configuration..."));
    ExportBtn->setObjectName(QStringLiteral("ghostBtn"));
    connect(ExportBtn, &QPushButton::clicked, this, [this]() 
    {
        QString Path = QFileDialog::getSaveFileName(this, QStringLiteral("Export Config"), 
            QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/GoPieMenu_Backup.json", 
            QStringLiteral("JSON Files (*.json)"));
        if (!Path.isEmpty()) 
        {
            QFile ConfigFile(Path);
            if (ConfigFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) 
            {
                ConfigFile.write(ConfigMgr->GetConfig().Serialize());
                ConfigFile.close();
                QMessageBox::information(this, QStringLiteral("Export Success"), QStringLiteral("Configuration exported successfully."));
            }
        }
    });
    
    auto* ImportBtn = new QPushButton(QStringLiteral("Import Configuration..."));
    ImportBtn->setObjectName(QStringLiteral("dangerBtn"));
    connect(ImportBtn, &QPushButton::clicked, this, [this]() 
    {
        if (QMessageBox::question(this, QStringLiteral("Import Config"), 
            QStringLiteral("Importing will completely overwrite your current settings. Continue?")) == QMessageBox::Yes) 
        {
            QString Path = QFileDialog::getOpenFileName(this, QStringLiteral("Import Config"), 
                QString(), QStringLiteral("JSON Files (*.json)"));
            if (!Path.isEmpty()) 
            {
                QFile ConfigFile(Path);
                if (ConfigFile.open(QIODevice::ReadOnly)) 
                {
                    auto NewConfigData = AppConfig::Deserialize(ConfigFile.readAll());
                    ConfigMgr->GetConfigMutable() = NewConfigData;
                    ConfigMgr->SaveCurrentConfig();
                    QMessageBox::information(this, QStringLiteral("Import Success"), QStringLiteral("Configuration imported. The app will now refresh."));
                    
                    bIsUpdatingUI = true;
                     PopulateProfileList();
                     LoadProfile(0);
                     LoadStyleSettings();
                     LoadGeneralSettings();
                     bIsUpdatingUI = false;
                 }
             }
         }
     });
     
     BtnRow_Config->addWidget(ExportBtn);
     BtnRow_Config->addWidget(ImportBtn);
     DataForm->addLayout(BtnRow_Config);
     Layout->addWidget(CreateCard(QStringLiteral("DATA MANAGEMENT"), DataForm));

     Layout->addStretch();

     return Page;
}

void SettingsWindow::PopulateProfileList()
{
    bIsUpdatingUI = true;
    ProfileList->clear();
    const auto& Profiles = ConfigMgr->GetConfig().Profiles;
    for (const auto& Pr : Profiles) 
    {
        ProfileList->addItem(Pr.Name);
    }
    bIsUpdatingUI = false;
}

void SettingsWindow::LoadProfile(int Index)
{
    if (Index < 0 || Index >= static_cast<int>(ConfigMgr->GetConfig().Profiles.size())) 
    {
        CurrentProfileIdx = -1;
        return;
    }

    CurrentProfileIdx = Index;
    const auto& P = ConfigMgr->GetConfig().Profiles[Index];

    bIsUpdatingUI = true;
    ProfileNameEdit->setText(P.Name);
    ProfileHotkeyRecorder->SetTrigger(P.Trigger);
    ProfileEnabledCheck->setChecked(P.bEnabled);
    AppFilterEdit->setText(P.AppFilter.join(QStringLiteral(", ")));

    PopulateItemList();
    PiePreview->SetConfig(P, ConfigMgr->GetConfig().GlobalStyle);
    bIsUpdatingUI = false;
}

void SettingsWindow::SaveCurrentProfile()
{
    if (CurrentProfileIdx < 0 || CurrentProfileIdx >= static_cast<int>(ConfigMgr->GetConfig().Profiles.size())) 
    {
        return;
    }

    auto& P = ConfigMgr->GetConfigMutable().Profiles[CurrentProfileIdx];
    P.Name = ProfileNameEdit->text();
    P.Trigger = ProfileHotkeyRecorder->GetTrigger();
    P.bEnabled = ProfileEnabledCheck->isChecked();
    P.AppFilter = AppFilterEdit->text().split(QStringLiteral(","), Qt::SkipEmptyParts);
    for (auto& s : P.AppFilter) 
    {
        s = s.trimmed();
    }
}

void SettingsWindow::OnAddProfile()
{
    PieMenuConfig NewProfile;
    NewProfile.Id   = QUuid::createUuid().toString(QUuid::WithoutBraces);
    NewProfile.Name = QStringLiteral("Pie Menu %1").arg(ConfigMgr->GetConfig().Profiles.size() + 1);
    
    ConfigMgr->AddProfile(std::move(NewProfile));
    
    PopulateProfileList();
    ProfileList->setCurrentRow(ProfileList->count() - 1);
    MarkDirty();
}

void SettingsWindow::OnRemoveProfile()
{
    if (CurrentProfileIdx < 0) 
    {
        return;
    }

    auto& Profiles = ConfigMgr->GetConfig().Profiles;
    if (Profiles.size() <= 1) 
    {
        QMessageBox::warning(this, QStringLiteral("Cannot Remove"),
                             QStringLiteral("At least one profile must remain."));
        return;
    }

    QString Id = Profiles[CurrentProfileIdx].Id;
    CurrentProfileIdx = -1;
    ConfigMgr->RemoveProfile(Id);
    
    PopulateProfileList();
    if (ProfileList->count() > 0) 
    {
        ProfileList->setCurrentRow(0);
    }
    MarkDirty();
}

void SettingsWindow::OnDuplicateProfile()
{
    const auto& Profiles = ConfigMgr->GetConfig().Profiles;
    if (CurrentProfileIdx < 0 || CurrentProfileIdx >= static_cast<int>(Profiles.size())) 
    {
        return;
    }

    SaveCurrentProfile();
    PieMenuConfig Clone = Profiles[CurrentProfileIdx];
    Clone.Id    = QUuid::createUuid().toString(QUuid::WithoutBraces);
    Clone.Name += QStringLiteral(" (Copy)");
    
    ConfigMgr->AddProfile(std::move(Clone));
    
    PopulateProfileList();
    ProfileList->setCurrentRow(ProfileList->count() - 1);
    MarkDirty();
}

void SettingsWindow::PopulateItemList()
{
    if (CurrentProfileIdx < 0 || CurrentProfileIdx >= static_cast<int>(ConfigMgr->GetConfig().Profiles.size())) 
    {
        return;
    }

    ItemTree->clear();
    const auto& P = ConfigMgr->GetConfig().Profiles[CurrentProfileIdx];

    for (const auto& Item : P.Items) 
    {
        auto* Node = new QTreeWidgetItem(ItemTree);
        Node->setText(0, Item.Name);
        Node->setData(0, Qt::UserRole, Item.Id);
        
        // Mark folders with a folder icon so they're obvious even when collapsed
        if (Item.Action == ActionType::ListMenu) 
        {
            Node->setIcon(0, ItemTree->style()->standardIcon(QStyle::SP_DirIcon));
        }

        for (const auto& Sub : Item.SubItems) 
        {
            auto* SubNode = new QTreeWidgetItem(Node);
            SubNode->setText(0, Sub.Name);
            SubNode->setData(0, Qt::UserRole, Sub.Id);
        }
    }
    ItemTree->expandAll();
    ItemEditorPanel->setEnabled(false);
}

PieItem* SettingsWindow::FindItemById(const QString& Id)
{
    if (CurrentProfileIdx < 0 || CurrentProfileIdx >= static_cast<int>(ConfigMgr->GetConfig().Profiles.size())) 
    {
        return nullptr;
    }
    
    auto& Items = ConfigMgr->GetConfigMutable().Profiles[CurrentProfileIdx].Items;
    for (auto& Item : Items) 
    {
        if (Item.Id == Id) 
        {
            return &Item;
        }
        for (auto& Sub : Item.SubItems) 
        {
            if (Sub.Id == Id) 
            {
                return &Sub;
            }
        }
    }
    return nullptr;
}

void SettingsWindow::SyncTreeToItems()
{
    if (CurrentProfileIdx < 0 || CurrentProfileIdx >= static_cast<int>(ConfigMgr->GetConfig().Profiles.size())) 
    {
        return;
    }
    
    std::vector<PieItem> NewItems;
    int TopCount = ItemTree->topLevelItemCount();
    
    for (int i = 0; i < TopCount; ++i) 
    {
        auto* TopNode = ItemTree->topLevelItem(i);
        QString TopId = TopNode->data(0, Qt::UserRole).toString();
        
        PieItem* TopOrig = FindItemById(TopId);
        if (!TopOrig) 
        {
            continue;
        }
        
        PieItem TopCopy = *TopOrig;
        TopCopy.SubItems.clear(); // Rebuild sub items
        
        if (TopCopy.Action != ActionType::ListMenu && TopNode->childCount() > 0) 
        {
            TopCopy.Action = ActionType::ListMenu;
        }
        
        int ChildCount = TopNode->childCount();
        for (int j = 0; j < ChildCount; ++j) 
        {
            auto* SubNode = TopNode->child(j);
            QString SubId = SubNode->data(0, Qt::UserRole).toString();
            PieItem* SubOrig = FindItemById(SubId);
            if (SubOrig) 
            {
                PieItem SubCopy = *SubOrig;
                SubCopy.SubItems.clear(); // Sub-items cannot have children
                TopCopy.SubItems.push_back(std::move(SubCopy));
            }
        }
        NewItems.push_back(std::move(TopCopy));
    }
    
    ConfigMgr->GetConfigMutable().Profiles[CurrentProfileIdx].Items = std::move(NewItems);
}

void SettingsWindow::LoadItem(QTreeWidgetItem* Node, int /*Column*/)
{
    if (CurrentItemNode) 
    {
        SaveCurrentItem();
    }
    bIsUpdatingUI = true;
    CurrentItemNode = Node;

    if (!Node) 
    {
        ItemEditorPanel->setEnabled(false);
        bIsUpdatingUI = false;
        return;
    }

    QString Id = Node->data(0, Qt::UserRole).toString();
    PieItem* ItemPtr = FindItemById(Id);

    if (!ItemPtr) 
    {
        bIsUpdatingUI = false;
        return;
    }

    auto& Item = *ItemPtr;
    ItemEditorPanel->setEnabled(true);
    
    ItemNameEdit->setText(Item.Name);

    CurrentIconPath = Item.Icon;
    if (CurrentIconPath.isEmpty()) 
    {
        ItemIconButton->setText(QStringLiteral("Select Icon..."));
        ItemIconButton->setIcon(QIcon());
    } 
    else 
    {
        ItemIconButton->setText(CurrentIconPath);
        ItemIconButton->setIcon(QIcon(GetIconFolderPath() + "/" + CurrentIconPath));
    }

    int ComboIdx = ItemActionTypeCombo->findData(static_cast<int>(Item.Action));
    if (ComboIdx >= 0) 
    {
        ItemActionTypeCombo->setCurrentIndex(ComboIdx);
    }
    
    UpdateItemEditorState();
    
    ItemActionDataEdit->setText(Item.ActionData);
    if (Item.Action == ActionType::SendHotkey) 
    {
        ItemHotkeyRecorder->SetShortcutString(Item.ActionData);
    }
    ItemActionArgsEdit->setText(Item.Arguments);
    ItemColorPicker->SetColor(Item.Color.value_or(QColor(Qt::transparent)));
    
    bIsUpdatingUI = false;
}

void SettingsWindow::SaveCurrentItem()
{
    if (!CurrentItemNode) 
    {
        return;
    }
    
    QString Id = CurrentItemNode->data(0, Qt::UserRole).toString();
    PieItem* ItemPtr = FindItemById(Id);

    if (!ItemPtr) 
    {
        return;
    }

    auto& Item = *ItemPtr;
    Item.Name       = ItemNameEdit->text();
    
    // Always store only the filename for portability
    if (!CurrentIconPath.isEmpty() && CurrentIconPath != QStringLiteral("(none)"))
    {
        Item.Icon = QFileInfo(CurrentIconPath).fileName();
    }
    else
    {
        Item.Icon.clear();
    }

    if (auto TypeData = ItemActionTypeCombo->currentData(); TypeData.isValid()) 
    {
        Item.Action = static_cast<ActionType>(TypeData.toInt());
    }
    
    if (Item.Action == ActionType::SendHotkey) 
    {
        Item.ActionData = ItemHotkeyRecorder->GetShortcutString();
    } 
    else 
    {
        Item.ActionData = ItemActionDataEdit->text();
    }
    Item.Arguments = ItemActionArgsEdit->text();
    auto Color = ItemColorPicker->GetColor();
    // Allow transparent but still valid colors (meaning user actually picked something, alpha > 0)
    Item.Color = (Color.isValid() && Color.alpha() > 0) ? std::optional(Color) : std::nullopt;
    
    // Also explicitly force update style so the button previews correctly right away
    
    CurrentItemNode->setText(0, Item.Name);
}

void SettingsWindow::OnAddItem()
{
    if (CurrentProfileIdx < 0 || CurrentProfileIdx >= static_cast<int>(ConfigMgr->GetConfig().Profiles.size())) 
    {
        return;
    }
    
    // Save current item first, then clear selection to avoid stale pointers
    if (CurrentItemNode) 
    {
        SaveCurrentItem();
    }
    CurrentItemNode = nullptr;
    
    auto& P = ConfigMgr->GetConfigMutable().Profiles[CurrentProfileIdx];
    
    // Check if a folder (ListMenu) is currently selected — add inside it
    auto* SelectedNode = ItemTree->currentItem();
    if (SelectedNode) 
    {
        // Determine the top-level node (could be the item itself or its parent)
        QTreeWidgetItem* TopNode = SelectedNode->parent() ? SelectedNode->parent() : SelectedNode;
        QString TopId = TopNode->data(0, Qt::UserRole).toString();
        PieItem* TopItem = FindItemById(TopId);
        
        if (TopItem && TopItem->Action == ActionType::ListMenu) 
        {
            // Add sub-item inside this folder
            TopItem->SubItems.push_back(PieItem::Create(QStringLiteral("New Item"), ActionType::LaunchApp, {}));
            PopulateItemList();
            
            // Select the newly added sub-item
            for (int i = 0; i < ItemTree->topLevelItemCount(); ++i) 
            {
                auto* Node = ItemTree->topLevelItem(i);
                if (Node->data(0, Qt::UserRole).toString() == TopId && Node->childCount() > 0) 
                {
                    ItemTree->setCurrentItem(Node->child(Node->childCount() - 1));
                    LoadItem(ItemTree->currentItem(), 0);
                    break;
                }
            }
            PiePreview->SetConfig(P, ConfigMgr->GetConfig().GlobalStyle);
            MarkDirty();
            return;
        }
    }
    
    // Default: add as top-level item
    if (P.Items.size() >= PieMenuConfig::MaxItems) 
    {
        QMessageBox::information(this, QStringLiteral("Limit Reached"),
            QStringLiteral("Maximum %1 items per pie menu.").arg(PieMenuConfig::MaxItems));
        return;
    }
    P.Items.push_back(PieItem::Create(QStringLiteral("New Item"), ActionType::LaunchApp, {}));
    PopulateItemList();
    
    if (ItemTree->topLevelItemCount() > 0) 
    {
        ItemTree->setCurrentItem(ItemTree->topLevelItem(ItemTree->topLevelItemCount() - 1));
        LoadItem(ItemTree->currentItem(), 0);
    }
    
    PiePreview->SetConfig(P, ConfigMgr->GetConfig().GlobalStyle);
    MarkDirty();
}

void SettingsWindow::OnAddSubItem()
{
    if (CurrentProfileIdx < 0 || CurrentProfileIdx >= static_cast<int>(ConfigMgr->GetConfig().Profiles.size())) 
    {
        return;
    }
    
    auto& P = ConfigMgr->GetConfigMutable().Profiles[CurrentProfileIdx];
    
    // Save current item first, then clear selection to avoid stale pointers
    if (CurrentItemNode) 
    {
        SaveCurrentItem();
    }
    CurrentItemNode = nullptr;
    
    auto* SelectedNode = ItemTree->currentItem();
    
    if (!SelectedNode) 
    {
        // No selection: create an empty folder at top level
        PieItem Folder = PieItem::Create(QStringLiteral("New Folder"), ActionType::ListMenu, {});
        P.Items.push_back(std::move(Folder));
        PopulateItemList();
        
        if (ItemTree->topLevelItemCount() > 0) 
        {
            ItemTree->setCurrentItem(ItemTree->topLevelItem(ItemTree->topLevelItemCount() - 1));
            LoadItem(ItemTree->currentItem(), 0);
        }
        PiePreview->SetConfig(P, ConfigMgr->GetConfig().GlobalStyle);
        MarkDirty();
        return;
    }
    
    // If a top-level non-folder item is selected, wrap it into a new folder
    if (!SelectedNode->parent()) 
    {
        QString SelectedId = SelectedNode->data(0, Qt::UserRole).toString();
        
        // Find the item's index in the top-level items vector
        int ItemIdx = -1;
        for (int i = 0; i < static_cast<int>(P.Items.size()); ++i) 
        {
            if (P.Items[i].Id == SelectedId) 
            {
                ItemIdx = i;
                break;
            }
        }
        
        if (ItemIdx >= 0) 
        {
            if (P.Items[ItemIdx].Action == ActionType::ListMenu) 
            {
                // Already a folder — just add a sub-item inside
                P.Items[ItemIdx].SubItems.push_back(PieItem::Create(QStringLiteral("New Sub-Item"), ActionType::LaunchApp, {}));
            } 
            else 
            {
                // Wrap the selected item into a new folder
                PieItem WrappedItem = std::move(P.Items[ItemIdx]);
                PieItem Folder = PieItem::Create(QStringLiteral("New Folder"), ActionType::ListMenu, {});
                Folder.SubItems.push_back(std::move(WrappedItem));
                P.Items[ItemIdx] = std::move(Folder);
            }
            
            QString FolderId = P.Items[ItemIdx].Id;
            PopulateItemList();
            
            // Select the folder or newly added item
            for (int i = 0; i < ItemTree->topLevelItemCount(); ++i) 
            {
                auto* Node = ItemTree->topLevelItem(i);
                if (Node->data(0, Qt::UserRole).toString() == FolderId) 
                {
                    Node->setExpanded(true);
                    if (P.Items[ItemIdx].Action == ActionType::ListMenu && !P.Items[ItemIdx].SubItems.empty())
                    {
                         ItemTree->setCurrentItem(Node->child(Node->childCount() - 1));
                    }
                    else
                    {
                         ItemTree->setCurrentItem(Node);
                    }
                    LoadItem(ItemTree->currentItem(), 0);
                    break;
                }
            }
        }
    } 
    else 
    {
        // A sub-item is selected — add a sibling sub-item in the parent folder
        QTreeWidgetItem* ParentNode = SelectedNode->parent();
        QString ParentId = ParentNode->data(0, Qt::UserRole).toString();
        PieItem* ParentItem = FindItemById(ParentId);
        
        if (ParentItem && ParentItem->Action == ActionType::ListMenu) 
        {
            ParentItem->SubItems.push_back(PieItem::Create(QStringLiteral("New Sub-Item"), ActionType::LaunchApp, {}));
            PopulateItemList();
            
            for (int i = 0; i < ItemTree->topLevelItemCount(); ++i) 
            {
                auto* Node = ItemTree->topLevelItem(i);
                if (Node->data(0, Qt::UserRole).toString() == ParentId && Node->childCount() > 0) 
                {
                    ItemTree->setCurrentItem(Node->child(Node->childCount() - 1));
                    LoadItem(ItemTree->currentItem(), 0);
                    break;
                }
            }
        }
    }
    
    PiePreview->SetConfig(P, ConfigMgr->GetConfig().GlobalStyle);
    MarkDirty();
}

void SettingsWindow::OnRemoveItem()
{
    if (!CurrentItemNode) 
    {
        return;
    }

    QString TargetId = CurrentItemNode->data(0, Qt::UserRole).toString();

    auto& Profiles = ConfigMgr->GetConfigMutable().Profiles;
    if (CurrentProfileIdx < 0 || CurrentProfileIdx >= static_cast<int>(Profiles.size())) 
    {
        return;
    }
    auto& ItemsList = Profiles[CurrentProfileIdx].Items;

    bool bRemoved = false;
    for (auto It = ItemsList.begin(); It != ItemsList.end(); ++It) 
    {
        if (It->Id == TargetId) 
        {
            if (ItemsList.size() <= static_cast<size_t>(PieMenuConfig::MinItems)) 
            {
                QMessageBox::information(this, QStringLiteral("Limit Reached"),
                    QStringLiteral("Minimum %1 items required.").arg(PieMenuConfig::MinItems));
                return;
            }
            ItemsList.erase(It);
            bRemoved = true;
            break;
        }
        for (auto SubIt = It->SubItems.begin(); SubIt != It->SubItems.end(); ++SubIt) 
        {
            if (SubIt->Id == TargetId) 
            {
                It->SubItems.erase(SubIt);
                bRemoved = true;
                break;
            }
        }
        if (bRemoved) 
        {
            break;
        }
    }
    
    if (bRemoved) 
    {
        CurrentItemNode = nullptr;
        PopulateItemList();
        PiePreview->SetConfig(Profiles[CurrentProfileIdx], ConfigMgr->GetConfig().GlobalStyle);
        MarkDirty();
    }
}

// =============================================================================
// STYLE SETTINGS
// =============================================================================

void SettingsWindow::LoadStyleSettings()
{
    const auto& S = ConfigMgr->GetConfig().GlobalStyle;
    
    OuterRadiusSpin->setValue(S.OuterRadius);
    InnerRadiusSpin->setValue(S.InnerRadius);
    IconSizeSpin->setValue(S.IconSize);
    GapAngleSpin->setValue(S.GapAngle);
    AnimDurationSpin->setValue(S.AnimationDuration);
    OpacitySpin->setValue(S.Opacity);
    FontSizeSpin->setValue(S.FontSize);
    TextOutlineThicknessSpin->setValue(S.TextOutlineThickness);
    
    BGColorPicker->SetColor(S.BackgroundColor);
    SectorColorPicker->SetColor(S.SectorColor);
    HoverColorPicker->SetColor(S.HoverColor);
    TextColorPicker->SetColor(S.TextColor);
    CenterColorPicker->SetColor(S.CenterColor);
    AutoContrastCheck->setChecked(S.bAutoContrast);
}

void SettingsWindow::SaveStyleSettings()
{
    auto& S = ConfigMgr->GetConfigMutable().GlobalStyle;
    
    S.OuterRadius      = OuterRadiusSpin->value();
    S.InnerRadius      = InnerRadiusSpin->value();
    S.IconSize         = IconSizeSpin->value();
    S.GapAngle         = GapAngleSpin->value();
    S.AnimationDuration = AnimDurationSpin->value();
    S.Opacity          = OpacitySpin->value();
    S.FontSize         = FontSizeSpin->value();
    S.TextOutlineThickness = TextOutlineThicknessSpin->value();
    
    S.BackgroundColor = BGColorPicker->GetColor();
    S.SectorColor     = SectorColorPicker->GetColor();
    S.HoverColor      = HoverColorPicker->GetColor();
    S.TextColor       = TextColorPicker->GetColor();
    S.CenterColor     = CenterColorPicker->GetColor();
    S.bAutoContrast    = AutoContrastCheck->isChecked();
}

// =============================================================================
// GENERAL SETTINGS
// =============================================================================

void SettingsWindow::LoadGeneralSettings()
{
    AutoStartCheck->setChecked(ConfigMgr->GetConfig().bStartWithWindows);
}

void SettingsWindow::SaveGeneralSettings()
{
    ConfigMgr->GetConfigMutable().bStartWithWindows = AutoStartCheck->isChecked();
}

// =============================================================================
// APPLY / DISCARD
// =============================================================================

void SettingsWindow::ApplyChanges()
{
    SaveCurrentItem();
    SaveCurrentProfile();
    SaveStyleSettings();
    SaveGeneralSettings();
    
    ConfigMgr->SaveCurrentConfig();
    bIsDirty = false;
    HideSaveBar();
    
    emit ConfigUpdated();
    
    // Refresh preview
    if (CurrentProfileIdx >= 0) 
    {
        PiePreview->SetConfig(ConfigMgr->GetConfig().Profiles[CurrentProfileIdx], ConfigMgr->GetConfig().GlobalStyle);
    }
}

void SettingsWindow::DiscardChanges()
{
    bIsDirty = false;
    HideSaveBar();
    
    ConfigMgr->Reload();
    
    bIsUpdatingUI = true;
    PopulateProfileList();
    if (!ConfigMgr->GetConfig().Profiles.empty()) 
    {
        CurrentProfileIdx = std::min(CurrentProfileIdx, static_cast<int>(ConfigMgr->GetConfig().Profiles.size() - 1));
        if (CurrentProfileIdx < 0) 
        {
            CurrentProfileIdx = 0;
        }
        ProfileList->setCurrentRow(CurrentProfileIdx);
        LoadProfile(CurrentProfileIdx);
    }
    
    LoadStyleSettings();
    LoadGeneralSettings();
    
    bIsUpdatingUI = false;
}

} // namespace gpm
