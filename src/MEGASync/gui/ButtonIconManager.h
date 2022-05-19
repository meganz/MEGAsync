#ifndef BUTTONICONMANAGER_H
#define BUTTONICONMANAGER_H

#include <QObject>
#include <QAbstractButton>
#include <QEvent>

class ButtonIconManager : public QObject
{
    Q_OBJECT
    static const char*   ICON_PREFIX;
    static const char*   HOVER_SELECTED_FLAG;
    static const char*   CHECK_STATE;
    static const char*   IGNORE_BUTTON;

    struct IconInfo
    {
        QString extension;
        QString iconName;
        QString iconPath;

        bool isEmpty(){return iconName.isEmpty();}
    };

public:
    struct Settings
    {
        QString hover_suffix;
        QString selected_suffix;
        QString hover_selected_suffix;
        QString default_suffix;
        double  opacityGap;

        Settings():
        hover_suffix(QString::fromLatin1("_hover")),
        selected_suffix(QString::fromLatin1("_selected")),
        hover_selected_suffix(QString::fromLatin1("_hover_selected")),
        default_suffix(QString::fromLatin1("_default")),
        opacityGap(0.5){}
    };

    explicit ButtonIconManager(QObject * parent = nullptr);
    void addButton(QAbstractButton* button);
    void setSettings(const Settings& settings);

protected:
    virtual bool eventFilter(QObject * watched, QEvent * event) override;

private slots:
    void onButtonChecked();

private:
    void fillIcon(const IconInfo& info, QIcon& icon);
    void changeButtonTextColor(QAbstractButton* button, double alpha);
    IconInfo splitIconPath(const QUrl& iconPath);
    bool cleanIconName(IconInfo& info, const QString& separator);

    void setDefaultIcon(QAbstractButton* button);
    void setHoverIcon(QAbstractButton* button);
    void setSelectedIcon(QAbstractButton* button);

    Settings mSettings;
};


#endif // BUTTONICONMANAGER_H
