#pragma once
#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QThreadPool>
#include <QTimer>
#include "IconService.h"

namespace gpm {
class IconCatalog : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)
public:
    explicit IconCatalog(IconService* icons, QObject* parent = nullptr);
    ~IconCatalog() override;
    enum Role { IconId = Qt::UserRole + 1, DisplayName };
    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    QString query() const { return Query; }
    void setQuery(const QString& query);
    bool loading() const { return Loading; }
    int count() const { return int(Filtered.size()); }
    int revision() const { return Revision; }
    Q_INVOKABLE void refresh();

signals:
    void queryChanged();
    void loadingChanged();
    void countChanged();
    void revisionChanged();

private:
    struct Entry { QString Id, Name, Search; };
    void filter();
    IconService* Icons;
    QVector<Entry> Entries;
    QVector<int> Filtered;
    QString Query;
    QThreadPool Pool;
    QFileSystemWatcher Watcher;
    QTimer RefreshTimer;
    bool Loading = false, RefreshAgain = false;
    int Revision = 0;
};
}
