#include "StalledIssuesModel.h"

#include "MegaApplication.h"
#include <StalledIssuesDelegateWidgetsCache.h>
#include <NameConflictStalledIssue.h>
#include <LocalOrRemoteUserMustChooseStalledIssue.h>
#include <AppStatsEvents.h>

#include "mega/types.h"

#include <QSortFilterProxyModel>

StalledIssuesReceiver::StalledIssuesReceiver(QObject *parent) : QObject(parent), mega::MegaRequestListener()
{
    qRegisterMetaType<StalledIssuesReceived>("StalledIssuesReceived");
}

void StalledIssuesReceiver::onRequestFinish(mega::MegaApi*, mega::MegaRequest *request, mega::MegaError*)
{
    if (request->getType() == ::mega::MegaRequest::TYPE_GET_SYNC_STALL_LIST)
    {
        QMutexLocker lock(&mCacheMutex);
        mCacheStalledIssues.clear();

        if (auto stalls = request->getMegaSyncStallList())
        {
            StalledIssuesVariantList solvableItems;

            auto totalSize(stalls->size());

            for (size_t i = 0; i < totalSize; ++i)
            {
                auto stall = stalls->get(i);
                std::shared_ptr<StalledIssueVariant> variant;

                if(stall->reason() == mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced)
                {
                    auto d = std::make_shared<NameConflictedStalledIssue>(stall);
                    variant = std::make_shared<StalledIssueVariant>(d);
                }
                else if(stall->reason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose
                        || stall->reason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose)
                {
                    auto d = std::make_shared<LocalOrRemoteUserMustChooseStalledIssue>(stall);
                    variant = std::make_shared<StalledIssueVariant>(d);
                }
                else
                {
                    auto d = std::make_shared<StalledIssue>(stall);
                    variant = std::make_shared<StalledIssueVariant>(d);
                }

                variant->getData()->fillIssue(stall);
                variant->getData()->endFillingIssue();

                if(mIsEventRequest)
                {
                    if(!variant->getData()->isSolvable())
                    {
                        QString eventMessage(QString::fromLatin1("Stalled issue received: Type %1").arg(QString::number(stall->reason())));
                        MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_STALLED_ISSUE_RECEIVED, eventMessage.toUtf8().constData(), false, nullptr);
                    }
                }
                else
                {
                    if(variant->getData()->isSolvable())
                    {
                        solvableItems.append(variant);
                    }
                    else
                    {
                        mCacheStalledIssues.stalledIssues.append(variant);
                    }
                }
            }

            auto solvableTotalIssues(solvableItems.size());
            auto counter(1);
            foreach(auto solvableIssue, solvableItems)
            {
                if(Preferences::instance()->stalledIssuesMode() == Preferences::StalledIssuesModeType::Smart)
                {
                    emit solvingIssues(counter, solvableTotalIssues);
                    solvableIssue->getData()->autoSolveIssue();
                }

                if(!solvableIssue->getData()->isSolved())
                {
                    mCacheStalledIssues.stalledIssues.append(solvableIssue);
                }

                counter++;
            }
        }

        if(mIsEventRequest)
        {
            mIsEventRequest = false;
        }
        else
        {
            emit stalledIssuesReady(mCacheStalledIssues);
        }
    }
}

void StalledIssuesReceiver::onSetIsEventRequest()
{
    mIsEventRequest = true;
}

const int StalledIssuesModel::ADAPTATIVE_HEIGHT_ROLE = Qt::UserRole;
const int EVENT_REQUEST_DELAY = 600000; /*10 minutes*/

StalledIssuesModel::StalledIssuesModel(QObject *parent)
    : QAbstractItemModel(parent),
    mMegaApi (MegaSyncApp->getMegaApi()),
    mUpdateWhenGlobalStateChanges(false),
    mRawInfoVisible(false)
{
    mStalledIssuesThread = new QThread();
    mStalledIssuedReceiver = new StalledIssuesReceiver();

    mRequestListener = new mega::QTMegaRequestListener(mMegaApi, mStalledIssuedReceiver);
    mStalledIssuedReceiver->moveToThread(mStalledIssuesThread);
    mRequestListener->moveToThread(mStalledIssuesThread);
    mMegaApi->addRequestListener(mRequestListener);

    mGlobalListener = new mega::QTMegaGlobalListener(mMegaApi,this);
    mMegaApi->addGlobalListener(mGlobalListener);

    connect(mStalledIssuedReceiver, &StalledIssuesReceiver::solvingIssues, this, [this](int issueCounter, int totalIssues)
    {
        LoadingSceneMessageHandler::MessageInfo info;
        info.message = tr("Solving issues");
        info.buttonType = LoadingSceneMessageHandler::MessageInfo::ButtonType::Stop;
        info.count = issueCounter;
        info.total = totalIssues;
        emit updateLoadingMessage(info);
    }, Qt::QueuedConnection);

    mStalledIssuesThread->start();

    connect(this, &StalledIssuesModel::setIsEventRequest,
            mStalledIssuedReceiver, &StalledIssuesReceiver::onSetIsEventRequest,
            Qt::QueuedConnection);

    connect(mStalledIssuedReceiver, &StalledIssuesReceiver::stalledIssuesReady,
            this, &StalledIssuesModel::onProcessStalledIssues,
            Qt::QueuedConnection);

    connect(&mEventTimer,&QTimer::timeout, this, &StalledIssuesModel::onSendEvent);
    mEventTimer.setSingleShot(true);
}

StalledIssuesModel::~StalledIssuesModel()
{
    mMegaApi->removeRequestListener(mRequestListener);
    mMegaApi->removeGlobalListener(mGlobalListener);

    mThreadFinished = true;

    mStalledIssuesThread->quit();
    mStalledIssuedReceiver->deleteLater();

    mRequestListener->deleteLater();
}

void StalledIssuesModel::onProcessStalledIssues(StalledIssuesReceiver::StalledIssuesReceived issuesReceived)
{
    reset();

    if(!issuesReceived.stalledIssues.isEmpty() && !mEventTimer.isActive())
    {
        mEventTimer.start(EVENT_REQUEST_DELAY);
    }

    Utilities::queueFunctionInObjectThread(mStalledIssuedReceiver, [this, issuesReceived]()
    {
        mModelMutex.lock();

        blockSignals(true);

        auto totalRows = rowCount(QModelIndex());
        auto rowsToBeInserted(static_cast<int>(issuesReceived.stalledIssues.size()));

        if(rowsToBeInserted > 0)
        {
            beginInsertRows(QModelIndex(), totalRows, totalRows + rowsToBeInserted - 1);

            for (auto it = issuesReceived.stalledIssues.begin(); it != issuesReceived.stalledIssues.end();)
            {
                if(mThreadFinished)
                {
                    return;
                }

                std::shared_ptr<StalledIssueVariant> issue(*it);
                mStalledIssues.append(issue);
                mStalledIssuesByOrder.insert(issue.get(), rowCount(QModelIndex()) - 1);
                mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason((*it)->consultData()->getReason()))]++;

                it++;
            }

            endInsertRows();
        }

        totalRows = rowCount(QModelIndex());
        rowsToBeInserted = static_cast<int>(mSolvedStalledIssues.size());

        if(rowsToBeInserted > 0)
        {
             beginInsertRows(QModelIndex(), totalRows, totalRows + rowsToBeInserted - 1);

             for (auto it = mSolvedStalledIssues.begin(); it != mSolvedStalledIssues.end();)
             {
                 if(mThreadFinished)
                 {
                     return;
                 }

                 std::shared_ptr<StalledIssueVariant> issue(*it);
                 mStalledIssues.append(issue);
                 mStalledIssuesByOrder.insert(issue.get(), rowCount(QModelIndex()) - 1);
                 mCountByFilterCriterion[static_cast<int>(StalledIssueFilterCriterion::SOLVED_CONFLICTS)]++;

                 it++;
             }

             endInsertRows();
        }

        blockSignals(false);
        mModelMutex.unlock();

        emit stalledIssuesCountChanged();
        emit stalledIssuesChanged();
    });
}

void StalledIssuesModel::onSendEvent()
{
    if(Preferences::instance()->stalledIssuesEventLastDate() != QDate::currentDate())
    {
        Preferences::instance()->updateStalledIssuesEventLastDate();

        emit setIsEventRequest();
        mMegaApi->getMegaSyncStallList(nullptr);
    }
}

void StalledIssuesModel::updateStalledIssues()
{
    blockUi();

    if(!mSolvingIssues)
    {
        mMegaApi->getMegaSyncStallList(nullptr);
    }
}

void StalledIssuesModel::updateStalledIssuesWhenReady()
{
    mUpdateWhenGlobalStateChanges = true;
}

void StalledIssuesModel::onGlobalSyncStateChanged(mega::MegaApi*)
{
}

void StalledIssuesModel::onNodesUpdate(mega::MegaApi*, mega::MegaNodeList *nodes)
{
    if(nodes)
    {
        for (int i = 0; i < nodes->size(); i++)
        {
            mega::MegaNode *node = nodes->get(i);
            if (node->getChanges() & mega::MegaNode::CHANGE_TYPE_PARENT)
            {
                std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
                if(parentNode && parentNode->getType() == mega::MegaNode::TYPE_FILE)
                {
                    for(int row = 0; row < rowCount(QModelIndex()); ++row)
                    {
                        auto item = mStalledIssues.at(row);
                        if(item->getData()->containsHandle(node->getHandle()))
                        {
                            auto parentFound(false);
                            while (!parentFound)
                            {
                                auto currentParentHandle(parentNode->getHandle());
                                parentNode.reset(MegaSyncApp->getMegaApi()->getNodeByHandle(parentNode->getParentHandle()));
                                if(!parentNode || parentNode->getType() != mega::MegaNode::TYPE_FILE)
                                {
                                    item->getData()->updateHandle(currentParentHandle);
                                    parentFound = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

Qt::DropActions StalledIssuesModel::supportedDropActions() const
{
    return Qt::IgnoreAction;
}

bool StalledIssuesModel::hasChildren(const QModelIndex &parent) const
{
    auto stalledIssueItem = static_cast<StalledIssue*>(parent.internalPointer());
    if (stalledIssueItem)
    {
        return false;
    }

    return true;
}

int StalledIssuesModel::rowCount(const QModelIndex &parent) const
{
   if(!parent.isValid())
   {
       return mStalledIssues.size();
   }
   else
   {
       return 1;
   }
}

int StalledIssuesModel::columnCount(const QModelIndex &) const
{
   return 1;
}

QVariant StalledIssuesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        auto stalledIssueItem = static_cast<StalledIssueVariant*>(index.internalPointer());
        if (stalledIssueItem)
        {
            return QVariant::fromValue((*stalledIssueItem));
        }
        else
        {
            return QVariant::fromValue(StalledIssueVariant(*(mStalledIssues.at(index.row()).get())));
        }
    }
    else if(role == ADAPTATIVE_HEIGHT_ROLE)
    {
        auto issue = mStalledIssues.at(index.row());
        return StalledIssuesDelegateWidgetsCache::adaptativeHeight(issue->getData()->getReason());
    }

    return QVariant();
}

QModelIndex StalledIssuesModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }

    auto stalledIssueItem = static_cast<StalledIssueVariant*>(index.internalPointer());
    if (!stalledIssueItem)
    {
        return QModelIndex();
    }

    auto row = mStalledIssuesByOrder.value(&(*stalledIssueItem),-1);
    if(row >= 0)
    {
        return createIndex(row, 0);
    }

    return QModelIndex();
}

QModelIndex StalledIssuesModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid() && mStalledIssues.size() > parent.row())
    {
        auto& stalledIssue = mStalledIssues[parent.row()];
        return createIndex(0, 0, stalledIssue.get());
    }
    else
    {
        return (row < rowCount(QModelIndex())) ?  createIndex(row, column) : QModelIndex();
    }
}

Qt::ItemFlags StalledIssuesModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

bool StalledIssuesModel::isEmpty() const
{
    return !MegaSyncApp->getMegaApi()->isSyncStalled();
}

void StalledIssuesModel::finishStalledIssues(const QModelIndexList &indexes)
{
    auto indexesToFinish(indexes);
    removeRows(indexesToFinish);
}

void StalledIssuesModel::removeRows(QModelIndexList &indexesToRemove)
{
    std::sort(indexesToRemove.begin(), indexesToRemove.end(),[](QModelIndex check1, QModelIndex check2){
        return check1.row() > check2.row();
    });

    // First clear finished transfers (remove rows), then cancel the others.
    // This way, there is no risk of messing up the rows order with cancel requests.
    int count (0);
    int row (indexesToRemove.last().row());
    for (auto index : indexesToRemove)
    {
        // Init row with row of first tag
        if (count == 0)
        {
            row = index.row();
        }

        // If rows are non-contiguous, flush and start from item
        if (row != index.row())
        {
            removeRows(row + 1, count, QModelIndex());
            count = 0;
            row = index.row();
        }

        // We have at least one row
        count++;
        row--;
    }
    // Flush pooled rows (start at row + 1).
    // This happens when the last item processed is in a finished state.
    if (count > 0)
    {
        removeRows(row + 1, count, QModelIndex());
    }

    updateStalledIssuedByOrder();
}

bool StalledIssuesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent == QModelIndex() && count > 0 && row >= 0)
    {
        beginRemoveRows(parent, row, row + count - 1);

        for (auto i (0); i < count; ++i)
        {
            mStalledIssues.removeAt(i);
        }

        endRemoveRows();

        return true;
    }
    else
    {
        return false;
    }
}

void StalledIssuesModel::updateStalledIssuedByOrder()
{
    mStalledIssuesByOrder.clear();
    mCountByFilterCriterion.clear();

    //Recalculate rest of items
    for(int row = 0; row < rowCount(QModelIndex()); ++row)
    {
        auto item = mStalledIssues.at(row);
        mStalledIssuesByOrder.insert(item.get(), row);

        mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason(item->consultData()->getReason()))]++;
    }

    emit stalledIssuesCountChanged();
}

void StalledIssuesModel::lockModelMutex(bool lock)
{
    if (lock)
    {
        mModelMutex.lock();
    }
    else
    {
        mModelMutex.unlock();
    }
}

void StalledIssuesModel::blockUi()
{
    emit uiBlocked();
}

void StalledIssuesModel::unBlockUi()
{
    emit uiUnblocked();
}

void StalledIssuesModel::updateIndex(const QModelIndex &index)
{
    emit dataChanged(index, index);
}

QModelIndexList StalledIssuesModel::getIssuesByReason(QList<mega::MegaSyncStall::SyncStallReason> reasons)
{
    auto checkerFunc = [reasons](const std::shared_ptr<const StalledIssue> check) -> bool{
        return reasons.contains(check->getReason());
    };

    return getIssues(checkerFunc);
}

QModelIndexList StalledIssuesModel::getIssues(std::function<bool (const std::shared_ptr<const StalledIssue>)> checker)
{
    QModelIndexList list;

    for(int row = 0; row < rowCount(QModelIndex()); ++row)
    {
        auto issue(mStalledIssues.at(row));
        if(issue && !issue->consultData()->isSolved() && checker(issue->consultData()))
        {
            list.append(index(row,0));
        }
    }

    return list;
}

void StalledIssuesModel::showRawInfo(bool state)
{
    mRawInfoVisible = state;
    emit showRawInfoChanged();
}

bool StalledIssuesModel::isRawInfoVisible() const
{
    return mRawInfoVisible;
}

void StalledIssuesModel::reset()
{
    beginResetModel();

    mStalledIssues.clear();
    mStalledIssuesByOrder.clear();
    mCountByFilterCriterion.clear();

    endResetModel();

    emit stalledIssuesCountChanged();
}

QModelIndex StalledIssuesModel::getSolveIssueIndex(const QModelIndex &index)
{
    auto indexParent(index.parent());
    return indexParent.isValid() ? indexParent : index;
}

void StalledIssuesModel::quitReceiverThread()
{
    mStalledIssuesThread->quit();
}

bool StalledIssuesModel::checkIfUserStopSolving()
{
    if(mThreadFinished || mSolvingIssuesFinished)
    {
        mSolvingIssuesFinished = false;
        return true;
    }

    return false;
}

void StalledIssuesModel::stopSolvingIssues()
{
    if(mIssuesSolved)
    {
        mIssuesSolved = false;
        refreshFilter();
    }
    else
    {
        mSolvingIssuesFinished = true;
    }
}

void StalledIssuesModel::startSolvingIssues()
{
    mSolvingIssues = true;
    blockUi();
}

void StalledIssuesModel::finishSolvingIssues(int issuesFixed)
{
    mSolvingIssues = false;
    mIssuesSolved = true;

    LoadingSceneMessageHandler::MessageInfo info;
    info.message = tr("%n issues fixed", "", issuesFixed);
    info.buttonType = LoadingSceneMessageHandler::MessageInfo::ButtonType::Ok;
    emit updateLoadingMessage(info);

    emit stalledIssuesCountChanged();
}

void StalledIssuesModel::sendFixingIssuesMessage(int issue, int totalIssues)
{
    LoadingSceneMessageHandler::MessageInfo info;
    info.message = tr("Fixing issues");
    info.buttonType = LoadingSceneMessageHandler::MessageInfo::ButtonType::Stop;
    info.count = issue;
    info.total = totalIssues;
    emit updateLoadingMessage(info);
}

int StalledIssuesModel::getCountByFilterCriterion(StalledIssueFilterCriterion criterion)
{
    if(criterion == StalledIssueFilterCriterion::ALL_ISSUES)
    {
        return (rowCount(QModelIndex()) - mCountByFilterCriterion.value(static_cast<int>(StalledIssueFilterCriterion::SOLVED_CONFLICTS),0));
    }
    else
    {
        return mCountByFilterCriterion.value(static_cast<int>(criterion),0);
    }
}

//METHODS TO SOLVE ISSUES
void StalledIssuesModel::issueSolved(std::shared_ptr<StalledIssueVariant> issue)
{
    if(issue->consultData()->isSolved())
    {
        mSolvedStalledIssues.append(issue);
        mCountByFilterCriterion[static_cast<int>(StalledIssueFilterCriterion::SOLVED_CONFLICTS)]++;
        auto& counter = mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason(issue->consultData()->getReason()))];
        if(counter > 0)
        {
            counter--;
        }
    }
}

void StalledIssuesModel::solveAllIssues()
{
    startSolvingIssues();
    Utilities::queueFunctionInObjectThread(mStalledIssuedReceiver, [this]()
    {
        auto totalRows(rowCount(QModelIndex()));
        auto issuesFixed(0);
        for(int row = 0; row < totalRows; ++row)
        {
            if(checkIfUserStopSolving())
            {
                break;
            }

            auto item = mStalledIssues.at(row);
            if(item->consultData()->isSolvable())
            {
                issuesFixed++;
                sendFixingIssuesMessage(issuesFixed, totalRows);
                item->getData()->autoSolveIssue();
                issueSolved(item);
            }
         }

        finishSolvingIssues(issuesFixed);
    });
}

void StalledIssuesModel::chooseSideManually(bool remote, const QModelIndexList &list)
{
    auto resolveIssue = [this, remote](int row)
    {
        auto item = mStalledIssues.at(row);
        if(item->consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose ||
           item->consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose)
        {
            if(auto issue = item->convert<LocalOrRemoteUserMustChooseStalledIssue>())
            {
                remote ? issue->chooseRemoteSide() : issue->chooseLocalSide();
                issueSolved(item);
                if(item->consultData()->isSolved())
                {
                    MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_LOCALREMOTE_SOLVED_MANUALLY,
                                                         "Local/Remote issue solved manually", false, nullptr);
                }
            }
        }
    };

    startSolvingIssues();
    Utilities::queueFunctionInObjectThread(mStalledIssuedReceiver, [this, resolveIssue, list]()
    {
        auto issuesFixed(0);
        auto totalRows(list.size());
        foreach(auto index, list)
        {
            if(checkIfUserStopSolving())
            {
                break;
            }

            issuesFixed++;
            sendFixingIssuesMessage(issuesFixed, totalRows);
            resolveIssue(index.row());
        }

        finishSolvingIssues(issuesFixed);
    });
}

void StalledIssuesModel::semiAutoSolveLocalRemoteIssues(const QModelIndexList &list)
{
    auto resolveIssue = [this](int row)
    {
        auto issue(mStalledIssues.at(row));
        auto localRemoteIssue = mStalledIssues.at(row)->convert<LocalOrRemoteUserMustChooseStalledIssue>();
        if(localRemoteIssue)
        {
            localRemoteIssue->chooseLastMTimeSide();
            issueSolved(issue);

            if(localRemoteIssue->isSolved())
            {
                MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_LOCALREMOTE_SOLVED_SEMI_AUTOMATICALLY,
                                                     "Local/Remote issue solved semi-automatically", false, nullptr);
            }
        }
    };

    startSolvingIssues();
    Utilities::queueFunctionInObjectThread(mStalledIssuedReceiver, [this, list, resolveIssue]()
    {
        auto issuesFixed(0);
        auto totalRows(list.size());
        foreach(auto index, list)
        {
            if(checkIfUserStopSolving())
            {
                break;
            }

            issuesFixed++;
            sendFixingIssuesMessage(issuesFixed, totalRows);

            auto potentialIndex = getSolveIssueIndex(index);
            resolveIssue(potentialIndex.row());
        }

        finishSolvingIssues(issuesFixed);
    });
}

void StalledIssuesModel::ignoreItems(const QModelIndexList &list)
{
    ignoredItems.clear();

    auto resolveIssue = [this](int row)
    {
        auto item = mStalledIssues.at(row);
        if(item->getData()->canBeIgnored())
        {
            auto ignoredFiles = item->getData()->getIgnoredFiles();

            foreach(auto file, ignoredFiles)
            {
                if(!ignoredItems.contains(file))
                {
                    ignoredItems.append(file);
                    mUtilities.ignoreFile(file);
                }
            }

            item->getData()->setIsSolved();
            issueSolved(item);
            MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_IGNORE_SOLVED_MANUALLY,
                                                 "Issue ignored manually", false, nullptr);
        }
    };

    startSolvingIssues();
    Utilities::queueFunctionInObjectThread(mStalledIssuedReceiver, [this, resolveIssue, list]()
    {
        auto issuesFixed(0);
        auto totalRows(list.size());
        foreach(auto index, list)
        {
            if(checkIfUserStopSolving())
            {
                break;
            }

            issuesFixed++;
            sendFixingIssuesMessage(issuesFixed, totalRows);
            resolveIssue(index.row());
        }

        finishSolvingIssues(issuesFixed);
    });
}

void StalledIssuesModel::ignoreSymLinks(const QModelIndex& index)
{
    auto item = mStalledIssues.at(index.row());
    if(item->getData()->canBeIgnored())
    {
        mUtilities.ignoreSymLinks(item->getData()->consultLocalData()->getNativeFilePath());
        MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_IGNORE_SOLVED_MANUALLY,
                                             "Issue ignored manually", false, nullptr);

        startSolvingIssues();
        Utilities::queueFunctionInObjectThread(mStalledIssuedReceiver, [this]()
        {
            auto issuesFixed(0);
            auto totalRows(rowCount(QModelIndex()));
            for(int row = 0; row < totalRows; ++row)
            {
                if(checkIfUserStopSolving())
                {
                    break;
                }

                auto item = mStalledIssues.at(row);
                if(item->getData()->isSymLink())
                {
                    item->getData()->setIsSolved();
                }

                issuesFixed++;
                sendFixingIssuesMessage(issuesFixed, totalRows);
                issueSolved(item);
            }

            finishSolvingIssues(issuesFixed);
        });
    }
}

void StalledIssuesModel::semiAutoSolveNameConflictIssues(const QModelIndexList &list, int option)
{
    auto resolveIssue = [this, option](int row)
    {
        auto item = mStalledIssues.at(row);
        if(item->consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced)
        {
            if(auto nameConflict = item->convert<NameConflictedStalledIssue>())
            {
                nameConflict->semiAutoSolveIssue(option);
                issueSolved(item);
                if(item->consultData()->isSolved())
                {
                    MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_NAMECONFLICT_SOLVED_SEMI_AUTOMATICALLY,
                                                         "Name conflict issue solved semi-automatically", false, nullptr);
                }
            }
        }
    };

    startSolvingIssues();
    Utilities::queueFunctionInObjectThread(mStalledIssuedReceiver, [this, list, option, resolveIssue]()
    {
        auto issuesFixed(0);
        auto totalRows(list.size());

        foreach(auto index, list)
        {
            if(checkIfUserStopSolving())
            {
                break;
            }

            issuesFixed++;
            sendFixingIssuesMessage(issuesFixed, totalRows);
            auto potentialIndex = getSolveIssueIndex(index);
            resolveIssue(potentialIndex.row());
        }

        finishSolvingIssues(issuesFixed);
    });
}

bool StalledIssuesModel::solveLocalConflictedNameByRemove(int conflictIndex, const QModelIndex &index)
{
    auto result(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue->convert<NameConflictedStalledIssue>())
    {
        result = nameConflict->solveLocalConflictedNameByRemove(conflictIndex);
        issueSolved(issue);

        if(nameConflict->isSolved())
        {
            MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_NAMECONFLICT_SOLVED_MANUALLY,
                                                 "Name conflict issue solved manually", false, nullptr);
            finishConflictManually();
        }

    }

    return result;
}

bool StalledIssuesModel::solveLocalConflictedNameByRename(const QString &renameTo, int conflictIndex, const QModelIndex &index)
{
    auto result(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue->convert<NameConflictedStalledIssue>())
    {
        result = nameConflict->solveLocalConflictedNameByRename(conflictIndex, renameTo);
        issueSolved(issue);

        if(nameConflict->isSolved())
        {
            MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_NAMECONFLICT_SOLVED_MANUALLY,
                                                 "Name conflict issue solved manually", false, nullptr);
            finishConflictManually();
        }

    }

    return result;
}

bool StalledIssuesModel::solveCloudConflictedNameByRemove(int conflictIndex, const QModelIndex &index)
{
    auto result(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue->convert<NameConflictedStalledIssue>())
    {
        result = nameConflict-> solveCloudConflictedNameByRemove(conflictIndex);
        issueSolved(issue);

        if(nameConflict->isSolved())
        {
            MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_NAMECONFLICT_SOLVED_MANUALLY,
                                                 "Name conflict issue solved manually", false, nullptr);
            finishConflictManually();
        }

    }

    return result;
}

bool StalledIssuesModel::solveCloudConflictedNameByRename(const QString& renameTo, int conflictIndex, const QModelIndex &index)
{
    auto result(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue->convert<NameConflictedStalledIssue>())
    {
        result = nameConflict->solveCloudConflictedNameByRename(conflictIndex, renameTo);
        issueSolved(issue);

        if(nameConflict->isSolved())
        {
            MegaSyncApp->getMegaApi()->sendEvent(AppStatsEvents::EVENT_SI_NAMECONFLICT_SOLVED_MANUALLY,
                                                 "Name conflict issue solved manually", false, nullptr);
            finishConflictManually();
        }
    }

    return result;
}

void StalledIssuesModel::finishConflictManually()
{
    blockUi();
    //Only was issue was fixed
    finishSolvingIssues(1);
}
