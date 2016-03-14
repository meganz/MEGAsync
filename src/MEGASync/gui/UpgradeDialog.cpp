#include "UpgradeDialog.h"
#include "Utilities.h"
#include "Preferences.h"
#include "ui_UpgradeDialog.h"
#include "gui/PlanWidget.h"
#include <QDebug>

UpgradeDialog::UpgradeDialog(mega::MegaApi *api, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpgradeDialog)
{
    ui->setupUi(this);
    this->megaApi = api;
    remainingTime = 0;

    Preferences *preferences = Preferences::instance();
    ui->lDescRecommendation->setText(tr("You have utilized %1 of data transfer in the last 6 hours which took you over our current limit."
                                        " To circumvent this limit, you can [A]upgrade to Pro[/A], "
                                        "which will give you your own bandwidth package and also ample extra storage space.")
                                        .arg(preferences->totalBandwidth())
                                        .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style=\" color:#d90007;\">"))
                                        .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>")));

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    //Add PLAN widgets layout->addWidget()
    ui->wPlans->setLayout(layout);

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
        ui->lRemainingTime->setText(tr("Please upgrade to Pro to continue immediately, or wait %1 to continue for free.").arg(Utilities::getTimeString(remainingTime)));
        qDebug() << Utilities::getTimeString(remainingTime);
    }
    else
    {
        //Timer reaches 0
    }

}
