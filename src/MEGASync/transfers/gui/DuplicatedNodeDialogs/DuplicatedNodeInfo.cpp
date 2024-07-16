#include "DuplicatedNodeInfo.h"
#include "DuplicatedUploadChecker.h"

#include <Utilities.h>
#include <MegaApplication.h>

DuplicatedNodeInfo::DuplicatedNodeInfo(DuplicatedUploadBase* checker)
    : mSolution(NodeItemType::UPLOAD),
    mSourceItemIsFile(false),
    mHasConflict(false),
    mIsNameConflict(false),
    mChecker(checker)
{
}

//UPLOAD INFO
const std::shared_ptr<mega::MegaNode> &DuplicatedNodeInfo::getParentNode() const
{
    return mParentNode;
}

void DuplicatedNodeInfo::setParentNode(const std::shared_ptr<mega::MegaNode> &newParentNode)
{
    mParentNode = newParentNode;
}

const std::shared_ptr<mega::MegaNode> &DuplicatedNodeInfo::getConflictNode() const
{
    return mConflictNode;
}

void DuplicatedNodeInfo::setConflictNode(const std::shared_ptr<mega::MegaNode> &newRemoteConflictNode)
{
    mConflictNode = newRemoteConflictNode;

    auto time = newRemoteConflictNode->isFile() ? mConflictNode->getModificationTime()
                                                : mConflictNode->getCreationTime();
    mConflictNodeModifiedTime = QDateTime::fromSecsSinceEpoch(time);
}

const QString &DuplicatedNodeInfo::getSourceItemPath() const
{
    return mSourcePath;
}

void DuplicatedNodeInfo::setSourceItemPath(const QString &newLocalPath)
{
    mSourcePath = newLocalPath;

    QFileInfo localNode(mSourcePath);
    mSourceItemIsFile = localNode.exists() && localNode.isFile();
}

NodeItemType DuplicatedNodeInfo::getSolution() const
{
    return mSolution;
}

void DuplicatedNodeInfo::setSolution(NodeItemType newSolution)
{
    mSolution = newSolution;
}

const QString &DuplicatedNodeInfo::getNewName()
{
    if(mSolution == NodeItemType::UPLOAD_AND_RENAME)
    {
        if(mNewName.isEmpty() && mConflictNode)
        {
            mNewName = Utilities::getNonDuplicatedNodeName(mConflictNode.get(), mParentNode.get(), mName, false, mChecker->getCheckedNames());
            auto& checkedNames = mChecker->getCheckedNames();
            checkedNames.removeOne(mName);
            checkedNames.append(mNewName);
        }
    }

    return mNewName;
}

const QString &DuplicatedNodeInfo::getDisplayNewName()
{
    if(mDisplayNewName.isEmpty())
    {
        mDisplayNewName = Utilities::getNonDuplicatedNodeName(mConflictNode.get(), mParentNode.get(), mName, false, mChecker->getCheckedNames());
    }

    return mDisplayNewName;
}

const QString &DuplicatedNodeInfo::getName() const
{
    return mName;
}

bool DuplicatedNodeInfo::hasConflict() const
{
    return mHasConflict;
}

void DuplicatedNodeInfo::setHasConflict(bool newHasConflict)
{
    mHasConflict = newHasConflict;
}

bool DuplicatedNodeInfo::sourceItemIsFile() const
{
    return mSourceItemIsFile;
}

bool DuplicatedNodeInfo::conflictNodeIsFile() const
{
    return mConflictNode->isFile();
}

const QDateTime &DuplicatedNodeInfo::getNodeModifiedTime() const
{
    return mConflictNodeModifiedTime;
}

const QDateTime &DuplicatedNodeInfo::getSourceItemModifiedTime() const
{
    return mSourceItemModifiedTime;
}

void DuplicatedNodeInfo::setSourceItemModifiedTime(const QDateTime &newSourceItemModifiedTime)
{
    mSourceItemModifiedTime = newSourceItemModifiedTime;
}

bool DuplicatedNodeInfo::haveDifferentType() const
{
    return sourceItemIsFile() != conflictNodeIsFile();
}

bool DuplicatedNodeInfo::isNameConflict() const
{
    return mIsNameConflict;
}

void DuplicatedNodeInfo::setIsNameConflict(bool newIsNameConflict)
{
    mIsNameConflict = newIsNameConflict;
}

DuplicatedUploadBase* DuplicatedNodeInfo::checker() const
{
    return mChecker;
}

void DuplicatedNodeInfo::setNewName(const QString &newNewName)
{
    mNewName = newNewName;
}

void DuplicatedNodeInfo::setName(const QString &newName)
{
    mName = newName;
}

mega::MegaHandle DuplicatedMoveNodeInfo::getSourceItemHandle() const
{
    return mSourceItemNode->getHandle();
}

void DuplicatedMoveNodeInfo::setSourceItemHandle(const mega::MegaHandle& sourceItemHandle)
{
    mSourceItemNode = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(sourceItemHandle));
}

bool DuplicatedMoveNodeInfo::sourceItemIsFile() const
{
    return mSourceItemNode->isFile();
}
