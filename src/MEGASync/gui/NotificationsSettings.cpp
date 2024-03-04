#include "NotificationsSettings.h"
#include "ui_NotificationsSettings.h"
#include "SwitchButton.h"

#include "Preferences/Preferences.h"

#include <QCheckBox>

const char* NotificationsSettings::NOTIFICATION_TYPE = "NotificationType";

NotificationsSettings::NotificationsSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationsSettings)
{
    ui->setupUi(this);

    initCheckBoxesState();

    //Everytime a checkbox changes its state, this class iterates over all the checkboxes to check if the
    //general switch is checked or unchecked. This may be improved keeping the checkboxes values and avoid
    //iterating every time they change, but as there are 8-9 of them, it is not worth it, as the code
    //gets more complicated
    //In addition, the first time the general switch state is checked could be done in the creation loop,
    //but as the action is very quick, and for debugging purposes, the iteration is donde again in the same
    //function called when a checkbox state is changed
}

NotificationsSettings::~NotificationsSettings()
{
    delete ui;
}

bool NotificationsSettings::event(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    return QWidget::event(event);
}

void NotificationsSettings::onGeneralSwitchToggled(bool state)
{
   changePreferencesFromCheckboxState(ui->GeneralSwitch, ui->GeneralSwitch->isChecked());
   setCheckBoxesEnable(state);
}

void NotificationsSettings::setCheckBoxesEnable(bool state)
{
    //Act over the QCheckboxes instead of the groupboxes (lower in number, would be more efficient)
    //because MacOS has not QGroupBoxes
    auto disableAction = [&state](QCheckBox* checkBox){
        checkBox->setEnabled(state);
    };

    performActionOnCheckBoxes(disableAction);
}

void NotificationsSettings::connectAllCheckBoxesToSlot()
{
    auto connectionAction = [this](QCheckBox* checkBox){
        connect(checkBox,&QCheckBox::toggled, this, &NotificationsSettings::onNotificationToggled);
    };

    performActionOnCheckBoxes(connectionAction);
}

void NotificationsSettings::initCheckBoxesState()
{
    //General Switch init (connection is not done yet)
    auto generalSwitchValue = Preferences::instance()->isGeneralSwitchNotificationsOn();
    ui->GeneralSwitch->setChecked(generalSwitchValue);
    connect(ui->GeneralSwitch, &SwitchButton::toggled, this, &NotificationsSettings::onGeneralSwitchToggled);

    setCheckBoxesEnable(generalSwitchValue);

    auto stateAction = [](QCheckBox* checkBox){
       //Signals are not blocked here as the connection is not done yet
       auto NotificationType = checkBox->property(NOTIFICATION_TYPE).value<Preferences::NotificationsTypes>();
       checkBox->setChecked(Preferences::instance()->isNotificationEnabled(NotificationType,false));
    };

    performActionOnCheckBoxes(stateAction);
    connectAllCheckBoxesToSlot();
}

void NotificationsSettings::changePreferencesFromCheckboxState(QWidget* selector, bool state)
{
    Preferences::instance()->enableNotifications(selector->property(NOTIFICATION_TYPE).value<Preferences::NotificationsTypes>(), state);
}

void NotificationsSettings::onNotificationToggled()
{
    auto checkBox = dynamic_cast<QCheckBox*>(sender());
    if(checkBox)
    {
        changePreferencesFromCheckboxState(checkBox, checkBox->isChecked());
    }
}

void NotificationsSettings::performActionOnCheckBoxes(std::function<void (QCheckBox *)> f)
{
    auto checkBoxes = ui->NotificationSettingCheckboxes->findChildren<QCheckBox*>();
    foreach(auto& checkBox, checkBoxes)
    {
       f(checkBox);
    }
}



