#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "Preferences/Preferences.h"
#include "ThemeStylesheetParser.h"

#include <QObject>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager* instance();
    QStringList themesAvailable() const;
    Preferences::ThemeType getSelectedTheme() const;
    void setTheme(Preferences::ThemeType theme);
    bool addClassToThemeManager(QWidget* classWidget);
    void applyStyleSheet(QWidget* parent);

signals:
    void themeChanged(Preferences::ThemeType theme);

private:
    ThemeManager();
    void loadStylesheets();
    QStringList getUiCssFiles() const;
    QString getCssPath() const;
    QString getFilePath(const QString& themeFilePath) const;
    QString themeToString(Preferences::ThemeType theme) const;
    void loadStylesheetAsync(const QString& filename, const QString& key);
    void onStylesheetLoaded(const QString& stylesheet, const QString& key);
    void parseStyleSheet(const QString& stylesheet, const QString& uiFileName);

    bool registerWidgetForTheming(QWidget* widget);
    void addToStyleCache(QObject* item, QList<QObject*>& list);

    QSet<QString> getObjectNamesInUICSSFile(const QString& themeKey) const;
    QString getThemeStylesheet(const QString& key) const;

    Preferences::ThemeType mCurrentTheme;
    ThemeStylesheetParser mThemeStylesheetParser;
    QList<QObject*> mWidgetsStyledByCSSFile;
    static const QMap<Preferences::ThemeType, QString> mThemesMap;
    static const QMap<Preferences::ThemeType, QString> mThemePaths;
};

#endif // THEMEMANAGER_H
