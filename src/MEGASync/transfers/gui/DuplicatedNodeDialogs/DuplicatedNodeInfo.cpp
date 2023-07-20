#include "DuplicatedNodeInfo.h"
#include "DuplicatedUploadChecker.h"

#include <Utilities.h>
#include <MegaApplication.h>

DuplicatedNodeInfo::DuplicatedNodeInfo(DuplicatedUploadBase* checker) : mSolution(NodeItemType::DONT_UPLOAD), mIsLocalFile(false), mHasConflict(false), mHaveDifferentType(false), mChecker(checker)
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

const std::shared_ptr<mega::MegaNode> &DuplicatedNodeInfo::getRemoteConflictNode() const
{
    return mRemoteConflictNode;
}

void DuplicatedNodeInfo::setRemoteConflictNode(const std::shared_ptr<mega::MegaNode> &newRemoteConflictNode)
{
    mRemoteConflictNode = newRemoteConflictNode;

    mName = QString::fromUtf8(mRemoteConflictNode->getName()).toHtmlEscaped();

    auto time = newRemoteConflictNode->isFile() ? mRemoteConflictNode->getModificationTime()
                                                : mRemoteConflictNode->getCreationTime();
    mNodeModifiedTime = QDateTime::fromSecsSinceEpoch(time);
}

const QString &DuplicatedNodeInfo::getLocalPath() const
{
    return mLocalPath;
}

void DuplicatedNodeInfo::setLocalPath(const QString &newLocalPath)
{
    mLocalPath = newLocalPath;

    QFileInfo localNode(mLocalPath);
    mIsLocalFile = localNode.exists() && localNode.isFile();
}

NodeItemType DuplicatedNodeInfo::getSolution() const
{
    return mSolution;
}

std::shared_ptr<mega::MegaNode> DuplicatedNodeInfo::checkNameNode(const QString &nodeName, std::shared_ptr<mega::MegaNode> parentNode)
{
    auto node = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getChildNodeOfType(parentNode.get(), nodeName.toUtf8().constData(),
                                                              isLocalFile() ? mega::MegaNode::TYPE_FILE : mega::MegaNode::TYPE_FOLDER));

    return node;
}

void DuplicatedNodeInfo::setSolution(NodeItemType newSolution)
{
    mSolution = newSolution;
}

const QString &DuplicatedNodeInfo::getNewName()
{
    if(mSolution == NodeItemType::UPLOAD_AND_RENAME)
    {
        if(mNewName.isEmpty() && mRemoteConflictNode)
        {
            mNewName = Utilities::getNonDuplicatedNodeName(mRemoteConflictNode.get(), mParentNode.get(), mName, false, mChecker->getCheckedNames());
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
        mDisplayNewName = Utilities::getNonDuplicatedNodeName(mRemoteConflictNode.get(), mParentNode.get(), mName, false, mChecker->getCheckedNames());
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

bool DuplicatedNodeInfo::isLocalFile() const
{
    return mIsLocalFile;
}

bool DuplicatedNodeInfo::isRemoteFile() const
{
    return mRemoteConflictNode->isFile();
}

const QDateTime &DuplicatedNodeInfo::getNodeModifiedTime() const
{
    return mNodeModifiedTime;
}

const QDateTime &DuplicatedNodeInfo::getLocalModifiedTime() const
{
    return mLocalModifiedTime;
}

void DuplicatedNodeInfo::setLocalModifiedTime(const QDateTime &newLocalModifiedTime)
{
    mLocalModifiedTime = newLocalModifiedTime;
    emit localModifiedDateUpdated();
}

bool DuplicatedNodeInfo::haveDifferentType() const
{
    return mHaveDifferentType;
}
