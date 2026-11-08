#include "TranslationManager.h"
#include <QCoreApplication>
#include <QDebug>

static void InitializeTranslations() { Q_INIT_RESOURCE(translations); }

namespace gpm {
QString ResolveLanguage(const QString& preference, const QStringList& systemLanguages) {
    QString tag = preference;
    if (tag.isEmpty() || tag == "system")
        tag = systemLanguages.isEmpty() ? QStringLiteral("en") : systemLanguages.front();
    const QLocale locale(tag);
    if (locale.language() != QLocale::Chinese) return QStringLiteral("en");
    return locale.script() == QLocale::TraditionalHanScript
        ? QStringLiteral("zh_TW") : QStringLiteral("zh_CN");
}
TranslationManager::TranslationManager() { InitializeTranslations(); }
QString InterfaceFontFamily(const QString& preference) {
    const auto language = ResolveLanguage(preference);
    const bool chinese = language.startsWith("zh_");
#ifdef Q_OS_MACOS
    // macOS ships PingFang for Chinese and Helvetica Neue for Latin UI text.
    if (!chinese) return QStringLiteral("Helvetica Neue");
    return language == "zh_TW" ? QStringLiteral("PingFang TC") : QStringLiteral("PingFang SC");
#else
    return chinese ? QStringLiteral("Microsoft JhengHei UI") : QStringLiteral("Segoe UI");
#endif
}
bool TranslationManager::load(const QString& preference) {
    QCoreApplication::removeTranslator(&Translator);
    const auto language = ResolveLanguage(preference);
    if (language == "en") return true;
    if (!Translator.load(":/i18n/gopiemenu_" + language + ".qm")) {
        qWarning() << "Could not load translation:" << language;
        return false;
    }
    return QCoreApplication::installTranslator(&Translator);
}
}
