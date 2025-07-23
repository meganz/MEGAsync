#include "LockedPopOver.h"

#include "RefreshAppChangeEvent.h"
#include "ui_LockedPopOver.h"

#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTimer>

LockedPopOver::LockedPopOver(QDialog *) :
    ui(new Ui::LockedPopOver)
{
    ui->setupUi(this);
    tweakStrings();
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
}

bool LockedPopOver::event(QEvent* event)
{
    if (RefreshAppChangeEvent::isRefreshEvent(event))
    {
        ui->retranslateUi(this);
        tweakStrings();
    }
    return QWidget::event(event);
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
