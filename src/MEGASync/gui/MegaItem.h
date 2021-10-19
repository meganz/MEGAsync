#ifndef MEGAITEM_H
#define MEGAITEM_H

#include <QList>
#include <megaapi.h>

#include <memory>

class MegaItem
{
public:
    MegaItem(std::shared_ptr<mega::MegaNode> node, MegaItem* parentItem = nullptr,
             bool showFiles = false);

    std::shared_ptr<mega::MegaNode> getNode();
    void setChildren(std::shared_ptr<mega::MegaNodeList> children);

    bool areChildrenSet();
    MegaItem *getParent();
    MegaItem *getChild(int i);
    int getNumChildren();
    int indexOf(MegaItem *item);

    int insertPosition(std::shared_ptr<mega::MegaNode> node);
    void insertNode(std::shared_ptr<mega::MegaNode> node, int index);
    void removeNode(std::shared_ptr<mega::MegaNode> node);
    void displayFiles(bool enable);

    ~MegaItem();

protected:
    bool mShowFiles;
    MegaItem* mParentItem;
    std::shared_ptr<mega::MegaNode> mNode;
    bool mAreChildrenSet;
    QList<MegaItem*> mChildItems;
    QList<std::shared_ptr<mega::MegaNode>> mInsertedNodes;
};

#endif // MEGAITEM_H
