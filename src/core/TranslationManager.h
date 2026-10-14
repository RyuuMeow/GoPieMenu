#pragma once
#include <QLocale>
#include <QStringList>
#include <QTranslator>

namespace gpm {
// Resolve the preferred UI language, falling back to the English source text.
QString ResolveLanguage(const QString& preference,
                        const QStringList& systemLanguages = QLocale::system().uiLanguages());
QString InterfaceFontFamily(const QString& preference);

class TranslationManager {
public:
    TranslationManager();
    bool load(const QString& preference);
private:
    QTranslator Translator;
};
}
