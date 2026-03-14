// =============================================================================
// GoPieMenu - Icon Picker Dialog Implementation
// =============================================================================

#include "IconPickerDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <thread>

namespace gpm 
{

IconPickerDialog::IconPickerDialog(const QString& InIconFolderPath, QWidget* Parent)
    : QDialog(Parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    , IconDir(InIconFolderPath)
{
    setAttribute(Qt::WA_TranslucentBackground);
    // 4 columns * 76px = 304px + 12px (margins) + 8px (border) + 20px (scrollbar) = 348
    setFixedSize(348, 420);
    SetupUI();
    ScanIcons();
}

void IconPickerDialog::SetupUI()
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

    auto* TitleRow = new QHBoxLayout;
    auto* TitleLabel = new QLabel(QStringLiteral("Select an Icon"));
    TitleLabel->setStyleSheet(QStringLiteral("color: #e0e0f0; font-size: 14px; font-weight: 600; border: none;"));
    TitleRow->addWidget(TitleLabel);
    TitleRow->addStretch();
    ContentLayout->addLayout(TitleRow);

    SearchEdit = new QLineEdit;
    SearchEdit->setPlaceholderText(QStringLiteral("Search icons..."));
    SearchEdit->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #131320; color: #d0d0e8; border: 1px solid #2a2a40; "
        "border-radius: 6px; padding: 8px 12px; font-size: 12px; margin-bottom: 4px; }"
        "QLineEdit:focus { border-color: #4f46e5; }"
    ));
    connect(SearchEdit, &QLineEdit::textChanged, this, &IconPickerDialog::FilterIcons);
    ContentLayout->addWidget(SearchEdit);

    ListWidget = new QListWidget;
    ListWidget->setViewMode(QListView::IconMode);
    ListWidget->setIconSize(QSize(32, 32));
    ListWidget->setGridSize(QSize(76, 76));
    ListWidget->setSpacing(0);
    ListWidget->setResizeMode(QListView::Adjust);
    ListWidget->setMovement(QListView::Static);
    
    ListWidget->setStyleSheet(QStringLiteral(
        "QListWidget {"
        "  background: #131320; border: 1px solid #1e1e30; border-radius: 6px;"
        "  padding: 8px; outline: none;"
        "}"
        "QListWidget::item {"
        "  color: #c0c0d8; font-size: 11px; border-radius: 6px;"
        "  width: 64px; height: 64px;"
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
        SelectedIconPath = Item->data(Qt::UserRole).toString();
        accept();
    });

    ContentLayout->addWidget(ListWidget, 1);

    auto* NoneBtn = new QPushButton(QStringLiteral("None (Clear Icon)"));
    NoneBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: transparent; color: #a0a0c0; border: 1px solid #2a2a40; border-radius: 6px; padding: 6px; font-size: 12px; }"
        "QPushButton:hover { background: #2a2a40; color: #e0e0f0; }"
    ));
    NoneBtn->setCursor(Qt::PointingHandCursor);
    connect(NoneBtn, &QPushButton::clicked, this, [this]() 
    {
        SelectedIconPath = QString(); // Empty string means clear
        accept();
    });
    ContentLayout->addWidget(NoneBtn);

    RootLayoutPointer->addWidget(Container);
}

void IconPickerDialog::ScanIcons()
{
    ListWidget->clear();
    QDir Directory(IconDir);
    if (!Directory.exists()) 
    {
        Directory.mkpath(".");
    }

    QStringList Filters;
    Filters << "*.png" << "*.jpg" << "*.jpeg" << "*.svg" << "*.ico";
    auto Entries = Directory.entryInfoList(Filters, QDir::Files, QDir::Name);

    // Initial pass: create items with a blank icon
    QIcon Placeholder;
    for (const auto& Entry : Entries) 
    {
        QString Path = Entry.absoluteFilePath();
        QString Name = Entry.baseName();
        auto* Item = new QListWidgetItem(Placeholder, Name);
        Item->setData(Qt::UserRole, Path);
        Item->setToolTip(Entry.fileName());
        if (Name.length() > 9) 
        {
            Name = Name.left(8) + "…";
        }
        Item->setText(Name);
        Item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        ListWidget->addItem(Item);
    }

    // Spawn a thread to load images if they aren't cached
    QPointer<QListWidget> ListPtr(ListWidget);
    std::thread([Entries, ListPtr]() 
    {
        for (int i = 0; i < Entries.size(); ++i) 
        {
            if (!ListPtr) 
            {
                break;
            }
            
            QString Path = Entries[i].absoluteFilePath();
            
            // Loading QImages here. QImage loading is safe off-thread.
            QImage Img(Path);
            if (!Img.isNull()) 
            {
                // Return image to main thread
                QMetaObject::invokeMethod(ListPtr.data(), [ListPtr, i, Img]() 
                {
                    if (ListPtr && i < ListPtr->count()) 
                    {
                        auto* Item = ListPtr->item(i);
                        Item->setIcon(QIcon(QPixmap::fromImage(Img)));
                    }
                }, Qt::QueuedConnection);
            }
        }
    }).detach();
}

void IconPickerDialog::FilterIcons(const QString& InText)
{
    QString Query = InText.toLower();
    for (int i = 0; i < ListWidget->count(); ++i) 
    {
        auto* Item = ListWidget->item(i);
        QString Name = Item->toolTip().toLower(); // Use tooltip which stores full extension name
        Item->setHidden(!Query.isEmpty() && !Name.contains(Query));
    }
}

} // namespace gpm
