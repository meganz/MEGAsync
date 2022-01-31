#include "SwitchButton.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QVariant>

#ifdef Q_OS_MACX
#include <QOperatingSystemVersion>
#include "gui/CocoaSwitchButton.h"
#endif

SwitchButton::SwitchButton(QWidget* parent)
    : QWidget(parent)
{
    //Create General Switch depending on system version and type
#ifdef Q_OS_MACX
    auto current = QOperatingSystemVersion::current();
    if (current < QOperatingSystemVersion::MacOSCatalina) //Mac lower versions than Catalina (10.15) use a normal switch
    {
        auto switchControl = new QCheckBox(this);
        connect(switchControl, &QCheckBox::toggled, this, &SwitchButton::onSwitchToggled);
        mSwitchButton = switchControl;

        applyMacStyleSheet();
    }
    else
    {
        auto switchControl = new CocoaSwitchButton(this);
        connect(switchControl, &CocoaSwitchButton::toggled, this, &SwitchButton::onSwitchToggled);
        mSwitchButton = switchControl;
    }

#else
    auto generalSwitch = new QCheckBox(this);
    connect(generalSwitch, &QCheckBox::toggled, this, &SwitchButton::onSwitchToggled);
    mSwitchButton = generalSwitch;
    applyWinLinuxStyleSheet();
#endif

    //Done with code to avoid creating an empty .ui
    auto layout  = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(mSwitchButton);
    setLayout(layout);

}

void SwitchButton::setChecked(bool state)
{
    if(mSwitchButton->property("checked").toBool() != state)
    {
        mSwitchButton->setProperty("checked", state);
        emit toggled(state);
    }
}

bool SwitchButton::isChecked()
{
    return mSwitchButton->property("checked").toBool();
}

void SwitchButton::paintEvent(QPaintEvent* event)
{
    //Only change text is necessary (NEVER in MAC, as it has no text)
#ifndef Q_OS_MACX
    auto text = mSwitchButton->property("text").toString();
    if(text.isEmpty() || ((isChecked() && text == tr("Off"))
                          || (!isChecked() && text == tr("On"))))
    {
        mSwitchButton->setProperty("text", isChecked() ? tr("On") : tr("Off"));
    }
#endif

    QWidget::paintEvent(event);
}

void SwitchButton::onSwitchToggled(bool state)
{
    emit toggled(state);
}

void SwitchButton::applyWinLinuxStyleSheet()
{
    setStyleSheet(QString::fromUtf8("QCheckBox::indicator {width: 40px;height: 19px;}"
    "QCheckBox::indicator:checked {image: url(:/images/Switches/switch_checked_rest.svg);}"
    "QCheckBox::indicator:checked:hover {image: url(:/images/Switches/switch_checked_hover.svg);}"
    "QCheckBox::indicator:checked:pressed {image: url(:/images/Switches/switch_checked_down.svg);}"
    "QCheckBox::indicator:unchecked {image: url(:/images/Switches/switch_unchecked_rest.svg);}"
    "QCheckBox::indicator:unchecked:hover {image: url(:/images/Switches/switch_unchecked_hover.svg);}"
    "QCheckBox::indicator:unchecked:pressed {image: url(:/images/Switches/switch_unchecked_down.svg);}"));
}

void SwitchButton::applyMacStyleSheet()
{
    setStyleSheet(QString::fromUtf8("QCheckBox::indicator {width: 40px;height: 19px;}"
    "QCheckBox::indicator:checked {image: url(:/images/Switches/switch_checked_rest-macx.svg);}"
    "QCheckBox::indicator:checked:pressed {image: url(:/images/Switches/switch_checked_down-macx.svg);}"
    "QCheckBox::indicator:unchecked {image: url(:/images/Switches/switch_unchecked_rest-macx.svg);}"
    "QCheckBox::indicator:unchecked:pressed {image: url(:/images/Switches/switch_unchecked_down-macx.svg);}"));
}
