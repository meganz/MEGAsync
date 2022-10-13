#ifndef MEGAITEMPROXYMODEL_H
#define MEGAITEMPROXYMODEL_H
#include "NodeSelector.h"

#include <QSortFilterProxyModel>
#include <QCollator>
#include <QFutureWatcher>
#include <QEventLoop>


namespace mega{
class MegaNode;
}
class MegaItemModel;

class MegaItemProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    struct Filter{
        bool showReadOnly = true;
        bool showReadWriteFolders = true;
        bool showOwnerColumn = true;
        Filter() : showReadOnly(true),
                    showReadWriteFolders(true),
                    showOwnerColumn(true){};
    };

    explicit MegaItemProxyModel(QObject* parent = nullptr);
    void showReadOnlyFolders(bool value);
    void showReadWriteFolders(bool value);
    void showOwnerColumn(bool value);
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    mega::MegaHandle getHandle(const QModelIndex &index);
    std::shared_ptr<mega::MegaNode> getNode(const QModelIndex& index);
    void addNode(std::unique_ptr<mega::MegaNode> node, const QModelIndex& parent);
    QModelIndex getIndexFromSource(const QModelIndex& index);
    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle);
    QModelIndex getIndexFromNode(const std::shared_ptr<mega::MegaNode> node);
    QVector<QModelIndex> getRelatedModelIndexes(const std::shared_ptr<mega::MegaNode> node);
    void removeNode(const QModelIndex &item);
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    void setSourceModel(QAbstractItemModel *sourceModel) override;

signals:
    void modelChanged();
    void modelAboutToBeChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex& sourceParent) const override;

private:
    QVector<QModelIndex> forEach(std::shared_ptr<mega::MegaNodeList> parentNodeList, QModelIndex parent = QModelIndex());
    MegaItemModel* getMegaModel();
    Filter mFilter;
    QCollator mCollator;
    int mSortColumn;
    Qt::SortOrder mOrder;
    QFutureWatcher<void> mFilterWatcher;
    QEventLoop loop;
    QModelIndex parentChildrensToMap;



private slots:
    void invalidateModel(const QModelIndex& parent);
    void onModelSortedFiltered();
};

#endif // MEGAITEMPROXYMODEL_H
