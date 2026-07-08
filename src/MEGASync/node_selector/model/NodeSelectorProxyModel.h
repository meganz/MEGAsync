#ifndef NODESELECTORPROXYMODEL_H
#define NODESELECTORPROXYMODEL_H

#include "ILoadingViewModel.h"
#include "megaapi.h"
#include "NodeSelectorModelItem.h"

#include <QCollator>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QSortFilterProxyModel>

#include <memory>

namespace mega
{
class MegaNode;
}
class NodeSelectorModel;

class NodeSelectorProxyModel: public QSortFilterProxyModel, public ILoadingViewModel
{
    Q_OBJECT

public:
    explicit NodeSelectorProxyModel(QObject* parent = nullptr);
    ~NodeSelectorProxyModel();

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual void applyProxyModelFlags(Qt::ItemFlags& flags, const QModelIndex& index) const {}

    mega::MegaHandle getHandle(const QModelIndex& index);
    std::shared_ptr<mega::MegaNode> getNode(const QModelIndex& index);
    QModelIndex getIndexFromSource(const QModelIndex& index);
    QModelIndex getIndexFromHandle(const mega::MegaHandle& handle);
    QModelIndex getIndexFromNode(const std::shared_ptr<mega::MegaNode> node);
    QModelIndex findIndexInParentList(const std::shared_ptr<mega::MegaNode> node);
    QModelIndex getTopRootIndex();
    void deleteNode(const QModelIndex& item);
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
    void setSourceModel(QAbstractItemModel* sourceModel) override;

    void setExpandMapped(bool value)
    {
        mExpandMapped = value;
    }

    NodeSelectorModel* getMegaModel() const;
    bool isWorking() const override;
    void onSortIndicatorChanged(int column, Qt::SortOrder order);

    virtual bool canBeDeleted() const;
    bool hasContextMenuOptions(const QModelIndexList& indexes) const;

signals:
    void expandReady();
    void navigateReady(const QModelIndex& index);
    void modelAboutToBeChanged();
    void modelSorted();
    void levelLoaded();

private:
    QModelIndex findIndexInParentList(mega::MegaNode* NodeToFind,
                                      QModelIndex sourceModelParent = QModelIndex());
    QCollator mCollator;
    int mSortColumn;
    Qt::SortOrder mOrder;
    QFutureWatcher<void> mFilterWatcher;
    QModelIndexList mItemsToMap;
    bool mExpandMapped;
    bool mForceInvalidate;
    bool mPendingSortIsLevelLoad;

private slots:
    void invalidateModel(const QList<QPair<mega::MegaHandle, QModelIndex> >& parents,
                         bool force = false);
    void onModelSortedFiltered();
};

class NodeSelectorProxyModelSync: public NodeSelectorProxyModel
{
public:
    explicit NodeSelectorProxyModelSync(QObject* parent = nullptr);
    void applyProxyModelFlags(Qt::ItemFlags& flags, const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
};

class NodeSelectorProxyModelSearch: public NodeSelectorProxyModel
{
    Q_OBJECT

public:
    explicit NodeSelectorProxyModelSearch(std::shared_ptr<NodeSelectorProxyModel> mainProxyModel,
                                          QObject* parent = nullptr);
    void setMode(TabTypes mode, bool forceFilter = true);
    bool canBeDeleted() const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

signals:
    void modeEmpty();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    TabTypes mMode;
    std::shared_ptr<NodeSelectorProxyModel> mMainProxyModel;
};

#endif // NODESELECTORPROXYMODEL_H
