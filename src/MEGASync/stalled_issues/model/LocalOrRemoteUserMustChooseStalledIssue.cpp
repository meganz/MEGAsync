#include "LocalOrRemoteUserMustChooseStalledIssue.h"

#include <MegaApplication.h>
#include <MegaUploader.h>
#include <TransfersModel.h>
#include <StalledIssuesUtilities.h>

LocalOrRemoteUserMustChooseStalledIssue::LocalOrRemoteUserMustChooseStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue(stallIssue),
      mUploader(new MegaUploader(MegaSyncApp->getMegaApi(), nullptr))
{

}

void LocalOrRemoteUserMustChooseStalledIssue::solveIssue(bool)
{
    if(isFile() &&
       consultLocalData()->getAttributes()->size() == consultCloudData()->getAttributes()->size() &&
       consultLocalData()->getFileName().compare(consultCloudData()->getFileName(), Qt::CaseSensitive))
    {
        chooseLocalSide();
    }
}

void LocalOrRemoteUserMustChooseStalledIssue::chooseLocalSide()
{
    if(getCloudData())
    {
        auto node = getCloudData()->getNode();
        if(node)
        {
            std::shared_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
            if(parentNode)
            {
                //Check if transfer already exists
                TransfersModel::UploadTransferInfo info;
                info.filename = consultLocalData()->getFileName();
                info.localPath = consultLocalData()->getNativeFilePath();
                info.parentHandle = parentNode->getHandle();
                auto transfer = MegaSyncApp->getTransfersModel()->getUploadTransferByInfo(info);

                if(!transfer)
                {
                    mUploader->upload(info.localPath, info.filename, parentNode, -1, nullptr);
                }

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
