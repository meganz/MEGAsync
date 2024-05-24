#include "LinkObject.h"

#include <QCoreApplication>

LinkObject::LinkObject(mega::MegaApi* megaApi, MegaNodeSPtr node, const QString& link)
    : mMegaApi(megaApi)
    , mLink(link)
    , mNode(node)
    , mImportNode(nullptr)
    , mName(getDefaultName())
    , mDownloadPath(DEFAULT_STR)
    , mIsSelected(false)
    , mLinkType(linkType::INVALID)
    , mLinkStatus(linkStatus::FAILED)
    , mShowFolderIcon(false)
{}

LinkObject::~LinkObject() {}

const QString LinkObject::getDefaultName() const
{
    return QCoreApplication::translate("ImportMegaLinksDialog", "Not found");
}

QString LinkObject::getLink() const { return mLink; }

void LinkObject::setName(const QString& name) { mName = name; }

QString LinkObject::getName() const { return mName; }

int64_t LinkObject::getSize() const { return 0; }

linkType LinkObject::getLinkType() const { return mLinkType; }

void LinkObject::setLinkStatus(const linkStatus status) { mLinkStatus = status; }

linkStatus LinkObject::getLinkStatus() const { return mLinkStatus; }

MegaNodeSPtr LinkObject::getMegaNode() const { return mNode; }

void LinkObject::setSelected(bool selected) { mIsSelected = selected; }

bool LinkObject::isSelected() const { return mIsSelected; }

void LinkObject::setImportNode(MegaNodeSPtr node) { mImportNode = node; }

MegaNodeSPtr LinkObject::getImportNode() const { return mImportNode; }

void LinkObject::setDownloadPath(const QString& path) { mDownloadPath = path; }

QString LinkObject::getDownloadPath() const { return mDownloadPath; }

bool LinkObject::showFolderIcon() const { return mShowFolderIcon; }

bool LinkObject::readyForProcessing() const { return false; }

void LinkObject::reset()
{
    mNode = nullptr;
    mImportNode = nullptr;
    mName = getDefaultName();
    mDownloadPath = DEFAULT_STR;
    mIsSelected = false;
    mLinkType = linkType::INVALID;
    mLinkStatus = linkStatus::FAILED;
    mShowFolderIcon = false;
}

LinkInvalid::LinkInvalid(mega::MegaApi* megaApi, MegaNodeSPtr node, const QString& link)
    : LinkObject(megaApi, node, link)
{}

LinkNode::LinkNode(mega::MegaApi* megaApi, MegaNodeSPtr node, const QString& link)
    : LinkObject(megaApi, node, link)
{
    if (!node) { return; }

    mLinkType = linkType::NODE;
    updateNodeDetails();
}

int64_t LinkNode::getSize() const
{
    if (!mNode) { return 0; }
    return mMegaApi->getSize(mNode.get());
}

bool LinkNode::readyForProcessing() const
{
    return (mNode && mIsSelected);
}

void LinkNode::setMegaNode(MegaNodeSPtr node)
{
    if (!mNode) { return; }
    mNode = node;
    updateNodeDetails();
}

void LinkNode::updateNodeDetails()
{
    if (!mNode) { return; }

    mName = QString::fromUtf8(mNode->getName());
    mLinkStatus = mNode->isNodeKeyDecrypted() ? linkStatus::CORRECT : linkStatus::WARNING;
    mShowFolderIcon = !(mNode->getType() == mega::MegaNode::TYPE_FILE);
}

LinkSet::LinkSet(mega::MegaApi* megaApi, const AlbumCollection& set)
    : LinkObject(megaApi, nullptr, set.link)
    , mNrElements(set.elementHandleList.size())
{
    mName = set.name;
    mLinkType = linkType::SET;
    mShowFolderIcon = true;
    mSet = set;
}

linkStatus LinkSet::getLinkStatus() const
{
    for (const auto& linkNodePtr : mSet.nodeList)
    {
        if (!linkNodePtr->isNodeKeyDecrypted())
        {
            return linkStatus::WARNING;
        }
    }

    return linkStatus::CORRECT;
}

bool LinkSet::readyForProcessing() const
{
    return mIsSelected;
}

void LinkSet::reset()
{
    LinkObject::reset();
    mSet.reset();
}

int64_t LinkSet::getSize() const
{
    int64_t totalValue = 0;

    for (const auto& linkNodePtr : mSet.nodeList)
    {
        totalValue += (linkNodePtr ? mMegaApi->getSize(linkNodePtr.get()) : 0);
    }

    return totalValue;
}

AlbumCollection LinkSet::getSet() const
{
    return mSet;
}
