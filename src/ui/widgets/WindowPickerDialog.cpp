// =============================================================================
// GoPieMenu - Window Picker Dialog Implementation
// =============================================================================

#include "WindowPickerDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QFileInfo>
#include <QSet>
#include <QFileIconProvider>

#include <windows.h>
#include <psapi.h>

namespace gpm 
{

WindowPickerDialog::WindowPickerDialog(QWidget* Parent)
    : QDialog(Parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(360, 480);
    SetupUI();
    ScanWindows();
}

void WindowPickerDialog::SetupUI()
{
    auto* RootLayoutPointer = new QVBoxLayout(this);
    RootLayoutPointer->setContentsMargins(12, 12, 12, 12);

    auto* Container = new QWidget(this);
    Container->setObjectName(QStringLiteral("container"));
    Container->setStyleSheet(QStringLiteral(
        "QWidget#container {"
        "  background: #1a1a28; border: 1px solid #2a2a40; border-radius: 8px;"
        "}"
    ));

    auto* ShadowEffect = new QGraphicsDropShadowEffect(this);
    ShadowEffect->setBlurRadius(20);
    ShadowEffect->setColor(QColor(0, 0, 0, 160));
    ShadowEffect->setOffset(0, 4);
    Container->setGraphicsEffect(ShadowEffect);

    auto* ContentLayout = new QVBoxLayout(Container);
    ContentLayout->setContentsMargins(12, 12, 12, 12);
    ContentLayout->setSpacing(8);

    auto* TitleLabel = new QLabel(QStringLiteral("Select Running Application"));
    TitleLabel->setStyleSheet(QStringLiteral("color: #e0e0f0; font-size: 14px; font-weight: 600; border: none;"));
    ContentLayout->addWidget(TitleLabel);

    SearchEdit = new QLineEdit;
    SearchEdit->setPlaceholderText(QStringLiteral("Search processes..."));
    SearchEdit->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #131320; color: #d0d0e8; border: 1px solid #2a2a40; "
        "border-radius: 6px; padding: 8px 12px; font-size: 12px; margin-bottom: 4px; }"
        "QLineEdit:focus { border-color: #4f46e5; }"
    ));
    connect(SearchEdit, &QLineEdit::textChanged, this, &WindowPickerDialog::FilterWindows);
    ContentLayout->addWidget(SearchEdit);

    ListWidget = new QListWidget;
    ListWidget->setIconSize(QSize(24, 24));
    ListWidget->setStyleSheet(QStringLiteral(
        "QListWidget {"
        "  background: #131320; border: 1px solid #1e1e30; border-radius: 6px;"
        "  padding: 4px; outline: none;"
        "}"
        "QListWidget::item {"
        "  color: #c0c0d8; font-size: 12px; border-radius: 4px; padding: 6px;"
        "}"
        "QListWidget::item:hover {"
        "  background: #2a2a40;"
        "}"
        "QListWidget::item:selected {"
        "  background: #4f46e5; color: #ffffff;"
        "}"
    ));
    connect(ListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* Item) 
    {
        SelectedProcessName = Item->data(Qt::UserRole).toString();
        accept();
    });
    ContentLayout->addWidget(ListWidget, 1);

    auto* CancelBtn = new QPushButton(QStringLiteral("Cancel"));
    CancelBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; color: #a0a0c0; border: 1px solid #2a2a40; border-radius: 6px; padding: 6px; font-size: 12px; }"
        "QPushButton:hover { background: #2a2a40; color: #e0e0f0; }"
    ));
    CancelBtn->setCursor(Qt::PointingHandCursor);
    connect(CancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    ContentLayout->addWidget(CancelBtn);

    RootLayoutPointer->addWidget(Container);
}

struct EnumArg 
{
    QListWidget* ListWidgetPointer;
    QSet<QString> AddedProcesses;
};

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) 
{
    auto* Arg = reinterpret_cast<EnumArg*>(lParam);

    if (!IsWindowVisible(hwnd)) 
    {
        return TRUE;
    }
    
    // Ignore tool windows and other invisible elements
    LONG ExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if ((ExStyle & WS_EX_TOOLWINDOW) != 0) 
    {
        return TRUE;
    }

    DWORD ProcessId = 0;
    GetWindowThreadProcessId(hwnd, &ProcessId);
    if (ProcessId == 0) 
    {
        return TRUE;
    }

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ProcessId);
    if (hProcess) 
    {
        WCHAR PathBuffer[MAX_PATH];
        DWORD Size = MAX_PATH;
        if (QueryFullProcessImageNameW(hProcess, 0, PathBuffer, &Size)) 
        {
            QString FullPath = QString::fromWCharArray(PathBuffer);
            QFileInfo Info(FullPath);
            QString ExeName = Info.fileName();
            
            // Exclude our own app and system apps
            if (ExeName.compare(QStringLiteral("GoPieMenu.exe"), Qt::CaseInsensitive) != 0 &&
                ExeName.compare(QStringLiteral("explorer.exe"), Qt::CaseInsensitive) != 0 &&
                !Arg->AddedProcesses.contains(ExeName)) 
            {
                QFileIconProvider Provider;
                QIcon AppIcon = Provider.icon(Info);
                
                auto* Item = new QListWidgetItem(AppIcon, ExeName);
                Item->setData(Qt::UserRole, ExeName);
                Arg->ListWidgetPointer->addItem(Item);
                Arg->AddedProcesses.insert(ExeName);
            }
        }
        CloseHandle(hProcess);
    }
    return TRUE;
}

void WindowPickerDialog::ScanWindows()
{
    ListWidget->clear();
    EnumArg Arg{ListWidget, QSet<QString>()};
    ::EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&Arg));
    ListWidget->sortItems();
}

void WindowPickerDialog::FilterWindows(const QString& InText)
{
    QString Query = InText.toLower();
    for (int i = 0; i < ListWidget->count(); ++i) 
    {
        auto* Item = ListWidget->item(i);
        Item->setHidden(!Query.isEmpty() && !Item->text().toLower().contains(Query));
    }
}

} // namespace gpm
