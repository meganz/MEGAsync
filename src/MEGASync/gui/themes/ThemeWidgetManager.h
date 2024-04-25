#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include "Preferences/Preferences.h"
#include "ThemeStylesheetParser.h"

#include <QObject>

class ThemeWidgetManager : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<ThemeWidgetManager> instance();

    bool registerWidgetForTheming(QWidget* widget);
    void applyStyleSheet(QWidget* widget);

private:
    explicit ThemeWidgetManager(QObject *parent = nullptr);

    void loadStylesheets();
    QStringList getCSSFiles() const;
    QString getCSSPath() const;
    QString getFilePath(const QString& themeFilePath) const;
    QString themeToString(Preferences::ThemeType theme) const;
    void loadStylesheetAsync(const QString& filename, const QString& key);
    void parseStyleSheet(const QString& stylesheet, const QString& uiFileName);
    void addToStyleCache(QObject* item);
    QSet<QString> getObjectNamesInCSSFile(const QString& widgetThemeKey) const;
    QString getThemeStylesheet(const QString& key) const;
    void onThemeChanged(Preferences::ThemeType theme);

    static const QMap<Preferences::ThemeType, QString> mThemePaths;
    ThemeStylesheetParser mThemeStylesheetParser;
    QList<QObject*> mWidgetsStyledByCSSFile;
};

#endif // THEMEWIDGET_H
