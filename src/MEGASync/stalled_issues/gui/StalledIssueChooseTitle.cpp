#include "StalledIssueChooseTitle.h"

#include "ui_StalledIssueActionTitle.h"
#include "Utilities.h"

StalledIssueChooseTitle::StalledIssueChooseTitle(QWidget* parent):
    StalledIssueActionTitle(parent)
{
}

void StalledIssueChooseTitle::showIcon()
{
    QString pixmapName;

    if(mIsCloud)
    {
        pixmapName = Utilities::getPixmapName(QLatin1String("MEGA"),
                                              isFailed() ? Utilities::AttributeType::INVERSE :
                                                           Utilities::AttributeType::NONE);
    }
    else
    {
        pixmapName = Utilities::getPixmapName(QLatin1String("monitor"),
                                              isFailed() ? Utilities::AttributeType::INVERSE :
                                                           Utilities::AttributeType::NONE);
    }

    ui->icon->setIcon(QIcon(pixmapName));
    ui->icon->show();
}
