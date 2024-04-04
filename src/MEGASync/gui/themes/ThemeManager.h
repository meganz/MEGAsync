#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include "Preferences/Preferences.h"
#include "ThemeStylesheetParser.h"

#include <QColor>
#include <QMap>
#include <QString>
#include <QWidget>
#include <QApplication>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager* instance();

    QStringList themesAvailable() const;

    void applyApplicationStyles(QObject* item);

    bool addClassToThemeManager(QWidget* classWidget);

    Preferences::Theme getSelectedTheme();

    void setTheme(Preferences::Theme newTheme);

    void applyStyleSheet(QWidget* parent);

private:

    ThemeManager();

    QString getCssPath() const;

    QStringList getUiCssFiles() const;

    bool registerWidgetForTheming(QWidget* widget);

    void addToStyleCache(QObject* item, QList<QObject*>& list);

    void loadStylesheets();

    void loadStylesheetAsync(const QString& filename, const QString& key);

    void onStylesheetLoaded(const QString& stylesheet, const QString& key);

    QString getFilePath(const QString& themeFilePath) const;

    QString getThemeStylesheet(const QString& key) const;

    QString themeToString(Preferences::Theme theme) const;

    void parseStyleSheet(const QString& stylesheet, const QString& uiFileName);

    QSet<QString> getObjectNamesInUICSSFile(const QString& themeKey) const;

    QMap<QString, ThemeStylesheetParser::IconStates> getIconsForObject(const QString& objectName, const QString &key) const;

    void registerIconForTheming(QWidget* item, const QString& key);

signals:
    void themeChanged(Preferences::Theme newTheme);

private:
    ThemeStylesheetParser mThemeStylesheetParser;
    Preferences::Theme mCurrentStyle;
    QList<QObject*> mWidgetsStyledByCSSFile;
    QList<QObject*> mWidgetsStyledByCode;
    const QMap<Preferences::Theme, QString> mThemePaths;
    static QMap<Preferences::Theme, QString> mThemesMap;
};

#endif // THEMEMANAGER_H
