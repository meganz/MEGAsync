#ifndef NODESELECTORSPECIALIZED_H
#define NODESELECTORSPECIALIZED_H

#include "NodeSelector.h"

class UploadNodeSelector : public NodeSelector
{
public:
    explicit UploadNodeSelector(QWidget *parent = 0);

private:
    bool isSelectionCorrect() override;
};

class DownloadNodeSelector : public NodeSelector
{
public:
    explicit DownloadNodeSelector(QWidget *parent = 0);

private:
    bool isSelectionCorrect() override;
};

class SyncNodeSelector : public NodeSelector
{
public:
    explicit SyncNodeSelector(QWidget *parent = 0);

private:
    bool isSelectionCorrect() override;
};

class StreamNodeSelector : public NodeSelector
{
public:
    explicit StreamNodeSelector(QWidget *parent = 0);

private:
    bool isSelectionCorrect() override;
};

#endif // NODESELECTORSPECIALIZED_H
