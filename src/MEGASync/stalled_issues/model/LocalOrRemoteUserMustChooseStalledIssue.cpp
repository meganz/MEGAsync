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

bool LocalOrRemoteUserMustChooseStalledIssue::solveIssue(bool autoSolve)
{
    if(isSolvable())
    {
        chooseLocalSide();
        return true;
    }

    return StalledIssue::solveIssue(autoSolve);
}

bool LocalOrRemoteUserMustChooseStalledIssue::isSolvable() const
{
    const char* localFingerPrint(MegaSyncApp->getMegaApi()->getFingerprint(consultLocalData()->getNativeFilePath().toUtf8().constData()));
    const char* cloudFingerPrint(consultCloudData()->getNode()->getFingerprint());

    if(std::strcmp(localFingerPrint, cloudFingerPrint) == 0)
    {
        if(consultLocalData()->getAttributes()->size() == consultCloudData()->getAttributes()->size())
        {
            //Check names
            auto localName(QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(consultLocalData()->getFileName().toUtf8().constData())));
            auto cloudName(QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(consultCloudData()->getFileName().toUtf8().constData())));
            if(localName.compare(cloudName, Qt::CaseSensitive) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

void LocalOrRemoteUserMustChooseStalledIssue::endFillingIssue()
{
    StalledIssue::endFillingIssue();

    //For autosolving
    getLocalData()->getAttributes()->requestSize(nullptr, nullptr);
    getCloudData()->getAttributes()->requestSize(nullptr, nullptr);
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
