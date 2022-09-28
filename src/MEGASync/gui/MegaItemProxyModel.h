#ifndef MEGAITEMPROXYMODEL_H
#define MEGAITEMPROXYMODEL_H
#include "NodeSelector.h"

#include <QSortFilterProxyModel>
#include <QCollator>


namespace mega{
class MegaNode;
}
class MegaItemModel;

class MegaItemProxyModel : public QSortFilterProxyModel
{

public:
    struct Filter{
        bool showReadOnly = true;
        bool showReadWriteFolders = true;
        bool showOwnerColumn = true;
        bool showFiles = true;
        Filter() : showReadOnly(true),
                    showReadWriteFolders(true),
                    showOwnerColumn(true),
                    showFiles(true){};
    };

    explicit MegaItemProxyModel(QObject* parent = nullptr);
    void showReadOnlyFolders(bool value);
    void showReadWriteFolders(bool value);
    void showOwnerColumn(bool value);
    void showFiles(bool value);

    mega::MegaHandle getHandle(const QModelIndex &index);
    std::shared_ptr<mega::MegaNode> getNode(const QModelIndex& index);
    void addNode(std::unique_ptr<mega::MegaNode> node, const QModelIndex& parent);
    QModelIndex getIndexFromSource(const QModelIndex& index);
    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle);
    QModelIndex getIndexFromNode(const std::shared_ptr<mega::MegaNode> node);
    QVector<QModelIndex> getRelatedModelIndexes(const std::shared_ptr<mega::MegaNode> node);
    void removeNode(const QModelIndex &item);
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;


protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex& sourceParent) const override;

private:
    QVector<QModelIndex> forEach(std::shared_ptr<mega::MegaNodeList> parentNodeList, QModelIndex parent = QModelIndex());
    MegaItemModel* getMegaModel();
    Filter mFilter;
    QCollator mCollator;
};

#endif // MEGAITEMPROXYMODEL_H
