#include "SwitchButton.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QVariant>

SwitchButton::SwitchButton(QWidget* parent)
    : QWidget(parent)
{
    auto generalSwitch = new QCheckBox(this);
    generalSwitch->setObjectName(QLatin1String("switch"));
    connect(generalSwitch, &QCheckBox::toggled, this, &SwitchButton::onSwitchToggled);
    mSwitchButton = generalSwitch;

    mSwitchButton->setProperty("showText", false);

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
    if (mSwitchButton->property("showText").toBool())
    {
        mSwitchButton->setProperty("text", isChecked() ? tr("On") : tr("Off"));
    }

    QWidget::paintEvent(event);
}

void SwitchButton::onSwitchToggled(bool state)
{
    emit toggled(state);
}
