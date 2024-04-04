#ifndef THEMESTYLESHEETPARSER_H
#define THEMESTYLESHEETPARSER_H

#include <QIcon>
#include <QMap>
#include <QString>

class ObjectSelector
{
public:
    QString mObjectName;
    QString mSelector;
    QString mThemeKey;

    ObjectSelector(const QString& name, const QString& sel, const QString& themeKey)
        : mObjectName(name), mSelector(sel), mThemeKey(themeKey) {}

    bool operator<(const ObjectSelector& other) const
    {
        return std::tie(mObjectName, mSelector, mThemeKey) < std::tie(other.mObjectName, other.mSelector, other.mThemeKey);
    }
};

class ThemeStylesheetParser
{
public:
    struct IconStates
    {
        QIcon normal;
        QIcon hover;
        QIcon pressed;
    };

    ThemeStylesheetParser();
    void parseStyleSheet(const QString& stylesheet, const QString& uiFileName);
    QString getThemeStylesheet(const QString &key) const;
    QSet<QString> getObjectNamesInUICSSFile(const QString& themeKey) const;
    QMap<QString, IconStates> getIconsForObject(const QString& objectName, const QString &themeKey) const;

private:
    QString determineState(const QString& selector);

    QMap<ObjectSelector, IconStates> mObjectSelectorMap;
    QMap<QString, QSet<QString>> mObjectNameToUiCSSFileNameMap;
    QMultiMap<QString, QString> mObjectNameToUiCSSFileStyleSheetsMap;

    static const QString TOKEN_HOVER;
    static const QString TOKEN_PRESSED;
    static const QString TOKEN_NORMAL;
};

#endif // THEMESTYLESHEETPARSER_H
