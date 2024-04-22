#ifndef THEMESTYLESHEETPARSER_H
#define THEMESTYLESHEETPARSER_H

#include <QMap>
#include <QString>

class ThemeStylesheetParser
{
public:
    ThemeStylesheetParser();
    void parseStyleSheet(const QString& stylesheet, const QString& uiFileName);
    QString getThemeStylesheet(const QString &key) const;
    QSet<QString> getObjectNamesInUICSSFile(const QString& themeKey) const;

private:
    QMap<QString, QSet<QString>> mObjectNameToUiCSSFileNameMap;
    QMultiMap<QString, QString> mObjectNameToUiCSSFileStyleSheetsMap;
};

#endif // THEMESTYLESHEETPARSER_H
