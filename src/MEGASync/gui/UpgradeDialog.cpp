#include "UpgradeDialog.h"
#include "Utilities.h"
#include "ui_UpgradeDialog.h"
#include "gui/PlanWidget.h"
#include <QDebug>

UpgradeDialog::UpgradeDialog(mega::MegaApi *api, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpgradeDialog)
{
    ui->setupUi(this);

    ui->lDescRecommendation->text().replace(QString::fromUtf8("[A]"), QString::fromUtf8("<font color=\"#d90007\"> "))
                             .replace(QString::fromUtf8("[/A]"), QString::fromUtf8(" </font>"));
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    //Add PLAN widgets layout->addWidget()
    ui->wPlans->setLayout(layout);

    remainingTime = 0;
    timer = new QTimer();
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(unitTimeElapsed()));

}

void UpgradeDialog::setTimeStamp(long long time)
{
    remainingTime = time;
    if (!timer->isActive())
    {
        timer->start(1000);
    }
}

UpgradeDialog::~UpgradeDialog()
{
    delete ui;
}

void UpgradeDialog::unitTimeElapsed()
{
    if (remainingTime - 1000 >= 0)
    {
        remainingTime -= 1000;
        ui->lRemainingTime->setText(Utilities::getTimeString(remainingTime));
        qDebug() << Utilities::getTimeString(remainingTime);
    }
    else
    {
        //Timer reaches 0
    }

}
