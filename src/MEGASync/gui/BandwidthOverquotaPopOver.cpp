#include "BandwidthOverquotaPopOver.h"
#include "ui_LockedPopOver.h"

#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTimer>
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

using namespace mega;

BandwidthOverquotaPopOver::BandwidthOverquotaPopOver(QDialog *parent) :
    ui(new Ui::BandwidthOverquotaPopOver)
{
    ui->setupUi(this);
    tweakStrings();
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
}

void BandwidthOverquotaPopOver::updateMessage(const QString &message)
{
    ui->labelMessage->setText(message);
}


void BandwidthOverquotaPopOver::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        tweakStrings();
    }
    QWidget::changeEvent(event);
}

void BandwidthOverquotaPopOver::tweakStrings()
{
    ui->labelMessage->setText(ui->labelMessage->text()
                                 .replace(QString::fromUtf8("[A]"),
                                          QString::fromUtf8("<p style=\"line-height: 20px;\">"))
                                 .replace(QString::fromUtf8("[/A]"),
                                          QString::fromUtf8("</p>")));
}


void BandwidthOverquotaPopOver::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::InCubic);
    anim->setDuration(300);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
