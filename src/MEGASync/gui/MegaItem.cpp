#include "MegaItem.h"

#include <QByteArray>

MegaItem::MegaItem(std::shared_ptr<mega::MegaNode> node, MegaItem* parentItem, bool showFiles) :
    mShowFiles (showFiles),
    mParentItem (parentItem),
    mNode (node),
    mAreChildrenSet (false)
{
}

std::shared_ptr<mega::MegaNode> MegaItem::getNode()
{
    return mNode;
}

void MegaItem::setChildren(std::shared_ptr<mega::MegaNodeList> children)
{
    for (int i = 0; i < children->size(); i++)
    {
        std::shared_ptr<mega::MegaNode> node (children->get(i)->copy());
        if (!mShowFiles && node->getType() == mega::MegaNode::TYPE_FILE)
        {
            break;
        }
        mChildItems.append(new MegaItem(node, this, mShowFiles));
    }
    mAreChildrenSet = true;
}

bool MegaItem::areChildrenSet()
{
    return mAreChildrenSet;
}

MegaItem *MegaItem::getParent()
{
    return mParentItem;
}

MegaItem* MegaItem::getChild(int i)
{
    return mChildItems.at(i);
}

int MegaItem::getNumChildren()
{
    return mChildItems.size();
}

int MegaItem::indexOf(MegaItem* item)
{
    return mChildItems.indexOf(item);
}

int MegaItem::insertPosition(std::shared_ptr<mega::MegaNode> node)
{
    int type = node->getType();

    int i;
    for (i = 0; i < mChildItems.size(); i++)
    {
        std::shared_ptr<mega::MegaNode> n (mChildItems.at(i)->getNode());
        int nodeType = n->getType();
        if (type < nodeType)
        {
            continue;
        }

        if (qstricmp(node->getName(), n->getName()) <= 0)
        {
            break;
        }
    }

    return i;
}

void MegaItem::insertNode(std::shared_ptr<mega::MegaNode> node, int index)
{
    mChildItems.insert(index, new MegaItem(node, this, mShowFiles));
    mInsertedNodes.append(node);
}

void MegaItem::removeNode(std::shared_ptr<mega::MegaNode> node)
{
    if (!node)
    {
        return;
    }

    for (int i = 0; i < mChildItems.size(); i++)
    {
        if (mChildItems[i]->getNode()->getHandle() == node->getHandle())
        {
            delete mChildItems[i];
            mChildItems.removeAt(i);
            break;
        }
    }

    for (int i = 0; i < mInsertedNodes.size(); i++)
    {
        if (mInsertedNodes[i]->getHandle() == node->getHandle())
        {
            mInsertedNodes.removeAt(i);
            break;
        }
    }
}

void MegaItem::displayFiles(bool enable)
{
    this->mShowFiles = enable;
}

MegaItem::~MegaItem()
{
    qDeleteAll(mChildItems);
}

