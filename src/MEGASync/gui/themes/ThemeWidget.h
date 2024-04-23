#ifndef THEMEWIDGET_H
#define THEMEWIDGET_H

#include "Preferences/Preferences.h"
#include "ThemeStylesheetParser.h"

#include <QObject>

class ThemeWidget : public QObject
{
    Q_OBJECT

public:
    explicit ThemeWidget(QObject *parent = nullptr);

    bool registerWidgetForTheming(QWidget* widget);
    void applyStyleSheet(QWidget* widget);

private:
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

    static const QMap<Preferences::ThemeType, QString> mThemePaths;
    ThemeStylesheetParser mThemeStylesheetParser;
    QList<QObject*> mWidgetsStyledByCSSFile;

private slots:
    void onThemeChanged(Preferences::ThemeType theme);

};

#endif // THEMEWIDGET_H
