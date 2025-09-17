#include "LocalAndRemoteChooseWidget.h"

#include "MegaApplication.h"
#include "PlatformStrings.h"
#include "StalledIssuesModel.h"
#include "ui_StalledIssueChooseWidget.h"

LocalAndRemoteStalledIssueBaseChooseWidget::LocalAndRemoteStalledIssueBaseChooseWidget(QWidget *parent) :
    StalledIssueChooseWidget(parent)
{
}

void LocalAndRemoteStalledIssueBaseChooseWidget::updateUi(StalledIssueDataPtr data,
    LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side)
{
    auto fileName = data->getFileName();
    ui->name->setTitle(fileName);

    ui->chooseTitle->showIcon();

    mega::MegaHandle handle(mega::INVALID_HANDLE);
    if(data->isCloud())
    {
        auto cloudData = data->convert<CloudStalledIssueData>();
        if(cloudData)
        {
            handle = cloudData->getPathHandle();
        }
    }

    ui->name->setInfo(data->getNativeFilePath(), handle);
    ui->name->setIsFile(data->isFile());
    ui->name->setIsCloud(data->isCloud());
    ui->name->showIcon();

    ui->path->show();
    ui->path->updateUi(data);

    if(side != LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::NONE)
    {
        ui->chooseTitle->setActionButtonVisibility(StalledIssueChooseWidget::BUTTON_ID, false);

        // In order to keep the old context, we use QApplication::translate even when the strings
        // are only used here QIcon icon;
        if(side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::REMOTE)
        {
            if(data->isCloud())
            {
                ui->chooseTitle->setMessage(
                    chosenString(),
                    Utilities::getPixmapName(QLatin1String("check"),
                                             Utilities::AttributeType::SMALL |
                                                 Utilities::AttributeType::THIN |
                                                 Utilities::AttributeType::OUTLINE),
                    QLatin1String("support-success"));
            }
            else
            {
                ui->chooseTitle->setMessage(
                    solvedString(),
                    Utilities::getPixmapName(QLatin1String("cross"),
                                             Utilities::AttributeType::SMALL |
                                                 Utilities::AttributeType::THIN |
                                                 Utilities::AttributeType::OUTLINE),
                    QLatin1String("support-error"));
            }
        }
        else if(side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::LOCAL)
        {
            if(data->isCloud())
            {
                ui->chooseTitle->setMessage(
                    solvedString(),
                    Utilities::getPixmapName(QLatin1String("cross"),
                                             Utilities::AttributeType::SMALL |
                                                 Utilities::AttributeType::THIN |
                                                 Utilities::AttributeType::OUTLINE),
                    QLatin1String("support-error"));
            }
            else
            {
                ui->chooseTitle->setMessage(
                    QApplication::translate("StalledIssueChooseWidget",
                                            "Local file is being uploaded"),
                    Utilities::getPixmapName(QLatin1String("check"),
                                             Utilities::AttributeType::SMALL |
                                                 Utilities::AttributeType::THIN |
                                                 Utilities::AttributeType::OUTLINE),
                    QLatin1String("support-success"));
            }
        }
        else if(side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::BOTH)
        {
            if(!data->renamedFileName().isEmpty())
            {
                ui->chooseTitle->setMessage(
                    QApplication::translate("NameConflict", "Renamed to \"%1\"")
                        .arg(data->renamedFileName()),
                    Utilities::getPixmapName(QLatin1String("check"),
                                             Utilities::AttributeType::SMALL |
                                                 Utilities::AttributeType::THIN |
                                                 Utilities::AttributeType::OUTLINE),
                    QLatin1String("support-success"));
            }
            else
            {
                ui->chooseTitle->setMessage(QString());
            }
        }
    }
    else
    {
        ui->chooseTitle->setActionButtonVisibility(StalledIssueChooseWidget::BUTTON_ID, true);
    }

    mData = data;
}

const StalledIssueDataPtr &LocalAndRemoteStalledIssueBaseChooseWidget::data()
{
    return mData;
}

//LOCAL
QString LocalStalledIssueChooseWidget::solvedString() const
{
    return PlatformStrings::movedFileToBin();
}

void LocalStalledIssueChooseWidget::updateUi(LocalStalledIssueDataPtr localData,
    LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side)
{
    updateExtraInfo(localData);
    ui->chooseTitle->setTitle(tr("Local Copy"));
    ui->chooseTitle->setIsCloud(false);
    addDefaultButton();

    LocalAndRemoteStalledIssueBaseChooseWidget::updateUi(localData,side);

    if(side != LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::NONE)
    {
        setSolved(true, side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::LOCAL);
    }
}

void LocalStalledIssueChooseWidget::onRawInfoToggled()
{
    updateExtraInfo(mData->convert<LocalStalledIssueData>());
}

void LocalStalledIssueChooseWidget::updateExtraInfo(LocalStalledIssueDataPtr localData)
{
    localData->getFileFolderAttributes()->requestModifiedTime(this, [this](const QDateTime& time){
        ui->name->updateLastTimeModified(time);
    });

#ifndef Q_OS_LINUX
    localData->getFileFolderAttributes()->requestCreatedTime(this, [this](const QDateTime& time){
        ui->name->updateCreatedTime(time);
    });
#endif

    localData->getFileFolderAttributes()->requestSize(this, [this](qint64 size){
        ui->name->updateSize(size);
    });

    if(MegaSyncApp->getStalledIssuesModel()->isRawInfoVisible())
    {
        localData->getFileFolderAttributes()->requestCRC(this, [this](const QString& fp)
            {
                ui->name->updateCRC(fp);
            });
    }
    else
    {
        ui->name->updateCRC(QString());
    }
}

//CLOUD
QString CloudStalledIssueChooseWidget::solvedString() const
{
    return tr("Moved to MEGA Bin");
}

void CloudStalledIssueChooseWidget::updateUi(CloudStalledIssueDataPtr cloudData,
    LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side)
{
    updateExtraInfo(cloudData);
    ui->chooseTitle->setTitle(tr("Remote Copy"));
    ui->chooseTitle->setIsCloud(true);
    addDefaultButton();

    LocalAndRemoteStalledIssueBaseChooseWidget::updateUi(cloudData, side);

    if(side != LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::NONE)
    {
        setSolved(true, side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::REMOTE);
    }
    else
    {
        setSolved(false, false);
    }
}

void CloudStalledIssueChooseWidget::onRawInfoToggled()
{
    updateExtraInfo(mData->convert<CloudStalledIssueData>());
}

void CloudStalledIssueChooseWidget::updateExtraInfo(CloudStalledIssueDataPtr cloudData)
{
    auto node = cloudData->getNode();
    if(node)
    {
        cloudData->getFileFolderAttributes()->requestModifiedTime(this, [this](const QDateTime& time){
            ui->name->updateLastTimeModified(time);
        });

        cloudData->getFileFolderAttributes()->requestCreatedTime(this, [this](const QDateTime& time){
            ui->name->updateCreatedTime(time);
        });

        cloudData->getFileFolderAttributes()->requestSize(this, [this](qint64 size){
            ui->name->updateSize(size);
        });

        if(MegaSyncApp->getStalledIssuesModel()->isRawInfoVisible())
        {
            cloudData->getFileFolderAttributes()->requestCRC(this, [this](const QString& fp)
                {
                    ui->name->updateCRC(fp);
                });
        }
        else
        {
            ui->name->updateCRC(QString());
        }

        cloudData->getFileFolderAttributes()->requestVersions(this, [this](int versions){
            ui->name->updateVersionsCount(versions);
        });

        cloudData->getFileFolderAttributes()->requestUser(this, [this, cloudData](QString user){
            ui->name->updateUser(user, !cloudData->getFileFolderAttributes()->isCurrentUser());
        });
    }
}
