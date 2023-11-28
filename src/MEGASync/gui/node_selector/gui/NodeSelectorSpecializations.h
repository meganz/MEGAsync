#ifndef NODESELECTORSPECIALIZED_H
#define NODESELECTORSPECIALIZED_H

#include "NodeSelector.h"

class UploadNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit UploadNodeSelector(QWidget *parent = 0);

private:
    void checkSelection() override;
};

class DownloadNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit DownloadNodeSelector(QWidget *parent = 0);

private:
    void checkSelection() override;
};

class SyncNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit SyncNodeSelector(QWidget *parent = 0);

private:
    void checkSelection() override;
};

class StreamNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit StreamNodeSelector(QWidget *parent = 0);

private:
    void checkSelection() override;
};

class CloudDriveNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit CloudDriveNodeSelector(QWidget *parent = 0);

    void enableDragAndDrop(bool enable);

protected slots:
    void onCustomBottomButtonClicked(uint8_t id) override;

private:
    void checkSelection(){}

    QWidget* mDragBackDrop;
};

#endif // NODESELECTORSPECIALIZED_H
