#pragma once
#include <QObject>
#include <QImage>
#include <QColor>
#include <QCache>
#include <QSet>
#include <QThreadPool>
#include <QQuickAsyncImageProvider>
#include <memory>

namespace gpm {
class IconBackend;

// Owns managed loading jobs. Construct before UI/engine; destroy after them.
class IconService : public QObject {
    Q_OBJECT
public:
    explicit IconService(QString directory = {}, QObject* parent = nullptr);
    ~IconService() override;
    QString directory() const { return Directory; }
    QStringList searchDirectories() const;
    QString request(const QString& id, int pixelSize, const QColor& tint);
    QImage cached(const QString& key) const;
    QQuickAsyncImageProvider* createProvider();
    void invalidate();
    std::shared_ptr<IconBackend> backend() const { return Backend; }

signals:
    void imageReady(const QString& key, const QImage& image);
    void invalidated();

private:
    QString Directory;
    std::shared_ptr<IconBackend> Backend;
    QCache<QString, QImage> Ready{16 * 1024}; // KiB, bounded independently of the library size.
    QSet<QString> Pending;
    quint64 Generation = 0;
};

class IconBackend {
public:
    explicit IconBackend(QString directory);
    ~IconBackend();
    QImage load(const QString& id, int pixelSize, const QColor& tint);
    void clear();
    QThreadPool Pool;
private:
    QString resolve(const QString& id) const;
    QString Directory;
    class CacheData;
    std::unique_ptr<CacheData> Data;
};
}
