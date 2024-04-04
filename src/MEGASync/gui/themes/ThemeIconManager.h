#ifndef THEMEICONMANAGER_H
#define THEMEICONMANAGER_H

#include <QEvent>
#include <QIcon>
#include <QMap>
#include <QObject>
#include <QWidget>

class ThemeIconManager : public QObject
{
    Q_OBJECT

public:
    struct IconStateInfo
    {
        QIcon normal;
        QIcon hover;
        QIcon pressed;

        bool isNull() const
        {
            return normal.availableSizes().isEmpty() &&
                   hover.availableSizes().isEmpty() &&
                   pressed.availableSizes().isEmpty();
        }
    };

    static ThemeIconManager& instance();
    void registerWidget(QWidget* widget, const IconStateInfo& icons);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    ThemeIconManager() = default;
    ThemeIconManager(const ThemeIconManager&) = delete;
    ThemeIconManager& operator=(const ThemeIconManager&) = delete;

    QMap<QWidget*, IconStateInfo> iconPaths;
};

#endif // THEMEICONMANAGER_H
