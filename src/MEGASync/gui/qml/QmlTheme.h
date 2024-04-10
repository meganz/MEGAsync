#ifndef QMLTHEME_H
#define QMLTHEME_H

#include <QObject>
#include "Preferences/Preferences.h"

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
    static QMap<Preferences::Theme, QString> mThemesQmlMap;

    void onThemeChanged(Preferences::Theme theme);
};

#endif // QMLTHEME_H
