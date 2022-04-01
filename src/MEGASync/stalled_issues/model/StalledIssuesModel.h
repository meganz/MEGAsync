#ifndef STALLEDISSUESMODEL_H
#define STALLEDISSUESMODEL_H

#include "QTMegaTransferListener.h"
#include "StalledIssue.h"

#include <QObject>
#include <QMutex>
#include <QAbstractItemModel>

class StalledIssuesReceiver : public QObject, public mega::MegaTransferListener
{
    Q_OBJECT
public:
    explicit StalledIssuesReceiver(QObject *parent = nullptr);
    ~StalledIssuesReceiver(){}

protected:
    void onRequestFinish(::mega::MegaApi* api, ::mega::MegaRequest *request, ::mega::MegaError* e);

private:
    QMutex mCacheMutex;
};

class StalledIssuesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit StalledIssuesModel(QObject* parent = 0);
    ~StalledIssuesModel();

    virtual Qt::DropActions supportedDropActions() const;

    bool hasChildren(const QModelIndex& parent) const;
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;
    QModelIndex parent(const QModelIndex& index) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;

private:
    QThread* mStalledIssuesThread;
    StalledIssuesReceiver* mStalledIssuedReceiver;
    mega::QTMegaTransferListener* mDelegateListener;
    mega::MegaApi* mMegaApi;

    mutable QList<QExplicitlySharedDataPointer<StalledIssueData>> mStalledIssues;
    mutable QHash<QExplicitlySharedDataPointer<StalledIssueData>, int> mStalledIssuesByOrder;
};

#endif // STALLEDISSUESMODEL_H
