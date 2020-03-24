#include "LockedPopOver.h"
#include "ui_LockedPopOver.h"

#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTimer>
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

using namespace mega;

LockedPopOver::LockedPopOver(QDialog *parent) :
    ui(new Ui::LockedPopOver)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
}


void LockedPopOver::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void LockedPopOver::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InCubic);
    anim->setDuration(300);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
