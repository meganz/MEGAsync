#ifndef STALLEDISSUESMODEL_H
#define STALLEDISSUESMODEL_H

#include "QTMegaRequestListener.h"
#include "QTMegaGlobalListener.h"
#include "StalledIssue.h"

#include <QObject>
#include <QMutex>
#include <QAbstractItemModel>
#include <QTimer>

class StalledIssuesReceiver : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT
public:
    explicit StalledIssuesReceiver(QObject *parent = nullptr);
    ~StalledIssuesReceiver(){}

signals:
    void stalledIssuesReady(StalledIssuesList);

protected:
    void onRequestFinish(::mega::MegaApi*, ::mega::MegaRequest *request, ::mega::MegaError*);

private:
    QMutex mCacheMutex;
    StalledIssuesList mCacheStalledIssues;

    void processStalledIssues();
};

class StalledIssuesModel : public QAbstractItemModel, public mega::MegaGlobalListener
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
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void finishStalledIssues(const QModelIndexList& indexes);
    void updateStalledIssues();

    bool hasStalledIssues() const;

signals:
    void stalledIssuesReceived(bool state);
    void stalledIssuesCountChanged();

protected slots:
    void onGlobalSyncStateChanged(mega::MegaApi *api) override;

private slots:
    void onProcessStalledIssues(StalledIssuesList list);

private:
    void removeRows(QModelIndexList &indexesToRemove);
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    void updateStalledIssuedByOrder();
    void reset();

    QThread* mStalledIssuesThread;
    StalledIssuesReceiver* mStalledIssuedReceiver;
    mega::QTMegaRequestListener* mRequestListener;
    mega::QTMegaGlobalListener* mGlobalListener;
    mega::MegaApi* mMegaApi;
    bool mHasStalledIssues;

    mutable StalledIssuesList mStalledIssues;
    mutable QHash<StalledIssue*, int> mStalledIssuesByOrder;
};

#endif // STALLEDISSUESMODEL_H
