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

public slots:
    void onItemInfoUpdated(int role);

signals:
    void requestIncomingSharesRootCreation(std::shared_ptr<mega::MegaNodeList> nodes);

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
    virtual ~NodeSelectorModelBackups();

    void createRootNodes() override;
    int rootItemsCount() const override;

    void fetchMore(const QModelIndex &parent) override;
    void firstLoad() override;

    bool canBeDeleted() const override;

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
    virtual ~NodeSelectorModelSearch();
    void firstLoad() override;
    void createRootNodes() override;
    void searchByText(const QString& text);
    void stopSearch();
    int rootItemsCount() const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    const NodeSelectorModelItemSearch::Types &searchedTypes() const;

protected:
    void proxyInvalidateFinished() override;

signals:
    void searchNodes(const QString& text, NodeSelectorModelItemSearch::Types);

private slots:
    void onRootItemsCreated(NodeSelectorModelItemSearch::Types searchedTypes);

private:
    NodeSelectorModelItemSearch::Types mAllowedTypes;
    NodeSelectorModelItemSearch::Types mSearchedTypes;
};

#endif // NODESELECTORMODELSPECIALISED_H
