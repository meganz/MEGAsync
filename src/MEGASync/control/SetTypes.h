#ifndef SETTYPES_H
#define SETTYPES_H

#include "megaapi.h"

#include <QList>
#include <QObject>
#include <QQueue>
#include <QString>

#include <memory>

using MegaNodeSPtr = std::shared_ptr<mega::MegaNode>;

struct SetImportParams
{
    MegaNodeSPtr importParentNode;
};

class WrappedNode;

struct AlbumCollection
{
    QString link = QString::fromUtf8("");
    QString name = QString::fromUtf8("");
    QList<mega::MegaHandle> elementHandleList = {};
    QQueue<WrappedNode> nodeList;

    // Default constructor
    AlbumCollection() = default;
    ~AlbumCollection()
    {
        reset();
    }

    void reset()
    {
        link = QString::fromUtf8("");
        name = QString::fromUtf8("");
        elementHandleList.clear();
        nodeList.clear();
    }

    bool isComplete() const
    {
        return (!link.isEmpty() &&
                !name.isEmpty() &&
                !elementHandleList.isEmpty() &&
                (elementHandleList.size() == nodeList.size()));
    }
};

Q_DECLARE_METATYPE(AlbumCollection)
Q_DECLARE_METATYPE(SetImportParams)

#endif // SETTYPES_H
