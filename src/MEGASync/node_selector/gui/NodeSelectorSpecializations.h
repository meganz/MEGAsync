#ifndef NODESELECTORSPECIALIZED_H
#define NODESELECTORSPECIALIZED_H

#include "NodeSelector.h"

class UploadNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit UploadNodeSelector(QWidget* parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void onOkButtonClicked() override;
};

class DownloadNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit DownloadNodeSelector(QWidget* parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void onOkButtonClicked() override;
};

class SyncNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit SyncNodeSelector(QWidget* parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void onOkButtonClicked() override;
    bool isFullSync();
};

class StreamNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit StreamNodeSelector(QWidget* parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void onOkButtonClicked() override;
};

////////////////////
class CloudDriveNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit CloudDriveNodeSelector(QWidget* parent = 0);

    void enableDragAndDrop(bool enable);

    static void sendStats();

protected:
    void createSpecialisedWidgets() override;

protected slots:
    void onCustomButtonClicked(uint id) override;
    void onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int type) override;
    void onItemsAboutToBeMovedFailed(const QList<mega::MegaHandle>& handles, int type) override;
    void onItemsAboutToBeRestored(const QSet<mega::MegaHandle>& targetHandles) override;

    void onItemAboutToBeReplaced(mega::MegaHandle handle) override;

    void onItemsAboutToBeMerged(const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
                                int actionType) override;

    void onItemsAboutToBeMergedFailed(
        const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
        int actionType) override;

private:
    void performMergeAction(const QList<std::shared_ptr<NodeSelectorMergeInfo>>& mergesInfo,
                            int actionType,
                            NodeSelector::IncreaseOrDecrease type);

    void onOkButtonClicked() override {}
    void checkMovingItems(const QList<mega::MegaHandle>& handles,
                          int moveType,
                          NodeSelector::IncreaseOrDecrease type);

    struct HandlesByTab
    {
        QList<mega::MegaHandle> cloudDriveNodes;
        QList<mega::MegaHandle> incomingSharedNodes;
    };

    HandlesByTab getTabs(const QList<mega::MegaHandle>& handles);
    void selectTabs(const HandlesByTab& tabsInfo);

    QWidget* mDragBackDrop;
};

//////////////////
class MoveBackupNodeSelector: public NodeSelector
{
    Q_OBJECT

public:
    explicit MoveBackupNodeSelector(QWidget* parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void onOkButtonClicked() override;
};

#endif // NODESELECTORSPECIALIZED_H
