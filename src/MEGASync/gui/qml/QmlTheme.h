#ifndef QMLTHEME_H
#define QMLTHEME_H

#include "Preferences/Preferences.h"

#include <QObject>

class QmlTheme : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString theme READ getTheme NOTIFY themeChanged)

public:
    explicit QmlTheme(QObject *parent = nullptr);
    QString getTheme() const;

signals:
    void themeChanged(QString theme);

private:
    static QMap<Preferences::ThemeType, QString> mThemesQmlMap;

    void onThemeChanged();
};

#endif // QMLTHEME_H
