#include "DuplicatedNodeInfo.h"

#include <Utilities.h>
#include <MegaApplication.h>

DuplicatedNodeInfo::DuplicatedNodeInfo() : mSolution(NodeItemType::DONT_UPLOAD), mIsLocalFile(false), mHasConflict(false), mHaveDifferentType(false)
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

    initNewName();

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
    auto node = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getChildNodeOfType(parentNode.get(), nodeName.toStdString().c_str(),
                                                              isLocalFile() ? mega::MegaNode::TYPE_FILE : mega::MegaNode::TYPE_FOLDER));

//    TODO -> asked if this feature should be added or not
//    if(!node)
//    {
//        node = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getChildNode(parentNode.get(), nodeName.toStdString().c_str()));
//        if(node)
//        {
//            mHaveDifferentType = true;
//        }
//    }

    return node;
}

void DuplicatedNodeInfo::setSolution(NodeItemType newSolution)
{
    mSolution = newSolution;

    if(mSolution != NodeItemType::UPLOAD_AND_RENAME)
    {
        mNewName.clear();
    }
}

const QString &DuplicatedNodeInfo::getNewName() const
{
    return mNewName;
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
    if(mLocalModifiedTime != newLocalModifiedTime)
    {
        mLocalModifiedTime = newLocalModifiedTime;
        emit localModifiedDateUpdated();
    }
}

bool DuplicatedNodeInfo::haveDifferentType() const
{
    return mHaveDifferentType;
}

void DuplicatedNodeInfo::initNewName()
{
    QString nodeName;
    QString suffix;

    if(mRemoteConflictNode->isFile())
    {
        QFileInfo fileInfo(QString::fromUtf8(mRemoteConflictNode->getName()));

        auto nameSplitted = Utilities::getFilenameBasenameAndSuffix(fileInfo.fileName());
        if(nameSplitted != QPair<QString, QString>())
        {
            nodeName = nameSplitted.first;
            suffix = nameSplitted.second;
        }
        else
        {
            nodeName = fileInfo.baseName();
            suffix = fileInfo.suffix();
        }
    }
    else
    {
        nodeName = mName;
    }

    bool nameFound(false);
    int counter(1);
    while(!nameFound)
    {
        QString repeatedName = nodeName + QString(QLatin1Literal("(%1)")).arg(QString::number(counter));
        if(mRemoteConflictNode)
        {
            if(mRemoteConflictNode->isFile())
            {
                repeatedName.append(suffix);
            }
        }

        auto foundNode = checkNameNode(repeatedName, mParentNode);
        if(!foundNode)
        {
            mNewName = repeatedName;
            nameFound = true;
        }

        counter++;
    }
}
