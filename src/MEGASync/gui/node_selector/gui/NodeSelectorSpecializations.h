#ifndef NODESELECTORSPECIALIZED_H
#define NODESELECTORSPECIALIZED_H

#include "NodeSelector.h"

class UploadNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit UploadNodeSelector(QWidget *parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void onOkButtonClicked() override;
};

class DownloadNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit DownloadNodeSelector(QWidget *parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void onOkButtonClicked() override;
};

class SyncNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit SyncNodeSelector(QWidget *parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void onOkButtonClicked() override;
    bool isFullSync();
};

class StreamNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit StreamNodeSelector(QWidget *parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void onOkButtonClicked() override;
};

////////////////////
class CloudDriveNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit CloudDriveNodeSelector(QWidget *parent = 0);

    void enableDragAndDrop(bool enable);

protected:
    void createSpecialisedWidgets() override;

protected slots:
    void onCustomBottomButtonClicked(uint id) override;
    void onItemsAboutToBeMoved(const QList<mega::MegaHandle>& handles, int type) override;
    void onMergeItemsAboutToBeMoved(mega::MegaHandle handle, int type) override;

private:
    void onOkButtonClicked() override;

    QWidget* mDragBackDrop;
};

//////////////////
class MoveBackupNodeSelector : public NodeSelector
{    
    Q_OBJECT
public:

    explicit MoveBackupNodeSelector(QWidget *parent = 0);

protected:
    void createSpecialisedWidgets() override;

private:
    void onOkButtonClicked() override;
};

#endif // NODESELECTORSPECIALIZED_H
