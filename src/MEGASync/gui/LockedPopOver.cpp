#include "LockedPopOver.h"
#include "ui_LockedPopOver.h"

#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTimer>
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

using namespace mega;

LockedPopOver::LockedPopOver(QDialog *) :
    ui(new Ui::LockedPopOver)
{
    ui->setupUi(this);
    tweakStrings();
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
}


void LockedPopOver::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        tweakStrings();
    }
    QWidget::changeEvent(event);
}

void LockedPopOver::tweakStrings()
{
    ui->lDescLock->setText(ui->lDescLock->text()
                                 .replace(QString::fromUtf8("[A]"),
                                          QString::fromUtf8("<p style=\"line-height: 20px;\">"))
                                 .replace(QString::fromUtf8("[/A]"),
                                          QString::fromUtf8("</p>")));

    ui->lPassLeaked->setText(ui->lPassLeaked->text()
                                 .replace(QString::fromUtf8("[A]"),
                                          QString::fromUtf8("<p style=\"line-height: 20px;\">"))
                                 .replace(QString::fromUtf8("[/A]"),
                                          QString::fromUtf8("</p>")));
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
