#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager* instance();
    QStringList themesAvailable() const;
    QString getSelectedTheme() const;
    void setTheme(QString theme);

signals:
    void themeChanged(QString theme);

private:
    QString mCurrentTheme;

    ThemeManager();
};

#endif // THEMEMANAGER_H
