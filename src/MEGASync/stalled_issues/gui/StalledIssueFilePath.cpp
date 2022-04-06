#include "StalledIssueFilePath.h"
#include "ui_StalledIssueFilePath.h"

#include "Utilities.h"

StalledIssueFilePath::StalledIssueFilePath(QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    ui(new Ui::StalledIssueFilePath)
{
    ui->setupUi(this);

    ui->lines->installEventFilter(this);
}

StalledIssueFilePath::~StalledIssueFilePath()
{
    delete ui;
}

void StalledIssueFilePath::refreshUi()
{
    auto data = getData();

    if(data->mIsCloud)
    {
        auto remoteIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/cloud_upload_item_ico.png"));
        ui->LocalOrRemoteIcon->setPixmap(remoteIcon.pixmap(ui->LocalOrRemoteIcon->size()));

        ui->LocalOrRemoteText->setText(tr("on MEGA:"));
    }
    else
    {
        auto localIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/StalledIssues/PC_ico_rest.png"));
        ui->LocalOrRemoteIcon->setPixmap(localIcon.pixmap(ui->LocalOrRemoteIcon->size()));

        ui->LocalOrRemoteText->setText(tr("Local:"));
    }

    ui->filePath->setText(data->mIndexPath);

    QIcon fileTypeIcon;
    auto splittedFile = data->mFileName.split(QString::fromUtf8("."));
    if(splittedFile.size() != 1)
    {
        fileTypeIcon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                                           data->mFileName, QLatin1Literal(":/images/drag_")));
    }
    else
    {
        fileTypeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/images/color_folder.png"));
    }

    ui->filePathIcon->setPixmap(fileTypeIcon.pixmap(ui->filePathIcon->size()));
}

bool StalledIssueFilePath::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->lines && event->type() == QEvent::Paint)
    {

    }

    return StalledIssueBaseDelegateWidget::eventFilter(watched, event);
}
