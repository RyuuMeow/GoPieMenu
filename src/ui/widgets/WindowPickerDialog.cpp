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
    setFixedSize(440, 520);
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
        "  background: #ffffff; border: 1px solid #d9e1ed; border-radius: 8px;"
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
    TitleLabel->setStyleSheet(QStringLiteral("color: #253247; font-size: 14px; font-weight: 600; border: none;"));
    ContentLayout->addWidget(TitleLabel);

    SearchEdit = new QLineEdit;
    SearchEdit->setPlaceholderText(QStringLiteral("Search processes..."));
    SearchEdit->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #f8faff; color: #253247; border: 1px solid #d9e1ed; "
        "border-radius: 6px; padding: 8px 12px; font-size: 14px; margin-bottom: 4px; }"
        "QLineEdit:focus { border-color: #527fc5; }"
    ));
    connect(SearchEdit, &QLineEdit::textChanged, this, &WindowPickerDialog::FilterWindows);
    ContentLayout->addWidget(SearchEdit);

    ListWidget = new QListWidget;
    ListWidget->setIconSize(QSize(24, 24));
    ListWidget->setStyleSheet(QStringLiteral(
        "QListWidget {"
        "  background: #f8faff; border: 1px solid #e4e9f1; border-radius: 6px;"
        "  padding: 4px; outline: none;"
        "}"
        "QListWidget::item {"
        "  color: #253247; font-size: 14px; border-radius: 4px; padding: 6px;"
        "}"
        "QListWidget::item:hover {"
        "  background: #d9e1ed;"
        "}"
        "QListWidget::item:selected {"
        "  background: #527fc5; color: #ffffff;"
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
        "QPushButton { background: transparent; color: #62728a; border: 1px solid #d9e1ed; border-radius: 6px; padding: 6px; font-size: 14px; }"
        "QPushButton:hover { background: #d9e1ed; color: #253247; }"
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
