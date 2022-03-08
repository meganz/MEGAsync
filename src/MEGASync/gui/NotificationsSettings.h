#ifndef NOTIFICATIONSSETTINGS_H
#define NOTIFICATIONSSETTINGS_H

#include <QWidget>
#include <QCheckBox>

#include <functional>

namespace Ui {
class NotificationsSettings;
}

class NotificationsSettingsCategory;

class NotificationsSettings : public QWidget
{
    Q_OBJECT

    static const char* NOTIFICATION_TYPE;

public:
    explicit NotificationsSettings(QWidget *parent = nullptr);
    ~NotificationsSettings();

protected:
    bool event(QEvent *event) override;

private slots:
    void onNotificationToggled();

private:
    Ui::NotificationsSettings *ui;
    QList<NotificationsSettingsCategory*> mNotificationsCategories;

    void performActionOnCheckBoxes(std::function<void(QCheckBox* cb)> f);
    void onGeneralSwitchToggled(bool state);
    void connectAllCheckBoxesToSlot();
    void initCheckBoxesState();
    void setCheckBoxesEnable(bool state);

    void changePreferencesFromCheckboxState(QWidget* selector, bool state);
};

#endif // NOTIFICATIONSSETTINGS_H
