#ifndef THEMESTYLESHEETPARSER_H
#define THEMESTYLESHEETPARSER_H

#include <QMap>
#include <QString>

class ThemeStylesheetParser
{
public:
    ThemeStylesheetParser() = default;
    void parseStyleSheet(const QString& stylesheet, const QString& uiFileName);
    QString getThemeStylesheet(const QString &key) const;
    QSet<QString> getObjectNamesInCSSFile(const QString& widgetThemeKey) const;

private:
    QMap<QString, QSet<QString>> mObjectNameToCSSFileNameMap;
    QMultiMap<QString, QString> mObjectNameToCSSFileStyleSheetsMap;
};

#endif // THEMESTYLESHEETPARSER_H
