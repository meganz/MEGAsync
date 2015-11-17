#ifndef MEGAITEM_H
#define MEGAITEM_H

#include <QList>
#include <megaapi.h>

class MegaItem
{
public:
    MegaItem(mega::MegaNode *node, MegaItem *parentItem = 0, bool showFiles = false);

    mega::MegaNode *getNode();
    void setChildren(mega::MegaNodeList *children);

    bool areChildrenSet();
    MegaItem *getParent();
    MegaItem *getChild(int i);
    int getNumChildren();
    int indexOf(MegaItem *item);

    int insertPosition(mega::MegaNode *node);
    void insertNode(mega::MegaNode *node, int index);
    void removeNode(mega::MegaNode *node);
    void displayFiles(bool enable);

    ~MegaItem();

protected:
    bool showFiles;
    MegaItem *parent;
    mega::MegaNode *node;
    mega::MegaNodeList *children;
    QList<MegaItem *> childItems;
    QList<mega::MegaNode *> insertedNodes;
};

#endif // MEGAITEM_H
