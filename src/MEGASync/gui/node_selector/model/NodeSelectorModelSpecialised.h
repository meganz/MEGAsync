#ifndef NODESELECTORMODELSPECIALISED_H
#define NODESELECTORMODELSPECIALISED_H

#include "NodeSelectorModel.h"

#include <memory>

namespace UserAttributes{
class CameraUploadFolder;
class MyChatFilesFolder;
}

class NodeSelectorModelCloudDrive : public NodeSelectorModel
{
    Q_OBJECT

public:
    explicit NodeSelectorModelCloudDrive(QObject *parent = 0);
    virtual ~NodeSelectorModelCloudDrive() = default;

    void createRootNodes() override;
    int rootItemsCount() const override;

    void fetchMore(const QModelIndex &parent) override;
    void firstLoad() override;
    bool showsSyncStates() override {return true;}

signals:
    void requestCloudDriveRootCreation();

private slots:
    void onRootItemCreated();
};

class NodeSelectorModelIncomingShares : public NodeSelectorModel
{
    Q_OBJECT

public:
    explicit NodeSelectorModelIncomingShares(QObject *parent = 0);
    virtual ~NodeSelectorModelIncomingShares() = default;

    void createRootNodes() override;
    int rootItemsCount() const override;

    void fetchMore(const QModelIndex &parent) override;
    void firstLoad() override;
    bool rootNodeUpdated(mega::MegaNode*node) override;
    bool showsSyncStates() override {return true;}

    bool canDropMimeData(const QMimeData* data,
                         Qt::DropAction action,
                         int,
                         int,
                         const QModelIndex& parent) const override;

    QModelIndex getTopRootIndex() const override;

    bool canBeDeleted() const override;

public slots:
    void onItemInfoUpdated(int role);

protected:
    void ignoreDuplicatedNodeOptions(std::shared_ptr<mega::MegaNode> targetNode) override;

signals:
    void requestIncomingSharesRootCreation(std::shared_ptr<mega::MegaNodeList> nodes);
    void addIncomingSharesRoot(std::shared_ptr<mega::MegaNode> node);
    void deleteIncomingSharesRoot(std::shared_ptr<mega::MegaNode> node);

private slots:
    void onRootItemsCreated();

private:
    std::shared_ptr<mega::MegaNodeList> mSharedNodeList;
};

class NodeSelectorModelBackups : public NodeSelectorModel
{
    Q_OBJECT

public:
    explicit NodeSelectorModelBackups(QObject *parent = 0);
    ~NodeSelectorModelBackups() = default;

    void createRootNodes() override;
    int rootItemsCount() const override;

    void fetchMore(const QModelIndex &parent) override;
    void firstLoad() override;

    bool canBeDeleted() const override;

    bool canDropMimeData(const QMimeData*,
                         Qt::DropAction,
                         int,
                         int,
                         const QModelIndex&) const override;
    bool canDropMimeData() const override;

signals:
    void requestBackupsRootCreation(mega::MegaHandle backupHandle);

private slots:
    void onRootItemCreated();
    void onMyBackupsHandleReceived(mega::MegaHandle handle);

private:
    std::shared_ptr<mega::MegaNodeList> mBackupsNodeList;
    mega::MegaHandle mBackupsHandle;
    bool addToLoadingList(const std::shared_ptr<mega::MegaNode> node) override;
    void loadLevelFinished() override;
    int mBackupDevicesSize;

};

class NodeSelectorModelSearch : public NodeSelectorModel
{
    Q_OBJECT
public:
    explicit NodeSelectorModelSearch(NodeSelectorModelItemSearch::Types allowedType, QObject* parent = 0);
    ~NodeSelectorModelSearch() = default;
    void firstLoad() override;
    void createRootNodes() override;
    void searchByText(const QString& text);
    void stopSearch();
    int rootItemsCount() const override;
    QModelIndex getTopRootIndex() const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool addNodes(QList<std::shared_ptr<mega::MegaNode>> nodes, const QModelIndex& parent) override;
    bool rootNodeUpdated(mega::MegaNode*node) override;
    bool canDropMimeData(const QMimeData*,
                         Qt::DropAction,
                         int,
                         int,
                         const QModelIndex&) const override;
    bool canDropMimeData() const override;
    bool canCopyNodes() const override;

    const NodeSelectorModelItemSearch::Types& searchedTypes() const;
    static NodeSelectorModelItemSearch::Types calculateSearchType(mega::MegaNode* node);

protected:
    void proxyInvalidateFinished() override;
    bool showAccess(mega::MegaNode* node) const override;

signals:
    void searchNodes(const QString& text, NodeSelectorModelItemSearch::Types);
    void nodeTypeHasChanged();
    void requestAddSearchRootItem(QList<std::shared_ptr<mega::MegaNode>> nodes, NodeSelectorModelItemSearch::Types typesAllowed);
    void requestDeleteSearchRootItem(std::shared_ptr<mega::MegaNode> node);

private slots:
    void onRootItemsCreated();

private:
    NodeSelectorModelItemSearch::Types mAllowedTypes;
};

class NodeSelectorModelRubbish : public NodeSelectorModel
{
    Q_OBJECT

public:
    explicit NodeSelectorModelRubbish(QObject *parent = 0);
    virtual ~NodeSelectorModelRubbish() = default;

    void createRootNodes() override;
    int rootItemsCount() const override;

    QModelIndex getTopRootIndex() const override;

    void fetchMore(const QModelIndex &parent) override;
    void firstLoad() override;

    bool isNodeAccepted(mega::MegaNode* node) override;

    bool canDropMimeData(const QMimeData* data,
                         Qt::DropAction action,
                         int,
                         int,
                         const QModelIndex& parent) const override;
    bool canCopyNodes() const override;

public slots:
    void onItemInfoUpdated(int role);

signals:
    void requestRubbishRootCreation();
    void addRubbishRoot(std::shared_ptr<mega::MegaNode> node);

private slots:
    void onRootItemsCreated();
};

#endif // NODESELECTORMODELSPECIALISED_H
