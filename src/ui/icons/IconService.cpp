#include "IconService.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QPainter>
#include <QSvgRenderer>
#include <QUrl>
#include <QUrlQuery>
#include <QQuickTextureFactory>
#include <QStandardPaths>
#include <atomic>
#include <algorithm>

static void InitializeIconResources() { Q_INIT_RESOURCE(icons); }

namespace gpm {
class IconBackend::CacheData {
public:
    QMutex Mutex;
    QCache<QString, QImage> Images{64 * 1024}; // 64 MiB
};

IconBackend::IconBackend(QString directory) : Directory(std::move(directory)), Data(std::make_unique<CacheData>()) {
    Pool.setMaxThreadCount(2);
    Pool.setExpiryTimeout(10000);
}
IconBackend::~IconBackend() { Pool.waitForDone(); }
void IconBackend::clear() { QMutexLocker lock(&Data->Mutex); Data->Images.clear(); }
QString IconBackend::resolve(const QString& id) const {
    if (id.startsWith(":/") || QFileInfo(id).isAbsolute()) return id;
    auto name = id;
    if (QFileInfo(name).suffix().isEmpty()) name += ".svg";
    const auto local = QDir(Directory).filePath(name);
    if (QFileInfo::exists(local)) return local;
    const auto legacy = QCoreApplication::applicationDirPath() + "/icons/" + name;
    if (QFileInfo::exists(legacy)) return legacy;
    const auto builtIn = QStringLiteral(":/icons/") + name;
    if (QFileInfo::exists(builtIn)) return builtIn;
    if (name == "notepad.svg") return QStringLiteral(":/icons/page.svg");
    return {};
}
QImage IconBackend::load(const QString& id, int pixelSize, const QColor& tint) {
    const auto path = resolve(id);
    const int size = std::clamp(pixelSize, 8, 512);
    const auto stamp = path.startsWith(":/") ? 0 : QFileInfo(path).lastModified().toMSecsSinceEpoch();
    const auto key = path + "|" + QString::number(stamp) + "|" + QString::number(size) + "|" + tint.name(QColor::HexArgb);
    {
        QMutexLocker lock(&Data->Mutex);
        if (const auto* image = Data->Images.object(key)) return *image;
    }
    QImage result(size, size, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    if (path.endsWith(".svg", Qt::CaseInsensitive)) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            auto svg = file.readAll();
            svg.replace("currentColor", tint.name(QColor::HexRgb).toUtf8());
            QSvgRenderer renderer(svg);
            if (renderer.isValid()) { QPainter painter(&result); renderer.render(&painter); }
        }
    } else if (!path.isEmpty()) {
        QImage original(path);
        if (!original.isNull()) {
            auto scaled = original.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QPainter painter(&result);
            painter.drawImage(QPoint((size - scaled.width()) / 2, (size - scaled.height()) / 2), scaled);
        }
    } else if (!id.isEmpty()) {
        // A missing icon remains recognizable, without repaint-time filesystem lookups.
        QPainter painter(&result);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(tint, std::max(1.0, size / 16.0)));
        painter.drawRoundedRect(QRectF(size * .2, size * .2, size * .6, size * .6), size * .1, size * .1);
        painter.drawLine(QPointF(size * .35, size * .5), QPointF(size * .65, size * .5));
    }
    const int cost = int((result.sizeInBytes() + 1023) / 1024);
    { QMutexLocker lock(&Data->Mutex); Data->Images.insert(key, new QImage(result), cost); }
    return result;
}

class IconResponse final : public QQuickImageResponse {
public:
    std::atomic<bool> Cancelled{false};
    QImage Result;
    QQuickTextureFactory* textureFactory() const override { return QQuickTextureFactory::textureFactoryForImage(Result); }
    void cancel() override { Cancelled.store(true); }
};
class IconProvider final : public QQuickAsyncImageProvider {
public:
    explicit IconProvider(std::shared_ptr<IconBackend> backend) : Backend(std::move(backend)) {}
    ~IconProvider() override { Backend->Pool.waitForDone(); }
    QQuickImageResponse* requestImageResponse(const QString& id, const QSize& requestedSize) override {
        auto* response = new IconResponse;
        const auto separator = id.indexOf('?');
        const auto name = QUrl::fromPercentEncoding((separator < 0 ? id : id.left(separator)).toUtf8());
        const auto query = QUrlQuery(separator < 0 ? QString() : id.mid(separator + 1));
        QColor color(query.queryItemValue("color", QUrl::FullyDecoded));
        if (!color.isValid()) color = QColor("#36465b");
        auto* backend = Backend.get();
        const int size = requestedSize.width() > 0 ? requestedSize.width() : 32;
        backend->Pool.start([backend, response, name, color, size] {
            QImage result;
            if (!response->Cancelled.load()) result = backend->load(name, size, color);
            // Finish on the response's owning thread. The engine may delete it as soon as
            // finished is delivered, so the worker must never emit while it is being torn down.
            QMetaObject::invokeMethod(response, [response, result = std::move(result)] {
                response->Result = result;
                emit response->finished();
            }, Qt::QueuedConnection);
        });
        return response;
    }
private:
    std::shared_ptr<IconBackend> Backend;
};

IconService::IconService(QString directory, QObject* parent) : QObject(parent) {
    InitializeIconResources();
    Directory = directory.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/icons" : std::move(directory);
    Backend = std::make_shared<IconBackend>(Directory);
}
IconService::~IconService() { Backend->Pool.waitForDone(); }
QStringList IconService::searchDirectories() const {
    QStringList roots{Directory, QCoreApplication::applicationDirPath() + "/icons", ":/icons"};
    roots.removeDuplicates();
    return roots;
}
QString IconService::request(const QString& id, int pixelSize, const QColor& tint) {
    const auto key = id + "|" + QString::number(pixelSize) + "|" + tint.name(QColor::HexArgb);
    if (Ready.contains(key) || Pending.contains(key)) return key;
    Pending.insert(key);
    const auto generation = Generation;
    auto* backend = Backend.get();
    backend->Pool.start([this, backend, key, id, pixelSize, tint, generation] {
        auto image = backend->load(id, pixelSize, tint);
        QMetaObject::invokeMethod(this, [this, key, image, generation] {
            if (generation != Generation) return;
            Pending.remove(key);
            Ready.insert(key, new QImage(image), int((image.sizeInBytes() + 1023) / 1024));
            emit imageReady(key, image);
        }, Qt::QueuedConnection);
    });
    return key;
}
QImage IconService::cached(const QString& key) const {
    const auto* image = Ready.object(key);
    return image ? *image : QImage();
}
QQuickAsyncImageProvider* IconService::createProvider() { return new IconProvider(Backend); }
void IconService::invalidate() {
    ++Generation; Pending.clear(); Ready.clear(); Backend->clear(); emit invalidated();
}
}
