#ifndef QMEGAMODEL_H
#define QMEGAMODEL_H

#include "MegaItem.h"
#include <megaapi.h>

#include <QAbstractItemModel>
#include <QList>

#include <memory>

class QMegaModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QMegaModel(mega::MegaApi* megaApi, QObject* parent = nullptr);

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    void setRequiredRights(int requiredRights);
    void setDisableFolders(bool option);
    void showFiles(bool show);
    QModelIndex insertNode(std::shared_ptr<mega::MegaNode> node, const QModelIndex& parent);
    void removeNode(QModelIndex& item);

    std::shared_ptr<mega::MegaNode> getNode(const QModelIndex& index);

    virtual ~QMegaModel();

protected:
    mega::MegaApi* mMegaApi;
    std::shared_ptr<mega::MegaNode> mRootNode;
    std::unique_ptr<MegaItem> mRootItem;
    QList<MegaItem*> mInshareItems;
    QStringList mInshareOwners;
    QList<std::shared_ptr<mega::MegaNode>> mOwnNodes;
    int mRequiredRights;
    bool mDisplayFiles;
    bool mDisableFolders;
    mega::MegaHandle mMyBackupsRootDirHandle;
    QString mDeviceId;

private slots:
    void onMyBackupsRootDir(mega::MegaHandle handle);
};

#endif // QMEGAMODEL_H
