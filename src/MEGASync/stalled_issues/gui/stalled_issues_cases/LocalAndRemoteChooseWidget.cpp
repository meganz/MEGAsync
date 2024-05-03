#include "LocalAndRemoteChooseWidget.h"
#include "ui_StalledIssueChooseWidget.h"
#include "StalledIssuesModel.h"

#include <platform/PlatformStrings.h>

///// Choose titles
///
///
//BASE CLASS
LocalAndRemoteStalledIssueBaseChooseWidget::LocalAndRemoteStalledIssueBaseChooseWidget(QWidget *parent) :
    StalledIssueChooseWidget(parent)
{
}

LocalAndRemoteStalledIssueBaseChooseWidget::~LocalAndRemoteStalledIssueBaseChooseWidget()
{
}

void LocalAndRemoteStalledIssueBaseChooseWidget::updateUi(StalledIssueDataPtr data,
    LocalOrRemoteUserMustChooseStalledIssue::ChosenSide side)
{
    auto fileName = data->getFileName();
    ui->chooseTitle->showIcon();

    ui->name->setTitle(fileName);

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

    if(!isSolved() &&
        side != LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::NONE)
    {
        ui->chooseTitle->hideActionButton(StalledIssueChooseWidget::BUTTON_ID);

        QIcon icon;
        if(side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::REMOTE)
        {
            if(data->isCloud())
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                ui->chooseTitle->setMessage(tr("Chosen"), icon.pixmap(16,16));
            }
            else
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/remove_default.png"));
                ui->chooseTitle->setMessage(solvedString(), icon.pixmap(16,16));
            }
        }
        else if(side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::LOCAL)
        {
            if(data->isCloud())
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/remove_default.png"));
                ui->chooseTitle->setMessage(solvedString(), icon.pixmap(16,16));
            }
            else
            {
                icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));
                ui->chooseTitle->setMessage(tr("Local file is being uploaded"), icon.pixmap(16,16));
            }
        }
        else if(side == LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::BOTH)
        {
            icon.addFile(QString::fromUtf8(":/images/StalledIssues/check_default.png"));

            if(data->isCloud())
            {
                auto cloudData = data->convert<CloudStalledIssueData>();
                auto node(cloudData->getNode(true));
                if(node)
                {
                    ui->chooseTitle->setMessage(tr("Renamed to %1").arg(QString::fromUtf8(node->getName())), icon.pixmap(16,16));
                }
            }
            else
            {
                ui->chooseTitle->setMessage(QString());
            }
        }
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

    LocalAndRemoteStalledIssueBaseChooseWidget::updateUi(localData,side);

    if(!isSolved() && side != LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::NONE)
    {
        setSolved(side != LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::LOCAL);
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

    LocalAndRemoteStalledIssueBaseChooseWidget::updateUi(cloudData, side);

    if(!isSolved() && side != LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::NONE)
    {
        setSolved(side != LocalOrRemoteUserMustChooseStalledIssue::ChosenSide::REMOTE);
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

        cloudData->getFileFolderAttributes()->requestUser(this, MegaSyncApp->getMegaApi()->getMyUserHandleBinary(), [this](QString user, bool show){
            ui->name->updateUser(user, show);
        });
    }
}
