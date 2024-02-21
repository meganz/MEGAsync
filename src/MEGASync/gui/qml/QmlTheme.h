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
    Q_INVOKABLE void setTheme(const QString& theme);
    Q_INVOKABLE QStringList getThemes() const;

signals:
    void themeChanged(QString theme);

private:
    QString mTheme;

    void startDemoChange();
};

#endif // QMLTHEME_H
