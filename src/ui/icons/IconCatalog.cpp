#include "IconCatalog.h"
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QRegularExpression>
#include <algorithm>

namespace gpm {
IconCatalog::IconCatalog(IconService* icons, QObject* parent) : QAbstractListModel(parent), Icons(icons) {
    Pool.setMaxThreadCount(1);
    RefreshTimer.setSingleShot(true); RefreshTimer.setInterval(200);
    connect(&RefreshTimer, &QTimer::timeout, this, &IconCatalog::refresh);
    connect(&Watcher, &QFileSystemWatcher::directoryChanged, this, [this] { RefreshTimer.start(); });
    connect(&Watcher, &QFileSystemWatcher::fileChanged, this, [this] { RefreshTimer.start(); });
    QTimer::singleShot(0, this, &IconCatalog::refresh);
}
IconCatalog::~IconCatalog() { Pool.waitForDone(); }
int IconCatalog::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : int(Filtered.size()); }
QVariant IconCatalog::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= Filtered.size()) return {};
    const auto& entry = Entries[Filtered[index.row()]];
    if (role == IconId) return entry.Id;
    if (role == DisplayName) return entry.Name;
    return {};
}
QHash<int, QByteArray> IconCatalog::roleNames() const { return {{IconId, "iconId"}, {DisplayName, "displayName"}}; }
void IconCatalog::setQuery(const QString& query) {
    if (Query == query) return;
    Query = query; emit queryChanged(); filter();
}
void IconCatalog::filter() {
    const auto words = Query.toCaseFolded().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    beginResetModel();
    Filtered.clear();
    for (int i = 0; i < Entries.size(); ++i) {
        bool match = true;
        for (const auto& word : words) if (!Entries[i].Search.contains(word)) { match = false; break; }
        if (match) Filtered.append(i);
    }
    endResetModel(); emit countChanged();
}
void IconCatalog::refresh() {
    if (Loading) { RefreshAgain = true; return; }
    Loading = true; emit loadingChanged();
    const auto roots = Icons->searchDirectories();
    Pool.start([this, roots] {
        QVector<Entry> entries;
        QSet<QString> seen;
        QStringList watched;
        for (const auto& root : roots) {
            QDir dir(root);
            const auto files = dir.entryInfoList({"*.svg", "*.png", "*.jpg", "*.jpeg", "*.ico"}, QDir::Files, QDir::Name);
            for (const auto& file : files) {
                if (seen.contains(file.fileName())) continue;
                seen.insert(file.fileName());
                auto name = file.completeBaseName().replace('-', ' ').replace('_', ' ');
                if (!name.isEmpty()) name[0] = name[0].toUpper();
                entries.append({file.fileName(), name, (name + " " + file.fileName()).toCaseFolded()});
                if (!root.startsWith(":/")) watched.append(file.absoluteFilePath());
            }
        }
        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) { return a.Name < b.Name; });
        QMetaObject::invokeMethod(this, [this, entries = std::move(entries), watched, roots] {
            Entries = entries;
            filter();
            if (!Watcher.files().isEmpty()) Watcher.removePaths(Watcher.files());
            if (!watched.isEmpty()) Watcher.addPaths(watched);
            for (const auto& root : roots)
                if (!root.startsWith(":/") && QDir(root).exists() && !Watcher.directories().contains(root)) Watcher.addPath(root);
            Icons->invalidate(); ++Revision; emit revisionChanged();
            Loading = false; emit loadingChanged();
            if (RefreshAgain) { RefreshAgain = false; refresh(); }
        }, Qt::QueuedConnection);
    });
}
}
