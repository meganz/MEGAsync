#include "UpgradeDialog.h"
#include "Utilities.h"
#include "Preferences.h"
#include "ui_UpgradeDialog.h"
#include "gui/PlanWidget.h"
#include <QDebug>
#include <QDateTime>

UpgradeDialog::UpgradeDialog(mega::MegaApi *api, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpgradeDialog)
{
    ui->setupUi(this);
    this->megaApi = api;
    finishTime = 0;

    ui->lDescRecommendation->setTextFormat(Qt::RichText);
    ui->lDescRecommendation->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    ui->lDescRecommendation->setOpenExternalLinks(true);
    refreshAccountDetails();

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);

    //Add PLAN widgets layout->addWidget()
    ui->wPlans->setLayout(layout);

    timer = new QTimer();
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(unitTimeElapsed()));
}

void UpgradeDialog::setTimestamp(long long time)
{
    finishTime = time;
    unitTimeElapsed();
    if (!timer->isActive())
    {
        timer->start(1000);
    }
}

void UpgradeDialog::refreshAccountDetails()
{
    Preferences *preferences = Preferences::instance();
    ui->lDescRecommendation->setText(tr("You have utilized %1 of data transfer in the last 6 hours which took you over our current limit."
                                        " To circumvent this limit, you can [A]upgrade to Pro[/A], "
                                        "which will give you your own bandwidth package and also ample extra storage space.")
                                        .arg(Utilities::getSizeString(preferences->totalBandwidth()))
                                        .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<a href=\"mega://#pro\"><span style=\"color:#d90007; text-decoration:none;\">"))
                                        .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span></a>")));

}

UpgradeDialog::~UpgradeDialog()
{
    delete ui;
}

void UpgradeDialog::unitTimeElapsed()
{
    long long remainingTime = finishTime - QDateTime::currentMSecsSinceEpoch() / 1000;
    if (remainingTime > 0)
    {
        ui->lRemainingTime->setText(tr("Please upgrade to Pro to continue immediately, or wait %1 to continue for free.").arg(Utilities::getTimeString(remainingTime)));
        qDebug() << finishTime << " " << (QDateTime::currentMSecsSinceEpoch() / 1000) << " "
                 << remainingTime << " " << Utilities::getTimeString(remainingTime);
    }
    else
    {
        //Timer reaches 0
    }
}
