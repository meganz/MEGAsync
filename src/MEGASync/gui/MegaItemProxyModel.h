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
        bool showInShares = false;
        bool showCloudDrive = true;
        bool showReadOnly = true;
        bool showReadWriteFolders = true;
        bool showOwnerColumn = true;
        QString textFilter;
        Filter() : showInShares(false), showCloudDrive(true), showReadOnly(true),
                    showReadWriteFolders(true){};
        void showOnlyCloudDrive(){showInShares=false; showCloudDrive = true;}
        void showOnlyInShares(bool isSyncSelect = true){showInShares = true; showCloudDrive = false; showReadWriteFolders = !isSyncSelect;}
        bool isShowOnlyInShares(){return showInShares && !showCloudDrive;}
        bool isShowOnlyCloudDrive(){return !showInShares && showCloudDrive;}
    };

    explicit MegaItemProxyModel(QObject* parent = nullptr);
    void showOnlyCloudDrive();
    void showOnlyInShares(bool isSyncSelect = true);
    void showReadOnlyFolders(bool value);
    void showReadWriteFolders(bool value);
    void showOwnerColumn(bool value);
    void setTextFilter(const QString& textFilter);

    mega::MegaHandle getHandle(const QModelIndex &index);
    std::shared_ptr<mega::MegaNode> getNode(const QModelIndex& index);
    void addNode(std::unique_ptr<mega::MegaNode> node, const QModelIndex& parent);
    QModelIndex getIndexFromSource(const QModelIndex& index);
    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle);
    QModelIndex getIndexFromNode(const std::shared_ptr<mega::MegaNode> node);
    QVector<QModelIndex> getRelatedModelIndexes(const std::shared_ptr<mega::MegaNode> node, bool isInShare);
    void removeNode(const QModelIndex &item);
    bool isShowOnlyInShares();
    bool isShowOnlyCloudDrive();
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
