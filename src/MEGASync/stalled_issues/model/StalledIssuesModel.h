#ifndef STALLEDISSUESMODEL_H
#define STALLEDISSUESMODEL_H

#include "QTMegaRequestListener.h"
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

    StalledIssuesDataList processStalledIssues();

protected:
    void onRequestFinish(::mega::MegaApi*, ::mega::MegaRequest *request, ::mega::MegaError*);

private:
    QMutex mCacheMutex;
    StalledIssuesDataList mCacheStalledIssues;
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
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void finishStalledIssues(const QModelIndexList& indexes);

signals:
    void stalledIssuesReceived(bool state);

private slots:
    void onProcessStalledIssues();

private:
    void removeRows(QModelIndexList &indexesToRemove);
    void updateStalledIssuedByOrder();

    QThread* mStalledIssuesThread;
    StalledIssuesReceiver* mStalledIssuedReceiver;
    mega::QTMegaRequestListener* mDelegateListener;
    mega::MegaApi* mMegaApi;
    QTimer mStalledIssuesTimer;

    mutable StalledIssuesDataList mStalledIssues;
    mutable QHash<QExplicitlySharedDataPointer<StalledIssueData>, int> mStalledIssuesByOrder;
    mutable QStringList mAddedStalledIssues;
};

#endif // STALLEDISSUESMODEL_H
