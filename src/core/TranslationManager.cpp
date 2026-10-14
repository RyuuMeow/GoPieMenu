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
    return ResolveLanguage(preference).startsWith("zh_")
        ? QStringLiteral("Microsoft JhengHei UI") : QStringLiteral("Segoe UI");
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
