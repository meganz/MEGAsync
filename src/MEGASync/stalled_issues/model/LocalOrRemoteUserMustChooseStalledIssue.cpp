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

void LocalOrRemoteUserMustChooseStalledIssue::autoSolveIssue()
{
    if(isSolvable())
    {
        chooseLastMTimeSide();

        if(isSolved())
        {
            MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_LOCALREMOTE_SOLVED_AUTOMATICALLY, "Local/Remote issue solved automatically", false, nullptr);
        }
    }
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

bool LocalOrRemoteUserMustChooseStalledIssue::isBeingSolved(TransfersModel::UploadTransferInfo& info) const
{
    auto result(false);

    auto node = getCloudData()->getNode();
    if(node)
    {
        info.filename = consultLocalData()->getFileName();
        info.localPath = consultLocalData()->getNativeFilePath();
        info.parentHandle = node->getParentHandle();
        auto transfer = MegaSyncApp->getTransfersModel()->activeTransferFound(info);

        result =  transfer != nullptr;
    }

    return result;
}

void LocalOrRemoteUserMustChooseStalledIssue::fillIssue(const mega::MegaSyncStall *stall)
{
    StalledIssue::fillIssue(stall);

    TransfersModel::UploadTransferInfo info;
    //Check if transfer already exists
    if(isBeingSolved(info))
    {
        MegaSyncApp->getMegaApi()->clearStalledPath(getOriginalStall().get());
        setIsSolved();
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
        TransfersModel::UploadTransferInfo info;
        //Check if transfer already exists
        if(!isBeingSolved(info))
        {
            std::shared_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(info.parentHandle));
            if(parentNode)
            {
                //Using appDataId == 0 means that there will be no notification for this upload
                mUploader->upload(info.localPath, info.filename, parentNode, 0, nullptr);

                // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
                MegaSyncApp->getMegaApi()->clearStalledPath(getOriginalStall().get());
                mChosenSide = ChosenSide::Local;
                setIsSolved();
            }
        }
    }
}

void LocalOrRemoteUserMustChooseStalledIssue::chooseRemoteSide()
{
    StalledIssuesUtilities utilities;
    utilities.removeLocalFile(consultLocalData()->getNativeFilePath());

    // Prevent this one showing again (if they Refresh) until sync has made a full fresh pass
    MegaSyncApp->getMegaApi()->clearStalledPath(getOriginalStall().get());
    mChosenSide = ChosenSide::Remote;
    setIsSolved();
}

LocalOrRemoteUserMustChooseStalledIssue::ChosenSide LocalOrRemoteUserMustChooseStalledIssue::getChosenSide() const
{
    return mChosenSide;
}
