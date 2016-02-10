#ifndef QMEGAMODEL_H
#define QMEGAMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include <QIcon>
#include "MegaItem.h"
#include <megaapi.h>

class QMegaModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QMegaModel(mega::MegaApi *megaApi, QObject *parent = 0);

    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex & index) const;
    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;

    void setRequiredRights(int requiredRights);
    void setDisableFolders(bool option);
    void showFiles(bool show);
    QModelIndex insertNode(mega::MegaNode *node, const QModelIndex &parent);
    void removeNode(QModelIndex &item);

    mega::MegaNode *getNode(const QModelIndex &index);

    virtual ~QMegaModel();

protected:
    mega::MegaApi *megaApi;
    mega::MegaNode *root;
    MegaItem *rootItem;
    QList<MegaItem *> inshareItems;
    QStringList inshareOwners;
    QList<mega::MegaNode *> ownNodes;
    QIcon folderIcon;
    int requiredRights;
    bool displayFiles;
    bool disableFolders;
};

#endif // QMEGAMODEL_H
