#include "StalledIssueChooseTitle.h"

#include "TokenizableItems/TokenPropertyNames.h"
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
                                              Utilities::AttributeType::SMALL |
                                                  Utilities::AttributeType::THIN |
                                                  Utilities::AttributeType::OUTLINE);
    }
    else
    {
        pixmapName = Utilities::getPixmapName(QLatin1String("monitor"),
                                              Utilities::AttributeType::SMALL |
                                                  Utilities::AttributeType::THIN |
                                                  Utilities::AttributeType::OUTLINE);
    }

    ui->icon->setProperty(TOKEN_PROPERTIES::normalOff,
                          isFailed() ? QLatin1String("icon-inverse-accent") :
                                       QLatin1String("icon-primary"));
    ui->icon->setIcon(QIcon(pixmapName));
    ui->icon->show();
}
