#ifndef STALLEDISSUESMODEL_H
#define STALLEDISSUESMODEL_H

#include "QTMegaRequestListener.h"
#include "QTMegaGlobalListener.h"
#include "StalledIssue.h"
#include "StalledIssuesUtilities.h"
#include "ViewLoadingScene.h"
#include <MoveOrRenameCannotOccurIssue.h>
#include <StalledIssuesFactory.h>
#include <DesktopNotifications.h>
#include "QMegaMessageBox.h"
#include "TextDecorator.h"

#include <QObject>
#include <QReadWriteLock>
#include <QAbstractItemModel>
#include <QTimer>
#include <QPointer>

class LoadingSceneMessageHandler;
class NameConflictedStalledIssue;

class StalledIssuesStrings
{
public:
    static QString RemoveFileFailedTitle()
    {
        return QT_TRANSLATE_NOOP("StalledIssues", QLatin1String("Unable to remove this file."));
    }

    static QString RemoveFolderFailedTitle()
    {
        return QT_TRANSLATE_NOOP("StalledIssues", QLatin1String("Unable to remove this folder."));
    }

    static QString RemoveLocalFileFailedDescription()
    {
        return QT_TRANSLATE_NOOP(
            "StalledIssues",
            QLatin1String(
                "Check if the file is in use, and the permissions of the file, then try again."));
    }

    static QString RemoveLocalFolderFailedDescription()
    {
        return QT_TRANSLATE_NOOP(
            "StalledIssues",
            QLatin1String(
                "Check if the folder is in use, and the permissions of the file, then try again."));
    }

    static QString RemoveRemoteFailedDescription(const mega::MegaError* error)
    {
        return QT_TRANSLATE_NOOP(
            "StalledIssues",
            QLatin1String("Error: %1").arg(Utilities::getTranslatedError(error)));
    }

    static QString RemoveRemoteFailedFile(const mega::MegaError* error)
    {
        QString errorStr = QT_TRANSLATE_NOOP("StalledIssues", QLatin1String("%1[BR]%2"))
                               .arg(RemoveFileFailedTitle(), RemoveRemoteFailedDescription(error));
        StalledIssuesNewLineTextDecorator::newLineTextDecorator.process(errorStr);
        return errorStr;
    }

    static QString RemoveRemoteFailedFolder(const mega::MegaError* error)
    {
        QString errorStr =
            QT_TRANSLATE_NOOP("StalledIssues", QLatin1String("%1[BR]%2"))
                .arg(RemoveFolderFailedTitle(), RemoveRemoteFailedDescription(error));
        StalledIssuesNewLineTextDecorator::newLineTextDecorator.process(errorStr);
        return errorStr;
    }

    static QString RemoveLocalFailedFile()
    {
        QString errorStr = QT_TRANSLATE_NOOP("StalledIssues", QLatin1String("%1[BR]%2"));
        errorStr = errorStr.arg(RemoveFileFailedTitle(), RemoveLocalFileFailedDescription());
        StalledIssuesNewLineTextDecorator::newLineTextDecorator.process(errorStr);
        return errorStr;
    }

    static QString RemoveLocalFailedFolder()
    {
        QString errorStr = QT_TRANSLATE_NOOP("StalledIssues", QLatin1String("%1[BR]%2"));
        errorStr = errorStr.arg(RemoveFolderFailedTitle(), RemoveLocalFolderFailedDescription());
        StalledIssuesNewLineTextDecorator::newLineTextDecorator.process(errorStr);
        return errorStr;
    }
};

class StalledIssuesReceiver : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT
public:
    explicit StalledIssuesReceiver(QObject* parent = nullptr);
    ~StalledIssuesReceiver(){}

    bool multiStepIssueSolveActive() const;

    template <class ISSUE_TYPE>
    void addMultiStepIssueSolver(MultiStepIssueSolverBase* solver)
    {
        solver->moveToThread(thread());
        solver->start();
        mIssueCreator.addMultiStepIssueSolver(solver);
    }

public slots:
    void onUpdateStalledISsues(UpdateType type);

signals:
    void stalledIssuesReady(ReceivedStalledIssues);
    void solvingIssues(StalledIssuesCreator::IssuesCount count);
    void solvingIssuesFinished(StalledIssuesCreator::IssuesCount count);

protected:
    void onRequestFinish(::mega::MegaApi*, ::mega::MegaRequest* request, ::mega::MegaError*);

private:
    QMutex mCacheMutex;
    ReceivedStalledIssues mStalledIssues;
    StalledIssuesCreator mIssueCreator;
    std::atomic<UpdateType> mUpdateType {UpdateType::NONE};
    std::atomic_int mUpdateRequests {0};
};

class StalledIssuesModel : public QAbstractItemModel, public mega::MegaGlobalListener
{
    Q_OBJECT

public:
    static const int ADAPTATIVE_HEIGHT_ROLE;

    explicit StalledIssuesModel(QObject* parent = 0);
    ~StalledIssuesModel();

    virtual Qt::DropActions supportedDropActions() const override;
    bool hasChildren(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    StalledIssueVariant getStalledIssueByRow(int row) const;
    QModelIndex parent(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    //Removes the pending and solved issues
    void fullReset();

    bool isEmpty() const;

    int getCountByFilterCriterion(StalledIssueFilterCriterion criterion);

    void finishStalledIssues(const QModelIndexList& indexes);
    void updateStalledIssues();

    void blockUi();
    void unBlockUi();

    void updateIndex(const QModelIndex& index);

    QModelIndexList getIssuesByReason(QList<mega::MegaSyncStall::SyncStallReason> reasons);
    QModelIndexList getIssues(std::function<bool (const std::shared_ptr<const StalledIssue>)> checker);

    static void runMessageBox(QMegaMessageBox::MessageBoxInfo info);

    void languageChanged();

    //SHOW RAW INFO
    void showRawInfo(bool state);
    bool isRawInfoVisible() const;

    //ISSUE USE FOR UI ITEM
    void UiItemUpdate(const QModelIndex& oldIndex, const QModelIndex& newIndex);

    //SOLVE PROBLEMS
    void stopSolvingIssues(MessageInfo::ButtonType buttonType);

    //Solve all issues
    void solveAllIssues();

    bool checkForExternalChanges(const QModelIndex& index);

    //Name conflicts
    bool solveLocalConflictedNameByRemove(int conflictIndex, const QModelIndex& index);
    bool solveLocalConflictedNameByRename(const QString& renameTo, const QString& renameFrom, int conflictIndex, const QModelIndex& index);
    void solveLocalConflictedNameFailed(int conflictIndex, const QModelIndex& index, const QString& error);

    bool solveCloudConflictedNameByRemove(int conflictIndex, const QModelIndex& index);
    bool solveCloudConflictedNameByRename(const QString& renameTo, const QString& renameFrom, int conflictIndex, const QModelIndex& index);
    void solveCloudConflictedNameFailed(int conflictIndex, const QModelIndex& index, const QString& error);

    void finishConflictManually();

    void semiAutoSolveNameConflictIssues(const QModelIndexList& list, int option);

    //LocalOrRemoteConflicts
    void chooseRemoteForBackups(const QModelIndexList& list);
    void chooseSideManually(bool remote, const QModelIndexList& list);
    void chooseBothSides(const QModelIndexList& list);
    void semiAutoSolveLocalRemoteIssues(const QModelIndexList& list);

    //IgnoreConflicts
    void ignoreItems(const QModelIndexList& list);
    void ignoreAllSimilarIssues();
    void ignoreSymLinks();
    void showIgnoreItemsError(bool allFailed);

    //Fingerprint missing
    void fixFingerprint(const QModelIndexList& list);

    //MoveOrRename issue
    void fixMoveOrRenameCannotOccur(const QModelIndexList& indexes, MoveOrRenameIssueChosenSide side);

    bool issuesRequested() const;

    //Common strings methods
    static QString fixingIssuesString();
    static QString processingIssuesString();
    static QString issuesFixedString(StalledIssuesCreator::IssuesCount numberOfIssues);

signals:
    void stalledIssuesChanged();
    void stalledIssuesCountChanged();
    void stalledIssuesReceived();

    void uiBlocked();
    void uiUnblocked();

    void updateStalledIssuesOnReceiver(UpdateType type);

    void showRawInfoChanged();

    void updateLoadingMessage(std::shared_ptr<MessageInfo> message);

    void refreshFilter();

protected slots:
    void onGlobalSyncStateChanged(mega::MegaApi* api) override;
    void onNodesUpdate(mega::MegaApi*, mega::MegaNodeList* nodes) override;

private slots:
    void onStalledIssueUpdated(StalledIssue* issue);
    void onAsyncIssueSolvingFinished(StalledIssue* issue);
    void onProcessStalledIssues(ReceivedStalledIssues issuesReceived);
    void onSendEvent();

private:
    void showIssueExternallyChangedMessageBox();

    void appendCachedIssuesToModel(const StalledIssuesVariantList& list, StalledIssueFilterCriterion type);

    void removeRows(QModelIndexList& indexesToRemove);
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    void updateStalledIssuedByOrder();
    int getRowByStalledIssue(const std::shared_ptr<const StalledIssue> issue) const;
    int getRowByStalledIssue(const StalledIssue* issue) const;
    void reset();
    QModelIndex getSolveIssueIndex(const QModelIndex& index);
    void quitReceiverThread();

    StalledIssueVariant getIssueVariantByIssue(const StalledIssue* issue);

    bool checkIfUserStopSolving();
    void startSolvingIssues();

    void finishSolvingIssues(StalledIssuesCreator::IssuesCount count, bool sendMessage = true);
    void sendFinishSolvingMessage(StalledIssuesCreator::IssuesCount count, bool sendMessage = true);

    void sendFixingIssuesMessage(int issue, int totalIssues);

    struct SolveListInfo
    {
        SolveListInfo(const QModelIndexList& uIndexes, std::function<bool(int)> uSolveFunc)
            : indexes(uIndexes),
            solveFunc(uSolveFunc)
        {
            Q_ASSERT(solveFunc);
        }

        bool async = false;
        QModelIndexList indexes;
        std::function<bool(int)> solveFunc = nullptr;
        std::function<void ()> startFunc = nullptr;
        std::function<void (int, bool)> finishFunc = nullptr;
    };

    void solveListOfIssues(const SolveListInfo& info);
    bool issueSolvingFinished(const StalledIssue* issue);
    bool issueSolvingFinished(StalledIssue* issue, bool wasSuccessful);
    bool issueSolved(const StalledIssue* issue);
    bool issueFailed(const StalledIssue* issue);
    
    StalledIssuesModel(const StalledIssuesModel&) = delete;
    void operator=(const StalledIssuesModel&) = delete;
    
    QThread* mStalledIssuesThread;
    StalledIssuesReceiver* mStalledIssuesReceiver;
    std::atomic_bool mThreadFinished { false };
    mega::QTMegaRequestListener* mRequestListener;
    mega::QTMegaGlobalListener* mGlobalListener;
    mega::MegaApi* mMegaApi;
    std::atomic_bool mIssuesRequested {false};
    bool mIsStalled;
    bool mIsStalledChanged;
    StalledIssuesCreator::IssuesCount mReceivedIssuesStats;

    mutable QReadWriteLock mModelMutex;

    mutable StalledIssuesVariantList mStalledIssues;
    mutable StalledIssuesVariantList mSolvedStalledIssues;
    mutable StalledIssuesVariantList mFailedStalledIssues;
    mutable QHash<const StalledIssue*, int> mStalledIssuesByOrder;

    QHash<int, int> mCountByFilterCriterion;

    QTimer mEventTimer;
    bool mRawInfoVisible;

    std::atomic_bool mSolvingIssues {false};
    std::atomic_bool mIssuesSolved {false};
    std::atomic_bool mSolvingIssuesStopped {false};

    //SyncDisable for backups
    QList<std::shared_ptr<SyncSettings>> mSyncsToDisable;
    
    //Fix fingerprint
    QList<StalledIssueVariant> mFingerprintIssuesToFix;
    FingerprintMissingSolver mFingerprintIssuesSolver;
};

#endif // STALLEDISSUESMODEL_H
