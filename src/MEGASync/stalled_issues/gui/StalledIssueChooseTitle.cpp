#include "StalledIssueChooseTitle.h"
#include "ui_StalledIssueActionTitle.h"

#include "Utilities.h"

StalledIssueChooseTitle::StalledIssueChooseTitle(QWidget *parent)
    : StalledIssueActionTitle(parent)
{
}

void StalledIssueChooseTitle::showIcon()
{
    QIcon icon;

    if(mIsCloud)
    {
        icon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/cloud_default.png"));
    }
    else
    {
        icon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/monitor_default.png"));
    }

    ui->icon->setPixmap(icon.pixmap(QSize(16,16)));
    ui->icon->show();
}
