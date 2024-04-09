#ifndef QMLTHEME_H
#define QMLTHEME_H

#include <QObject>

class QmlTheme : public QObject
{
    Q_OBJECT
    Q_PROPERTY (QString theme READ getTheme NOTIFY themeChanged)

public:
    explicit QmlTheme(QObject *parent = nullptr);
    QString getTheme() const;
    Q_INVOKABLE QStringList getThemes() const;

signals:
    void themeChanged(QString theme);
};

#endif // QMLTHEME_H
