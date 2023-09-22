#include "LocalOrRemoteUserMustChooseStalledIssue.h"

#include <MegaApplication.h>
#include <MegaUploader.h>
#include <TransfersModel.h>
#include <StalledIssuesUtilities.h>
#include "AppStatsEvents.h"

LocalOrRemoteUserMustChooseStalledIssue::LocalOrRemoteUserMustChooseStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue(stallIssue),
      mUploader(new MegaUploader(MegaSyncApp->getMegaApi(), nullptr))
{
}

bool LocalOrRemoteUserMustChooseStalledIssue::autoSolveIssue()
{
    if(isSolvable())
    {
        chooseLastMTimeSide();

        if(isSolved())
        {
            MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_LOCALREMOTE_SOLVED_AUTOMATICALLY, "Local/Remote issue solved automatically", false, nullptr);
            return true;
        }
    }

    return false;
}

void LocalOrRemoteUserMustChooseStalledIssue::chooseLastMTimeSide()
{
    if(consultLocalData()->getAttributes()->modifiedTime() >= consultCloudData()->getAttributes()->modifiedTime())
    {
        chooseLocalSide();
    }
    else
    {
        chooseRemoteSide();
    }
}

bool LocalOrRemoteUserMustChooseStalledIssue::isSolvable() const
{
    //In case it is a backup, we cannot automatically solve it
    if(getSyncType() == mega::MegaSync::SyncType::TYPE_BACKUP)
    {
        return false;
    }

    if(isFile() && (consultLocalData()->getAttributes()->size() == consultCloudData()->getAttributes()->size()))
    {
        //Check names
        auto localName(QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(consultLocalData()->getFileName().toUtf8().constData())));
        auto cloudName(QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(consultCloudData()->getFileName().toUtf8().constData())));
        if(localName.compare(cloudName, Qt::CaseSensitive) == 0)
        {
            return true;
        }
    }

    return false;
}

void LocalOrRemoteUserMustChooseStalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    StalledIssue::fillIssue(stall);

    std::shared_ptr<UploadTransferInfo> info(new UploadTransferInfo());
    //Check if transfer already exists
    if(isBeingSolvedByUpload(info))
    {
        setIsSolved(false);
    }
}

void LocalOrRemoteUserMustChooseStalledIssue::endFillingIssue()
{
    StalledIssue::endFillingIssue();

    if(isFile())
    {
        //For autosolving
        getLocalData()->getAttributes()->requestSize(nullptr, nullptr);
        getCloudData()->getAttributes()->requestSize(nullptr, nullptr);

        getLocalData()->getAttributes()->requestModifiedTime(nullptr, nullptr);
        getCloudData()->getAttributes()->requestModifiedTime(nullptr, nullptr);
    }
}

void LocalOrRemoteUserMustChooseStalledIssue::chooseLocalSide()
{
    if(getCloudData())
    {
        std::shared_ptr<UploadTransferInfo> info(new UploadTransferInfo());
        //Check if transfer already exists
        if(!isBeingSolvedByUpload(info))
        {
            std::shared_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(info->parentHandle));
            if(parentNode)
            {
                //Using appDataId == 0 means that there will be no notification for this upload
                mUploader->upload(info->localPath, info->filename, parentNode, 0, nullptr);

                mChosenSide = ChosenSide::Local;
                setIsSolved(false);
            }
        }
    }
}

void LocalOrRemoteUserMustChooseStalledIssue::chooseRemoteSide()
{
    StalledIssuesUtilities utilities;
    utilities.removeLocalFile(consultLocalData()->getNativeFilePath());

    mChosenSide = ChosenSide::Remote;
    setIsSolved(false);
}

LocalOrRemoteUserMustChooseStalledIssue::ChosenSide LocalOrRemoteUserMustChooseStalledIssue::getChosenSide() const
{
    return mChosenSide;
}
