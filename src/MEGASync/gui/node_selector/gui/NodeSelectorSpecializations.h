#ifndef NODESELECTORSPECIALIZED_H
#define NODESELECTORSPECIALIZED_H

#include "NodeSelector.h"

class UploadNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit UploadNodeSelector(QWidget *parent = 0);

private:
    bool isSelectionCorrect() override;
};

class DownloadNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit DownloadNodeSelector(QWidget *parent = 0);

private:
    bool isSelectionCorrect() override;
};

class SyncNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit SyncNodeSelector(QWidget *parent = 0);

private:
    bool isSelectionCorrect() override;
};

class StreamNodeSelector : public NodeSelector
{
    Q_OBJECT

public:
    explicit StreamNodeSelector(QWidget *parent = 0);

private:
    bool isSelectionCorrect() override;
};

#endif // NODESELECTORSPECIALIZED_H
