#include "StalledIssuesModel.h"

#include "DialogOpener.h"
#include "FolderMatchedAgainstFileIssue.h"
#include "IgnoredStalledIssue.h"
#include "LocalOrRemoteUserMustChooseStalledIssue.h"
#include "MegaApplication.h"
#include "MegaIgnoreManager.h"
#include "MoveOrRenameCannotOccurIssue.h"
#include "NameConflictStalledIssue.h"
#include "QMegaMessageBox.h"
#include "QSortFilterProxyModel"
#include "StalledIssuesDelegateWidgetsCache.h"
#include "StalledIssuesDialog.h"
#include "StatsEventHandler.h"
#include "SyncController.h"

StalledIssuesReceiver::StalledIssuesReceiver(QObject* parent) : QObject(parent), mega::MegaRequestListener()
{
    connect(&mIssueCreator, &StalledIssuesCreator::solvingIssues, this, &StalledIssuesReceiver::solvingIssues);
    connect(&mIssueCreator, &StalledIssuesCreator::solvingIssuesFinished, this, &StalledIssuesReceiver::solvingIssuesFinished);
}

bool StalledIssuesReceiver::multiStepIssueSolveActive() const
{
    return mIssueCreator.multiStepIssueSolveActive();
}

void StalledIssuesReceiver::onUpdateStalledISsues(UpdateType type)
{
    if(mUpdateRequests == 0)
    {
        mUpdateRequests++;
        mUpdateType = type;
        MegaSyncApp->getMegaApi()->getMegaSyncStallList(nullptr);
    }
}

void StalledIssuesReceiver::onRequestFinish(mega::MegaApi*, mega::MegaRequest* request, mega::MegaError*)
{
    if (request->getType() == ::mega::MegaRequest::TYPE_GET_SYNC_STALL_LIST)
    {
        {
            QMutexLocker lock(&mCacheMutex);
            mStalledIssues.clear();
            IgnoredStalledIssue::clearIgnoredSyncs();

            mIssueCreator.createIssues(request->getMegaSyncStallList(), mUpdateType);
            mStalledIssues = mIssueCreator.getStalledIssues();
            mUpdateRequests = 0;
        }

        StalledIssuesBySyncFilter::resetFilter();

        if (mUpdateType != UpdateType::EVENT)
        {
            emit stalledIssuesReady(mStalledIssues, mUpdateType);
        }
    }
}

const int StalledIssuesModel::ADAPTATIVE_HEIGHT_ROLE = Qt::UserRole;
const int EVENT_REQUEST_DELAY = 600000; /*10 minutes*/
const int UPDATE_ISSUES_INTERVAL = 5000; /*5 seconds*/
const int UPDATE_ISSUES_MAX_INTERVAL = 300000; /*5 minutes*/
const int UPDATE_ISSUES_INTERVAL_DELAY_CONSTANT = 2;
const int MAX_EMPTY_STALLED_LIST_ALLOWED = 5;

StalledIssuesModel::StalledIssuesModel():
    QAbstractItemModel(),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mIsStalled(false),
    mIsStalledChanged(false),
    mReceivedEmptyStalledIssuesCounter(0),
    mRawInfoVisible(false)
{
    mStalledIssuesThread = new QThread();
    mStalledIssuesReceiver = new StalledIssuesReceiver();

    mRequestListener =
        std::make_unique<mega::QTMegaRequestListener>(mMegaApi, mStalledIssuesReceiver);
    mStalledIssuesReceiver->moveToThread(mStalledIssuesThread);
    mRequestListener->moveToThread(mStalledIssuesThread);
    mMegaApi->addRequestListener(mRequestListener.get());

    mGlobalListener = std::make_unique<mega::QTMegaGlobalListener>(mMegaApi, this);
    mMegaApi->addGlobalListener(mGlobalListener.get());

    connect(
        mStalledIssuesReceiver,
        &StalledIssuesReceiver::solvingIssues,
        this,
        [this](StalledIssuesCreator::IssuesCount count)
        {
            blockUi();

            auto info = std::make_shared<MessageInfo>();
            info->message = processingIssuesString();
            info->buttonType = MessageInfo::ButtonType::NONE;
            info->count = count.currentIssueBeingSolved;
            info->total = count.totalIssues;
            emit updateLoadingMessage(info);
        },
        Qt::QueuedConnection);

    connect(mStalledIssuesReceiver, &StalledIssuesReceiver::solvingIssuesFinished, this, [this](StalledIssuesCreator::IssuesCount count)
        {
            sendFinishSolvingMessage(count, true);
        }, Qt::QueuedConnection);

    mStalledIssuesThread->start();

    connect(mStalledIssuesReceiver, &StalledIssuesReceiver::stalledIssuesReady,
            this, &StalledIssuesModel::onProcessStalledIssues,
            Qt::QueuedConnection);

    connect(this, &StalledIssuesModel::updateStalledIssuesOnReceiver,
        mStalledIssuesReceiver, &StalledIssuesReceiver::onUpdateStalledISsues,
        Qt::QueuedConnection);

    connect(&mEventTimer,&QTimer::timeout, this, &StalledIssuesModel::onSendEvent);
    mEventTimer.setSingleShot(true);

    connect(&mUpdateIssuesTimer, &QTimer::timeout, this, &StalledIssuesModel::needsUpdate);

    mStopUpdateIssuesTimer.setSingleShot(true);
    connect(&mStopUpdateIssuesTimer, &QTimer::timeout, this, [this]() {
        mUpdateIssuesTimer.stop();
    });
}

StalledIssuesModel::~StalledIssuesModel()
{
    mThreadFinished = true;
    mStalledIssuesThread->quit();
    mStalledIssuesThread->wait();
    mStalledIssuesReceiver->deleteLater();
}

bool StalledIssuesModel::issuesRequested() const
{
    return mIssuesRequested.load();
}

QString StalledIssuesModel::fixingIssuesString()
{
    return tr("Fixing issues");
}

QString StalledIssuesModel::processingIssuesString()
{
    return tr("Processing issues");
}

QString StalledIssuesModel::issuesFixedString(StalledIssuesCreator::IssuesCount numberOfIssues)
{
    QString message;

    if(numberOfIssues.issuesFailed == 0 && numberOfIssues.issuesFixed > 0)
    {
        message = tr("%n issues fixed", "", numberOfIssues.issuesFixed);
    }
    else if(numberOfIssues.issuesFailed > 0 && numberOfIssues.issuesFixed == 0)
    {
        message = tr("%n issues failed", "", numberOfIssues.issuesFailed);
    }
    else if(numberOfIssues.issuesFailed > 0 && numberOfIssues.issuesFixed == 1)
    {
        message = tr("1 issue fixed and %n issues failed", "", numberOfIssues.issuesFailed);
    }
    else if(numberOfIssues.issuesFailed == 1 && numberOfIssues.issuesFixed > 0)
    {
        message = tr("%n issues fixed and 1 issue failed", "", numberOfIssues.issuesFixed);
    }
    else
    {
        QString successItems = tr("%n issues fixed", "", numberOfIssues.issuesFixed);
        message = tr("%1 and %n issues failed.", "", numberOfIssues.issuesFailed).arg(successItems);
    }

    return message;
}

void StalledIssuesModel::startUpdateIssuesTimer(int interval)
{
    mUpdateIssuesTimer.start(interval);
    mStopUpdateIssuesTimer.stop();
}

void StalledIssuesModel::stopUpdateIssuesTimer()
{
    // Don´t stop immediately the updateIssuesTimer in case we receive a new mIsStalled == true
    mStopUpdateIssuesTimer.start(UPDATE_ISSUES_INTERVAL);
}

void StalledIssuesModel::onGlobalSyncStateChanged(mega::MegaApi* api)
{
    auto isSyncStalled(api->isSyncStalled());
    auto isSyncStalledChanged(api->isSyncStalledChanged());

    // Start/Stop timer
    if (mIsStalled != isSyncStalled || isSyncStalledChanged)
    {
        mIsStalled = isSyncStalled;
        mIsStalledChanged = isSyncStalledChanged;

        mIsStalled ? startUpdateIssuesTimer(UPDATE_ISSUES_INTERVAL) : stopUpdateIssuesTimer();
    }
}

void StalledIssuesModel::needsUpdate()
{
    mIsStalled = MegaSyncApp->getMegaApi()->isSyncStalled();
    if (MegaSyncApp->getMegaApi()->isSyncStalledChanged())
    {
        mIsStalledChanged = true;
    }

    if (mIsStalled && mIsStalledChanged)
    {
        startUpdateIssuesTimer(UPDATE_ISSUES_INTERVAL);
    }
    else if (!mIsStalled)
    {
        stopUpdateIssuesTimer();
    }

    if (mIsStalled)
    {
        if (mStalledIssues.size() == mSolvedStalledIssues.size())
        {
            updateActiveStalledIssues();
        }
        else
        {
            updateStalledIssuesForAutoSolve();
        }
    }
}

void StalledIssuesModel::onProcessStalledIssues(ReceivedStalledIssues issuesReceived,
                                                UpdateType updateType)
{
    if(!issuesReceived.isEmpty() && !mEventTimer.isActive())
    {
        mEventTimer.start(EVENT_REQUEST_DELAY);
    }

    mIsStalledChanged = MegaSyncApp->getMegaApi()->isSyncStalledChanged();

    auto updateTimer = [this](bool reset) {
        Utilities::queueFunctionInAppThread([this, reset]() {
            if (mUpdateIssuesTimer.isActive())
            {
                if (reset)
                {
                    startUpdateIssuesTimer(UPDATE_ISSUES_INTERVAL);
                }
                else
                {
                    auto newInterval =
                        mUpdateIssuesTimer.interval() * UPDATE_ISSUES_INTERVAL_DELAY_CONSTANT;
                    if (newInterval > UPDATE_ISSUES_MAX_INTERVAL)
                    {
                        newInterval = UPDATE_ISSUES_MAX_INTERVAL;
                    }

                    startUpdateIssuesTimer(newInterval);
                }
            }
        });
    };

    if (!issuesReceived.isEmpty())
    {
        Utilities::queueFunctionInObjectThread(
            mStalledIssuesReceiver,
            [this, issuesReceived, updateType, updateTimer]() mutable {
                if (updateType == UpdateType::UI)
                {
                    reset();
                }

                checkActiveIssues(issuesReceived.activeStalledIssues());
                checkAutoSolvedIssues(issuesReceived.autoSolvedStalledIssues());
                checkFailedAutoSolvedIssues(issuesReceived.failedAutoSolvedStalledIssues());

                if (updateType == UpdateType::AUTO_SOLVE && issuesReceived.isEmpty())
                {
                    updateTimer(false);
                    return;
                }

                updateTimer(true);

                QLatin1String message("Stalled Issues list received. Size: %1");
                mega::MegaApi::log(
                    mega::MegaApi::LOG_LEVEL_DEBUG,
                    message.arg(QString::number(issuesReceived.size())).toStdString().c_str());

                blockSignals(true);
                mModelMutex.lockForWrite();

                appendCachedIssuesToModel(issuesReceived.activeStalledIssues(),
                                          StalledIssueFilterCriterion::ALL_ISSUES);

                appendCachedIssuesToModel(issuesReceived.autoSolvedStalledIssues(),
                                          StalledIssueFilterCriterion::SOLVED_CONFLICTS);

                appendCachedIssuesToModel(issuesReceived.failedAutoSolvedStalledIssues(),
                                          StalledIssueFilterCriterion::FAILED_CONFLICTS);

                mModelMutex.unlock();
                blockSignals(false);

                emit stalledIssuesCountChanged();
                emit stalledIssuesChanged();

                if (updateType == UpdateType::UI)
                {
                    mIssuesRequested = false;
                    emit stalledIssuesReceived();
                }
                else
                {
                    emit refreshFilter();
                }
            });

        mReceivedEmptyStalledIssuesCounter = 0;
    }
    else
    {
        if (updateType == UpdateType::UI)
        {
            // No issues, no items on the model, reset
            reset();
            setIssuesRequested(false);
            emit stalledIssuesChanged();
            emit refreshFilter();
        }

        updateTimer(false);

        // Error case, corner case
        if (mIsStalled && !mIsStalledChanged)
        {
            mReceivedEmptyStalledIssuesCounter++;

            if (mReceivedEmptyStalledIssuesCounter == MAX_EMPTY_STALLED_LIST_ALLOWED)
            {
                mIsStalled = false;
                mIsStalledChanged = false;
                mReceivedEmptyStalledIssuesCounter = 0;
                stopUpdateIssuesTimer();

                mega::MegaApi::log(
                    mega::MegaApi::LOG_LEVEL_ERROR,
                    "Empty list received 5 times with IsSyncStalled flag set to true");
            }
        }
    }
}

void StalledIssuesModel::checkActiveIssues(StalledIssuesVariantList& list)
{
    checkIssues(list, [](const StalledIssue* issue) {
        return issue->isUnsolved() || issue->isBeingSolved();
    });
}

void StalledIssuesModel::checkAutoSolvedIssues(StalledIssuesVariantList& list)
{
    checkIssues(list, [](const StalledIssue* issue) {
        return issue->isSolved();
    });
}

void StalledIssuesModel::checkFailedAutoSolvedIssues(StalledIssuesVariantList& list)
{
    checkIssues(list, [](const StalledIssue* issue) {
        return issue->isFailed();
    });
}

void StalledIssuesModel::checkIssues(StalledIssuesVariantList& list,
                                     std::function<bool(const StalledIssue* issue)> func)
{
    foreach(auto& issueToAppend, list)
    {
        long long unsigned hash = issueToAppend.consultData()->getOriginalStall()->getHash();
        auto issues(mStalledIssueRowByHash.values(hash));
        if (!issues.isEmpty())
        {
            foreach(auto& issue, issues)
            {
                if (issue->isValid() && func(issue))
                {
                    list.removeOne(issueToAppend);
                    break;
                }
            }
        }
        else
        {
            mStalledIssueRowByHash.insert(hash, issueToAppend.consultData().get());
        }
    }
}

void StalledIssuesModel::appendCachedIssuesToModel(
    const StalledIssuesVariantList& list, StalledIssueFilterCriterion type)
{
    auto totalRows = rowCount(QModelIndex());
    auto rowsToBeInserted = static_cast<int>(list.size());

    if(rowsToBeInserted > 0)
    {
        beginInsertRows(QModelIndex(), totalRows, totalRows + rowsToBeInserted - 1);

        for (auto it = list.begin(); it != list.end();)
        {
            if(mThreadFinished)
            {
                return;
            }

            StalledIssueVariant issue(*it);

            mStalledIssues.append(issue);
            mStalledIssuesByOrder.insert(issue.consultData().get(), rowCount(QModelIndex()) - 1);

            if (type == StalledIssueFilterCriterion::ALL_ISSUES)
            {
                mCountByFilterCriterion[static_cast<int>(
                    StalledIssue::getCriterionByReason((*it).consultData()->getReason()))]++;

                if (!(*it).consultData()->isBeingSolved())
                {
                    // Connect issue signals
                    connect(issue.getData().get(),
                            &StalledIssue::asyncIssueSolvingFinished,
                            this,
                            &StalledIssuesModel::onAsyncIssueSolvingFinished,
                            Qt::UniqueConnection);

                    connect(issue.getData().get(),
                            &StalledIssue::dataUpdated,
                            this,
                            &StalledIssuesModel::onStalledIssueUpdated,
                            Qt::UniqueConnection);
                }
            }
            else
            {
                mCountByFilterCriterion[static_cast<int>(type)]++;

                if (type == StalledIssueFilterCriterion::FAILED_CONFLICTS)
                {
                    mFailedStalledIssues.append(issue);
                }
                else if (type == StalledIssueFilterCriterion::SOLVED_CONFLICTS)
                {
                    mSolvedStalledIssues.append(issue);
                }
            }

            it++;
        }

        endInsertRows();
    }
}

void StalledIssuesModel::onSendEvent()
{
    if(Preferences::instance()->stalledIssuesEventLastDate() != QDate::currentDate())
    {
        Preferences::instance()->updateStalledIssuesEventLastDate();

        emit updateStalledIssuesOnReceiver(UpdateType::EVENT);
    }
}

void StalledIssuesModel::runMessageBox(QMegaMessageBox::MessageBoxInfo info)
{
    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();
    info.parent = dialog ? dialog->getDialog() : nullptr;

    QMegaMessageBox::warning(info);
}

void StalledIssuesModel::languageChanged()
{
    //We need to update all the UI
    for(int row = 0; row < rowCount(QModelIndex()); ++row)
    {
        auto item(getStalledIssueByRow(row));
        item.getData()->resetUIUpdated();
    }
}

StalledIssueVariant StalledIssuesModel::getStalledIssueByRow(int row) const
{
    StalledIssueVariant issue;
    mModelMutex.lockForRead();
    if(mStalledIssues.size() > row)
    {
        issue = mStalledIssues.at(row);
    }
    mModelMutex.unlock();
    return issue;
}

StalledIssueVariant StalledIssuesModel::getIssueVariantByIssue(const StalledIssue* issue)
{
    auto row(mStalledIssuesByOrder.value(issue, -1));
    if(row >= 0)
    {
        return getStalledIssueByRow(row);
    }

    return StalledIssueVariant();
}

void StalledIssuesModel::updateActiveStalledIssues()
{
    if(!mIssuesRequested && !mSolvingIssues)
    {
        setIssuesRequested(true);
        emit updateStalledIssuesOnReceiver(UpdateType::UI);
    }
}

void StalledIssuesModel::setIssuesRequested(bool state)
{
    state ? blockUi() : unBlockUi();
    mIssuesRequested = state;
}

void StalledIssuesModel::updateStalledIssuesForAutoSolve()
{
    emit updateStalledIssuesOnReceiver(UpdateType::AUTO_SOLVE);
}

void StalledIssuesModel::onNodesUpdate(mega::MegaApi*, mega::MegaNodeList* nodes)
{
    if(nodes)
    {
        mega::MegaNodeList* copiedNodes(nodes->copy());
        Utilities::queueFunctionInObjectThread(mStalledIssuesReceiver, [this, copiedNodes]()
        {
            for (int i = 0; i < copiedNodes->size(); i++)
            {
                if (mThreadFinished)
                {
                    return;
                }

                mega::MegaNode *node = copiedNodes->get(i);
                if (node->getChanges() & mega::MegaNode::CHANGE_TYPE_PARENT)
                {
                    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
                    if(parentNode && parentNode->getType() == mega::MegaNode::TYPE_FILE)
                    {
                        for(int row = 0; row < rowCount(QModelIndex()); ++row)
                        {
                            auto item(getStalledIssueByRow(row));

                            if(item.getData()->containsHandle(node->getHandle()))
                            {
                                auto parentFound(false);
                                while (!parentFound)
                                {
                                    auto currentParentHandle(parentNode->getHandle());
                                    auto parentNodeRaw(MegaSyncApp->getMegaApi()->getParentNode(parentNode.get()));
                                    parentNode.reset(parentNodeRaw);
                                    if(!parentNode || parentNode->getType() != mega::MegaNode::TYPE_FILE)
                                    {
                                        item.getData()->updateHandle(currentParentHandle);
                                        item.getData()->resetUIUpdated();
                                        parentFound = true;
                                    }
                                }
                            }
                        }
                    }
                }
                else if (node->getChanges() & mega::MegaNode::CHANGE_TYPE_COUNTER &&
                        node->isFolder())
                {
                    for (int row = 0; row < rowCount(QModelIndex()); ++row)
                    {
                        auto item(getStalledIssueByRow(row));

                        if (item.getData()->containsHandle(node->getHandle()))
                        {
                            item.getData()->resetUIUpdated();
                        }
                    }
                }
            }
            delete copiedNodes;
        });
    }
    else
    {
        auto stalledIssuesDialog = DialogOpener::findDialog<StalledIssuesDialog>();
        if (stalledIssuesDialog && stalledIssuesDialog->getDialog()->isActiveWindow())
        {
            showIssueExternallyChangedMessageBox();
        }
    }
}

Qt::DropActions StalledIssuesModel::supportedDropActions() const
{
    return Qt::IgnoreAction;
}

bool StalledIssuesModel::hasChildren(const QModelIndex& parent) const
{
    auto result(false);
    auto stalledIssueItem = static_cast<StalledIssue*>(parent.internalPointer());
    if (stalledIssueItem)
    {
        result = false;
    }
    else if(parent.isValid())
    {
        auto issue = getStalledIssueByRow(parent.row());
        if(issue.consultData())
        {
            result = issue.consultData()->isExpandable();
        }
    }
    else
    {
        //Top parent has always children
        result = true;
    }

    return result;
}

int StalledIssuesModel::rowCount(const QModelIndex& parent) const
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

int StalledIssuesModel::columnCount(const QModelIndex&) const
{
   return 1;
}

QVariant StalledIssuesModel::data(const QModelIndex& index, int role) const
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
            return QVariant::fromValue(getStalledIssueByRow(index.row()));
        }
    }
    else if(role == ADAPTATIVE_HEIGHT_ROLE)
    {
        auto issue = getStalledIssueByRow(index.row());
        if(issue.consultData())
        {
            return StalledIssuesDelegateWidgetsCache::adaptativeHeight(issue.consultData()->getReason());
        }
    }

    return QVariant();
}

QModelIndex StalledIssuesModel::parent(const QModelIndex& index) const
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

    auto row(getRowByStalledIssue(stalledIssueItem->consultData()));
    if(row >= 0)
    {
        return createIndex(row, 0);
    }

    return QModelIndex();
}

QModelIndex StalledIssuesModel::index(int row, int column, const QModelIndex& parent) const
{
    if(parent.isValid() && mStalledIssues.size() > parent.row())
    {
        auto& stalledIssue = mStalledIssues[parent.row()];
        return createIndex(0, 0, &stalledIssue);
    }
    else
    {
        return (row < rowCount(QModelIndex())) ?  createIndex(row, column) : QModelIndex();
    }
}

Qt::ItemFlags StalledIssuesModel::flags(const QModelIndex& index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

void StalledIssuesModel::fullReset()
{
    mSolvedStalledIssues.clear();
    reset();
}

bool StalledIssuesModel::isEmpty() const
{
    return !MegaSyncApp->getMegaApi()->isSyncStalled() ||
           rowCount(QModelIndex()) == 0 ||
           (mSolvedStalledIssues.size() > 0 && rowCount(QModelIndex()) == mSolvedStalledIssues.size());
}

void StalledIssuesModel::finishStalledIssues(const QModelIndexList& indexes)
{
    auto indexesToFinish(indexes);
    removeRows(indexesToFinish);
}

void StalledIssuesModel::removeRows(QModelIndexList& indexesToRemove)
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

bool StalledIssuesModel::removeRows(int row, int count, const QModelIndex& parent)
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
        auto item = getStalledIssueByRow(row);
        mStalledIssuesByOrder.insert(item.consultData().get(), row);

        mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason(item.consultData()->getReason()))]++;
    }

    emit stalledIssuesCountChanged();
}

int StalledIssuesModel::getRowByStalledIssue(const std::shared_ptr<const StalledIssue> issue) const
{
    return mStalledIssuesByOrder.value(issue.get(), -1);
}

int StalledIssuesModel::getRowByStalledIssue(const StalledIssue* issue) const
{
    return mStalledIssuesByOrder.value(issue, -1);
}

void StalledIssuesModel::blockUi()
{
    emit uiBlocked();
}

void StalledIssuesModel::unBlockUi()
{
    emit uiUnblocked();
}

void StalledIssuesModel::onStalledIssueUpdated(StalledIssue* issue)
{
    auto row(getRowByStalledIssue(issue));
    if(row >= 0)
    {
        auto headerIndex(index(row, 0));
        auto bodyIndex(index(0, 0, headerIndex));
        updateIndex(headerIndex);
        updateIndex(bodyIndex);
    }
}

void StalledIssuesModel::updateIndex(const QModelIndex& index)
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
        auto issue(getStalledIssueByRow(row));
        if(issue.consultData() && !issue.consultData()->isSolved() && checker(issue.consultData()))
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

void StalledIssuesModel::UiItemUpdate(const QModelIndex& oldIndex, const QModelIndex& newIndex)
{
    if(oldIndex.isValid() &&
       oldIndex != newIndex)
    {
        auto newType = newIndex.parent().isValid() ? StalledIssue::Type::Body : StalledIssue::Type::Header;
        auto row(newType == StalledIssue::Type::Body ? oldIndex.parent().row() :
                                                       oldIndex.row());
        auto oldIssue(getStalledIssueByRow(row));
        oldIssue.getData()->resetUIUpdated();

        oldIssue.getData()->removeFileWatcher();
    }

    auto row(-1);

    if(newIndex.parent().isValid())
    {
        row = newIndex.parent().row();
    }
    else
    {
        row = newIndex.row();
    }

    auto newIssue(getStalledIssueByRow(row));
    auto newType = newIndex.parent().isValid() ? StalledIssue::Type::Body
                                               : StalledIssue::Type::Header;
    newIssue.getData()->UIUpdated(newType);
    newIssue.getData()->createFileWatcher();
}

void StalledIssuesModel::reset()
{
    auto solvedIssues(mSolvedStalledIssues);

    beginResetModel();

    mStalledIssues.clear();
    mFailedStalledIssues.clear();
    mStalledIssuesByOrder.clear();
    mCountByFilterCriterion.clear();
    mStalledIssueRowByHash.clear();
    mSolvedStalledIssues.clear();

    endResetModel();

    // Re-insert solved issues
    checkAutoSolvedIssues(solvedIssues);
    appendCachedIssuesToModel(solvedIssues, StalledIssueFilterCriterion::SOLVED_CONFLICTS);

    emit stalledIssuesCountChanged();
}

QModelIndex StalledIssuesModel::getSolveIssueIndex(const QModelIndex& index)
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
    if(mThreadFinished || mSolvingIssuesStopped)
    {
        mSolvingIssuesStopped = false;
        return true;
    }

    return false;
}

void StalledIssuesModel::stopSolvingIssues(MessageInfo::ButtonType buttonType)
{
    if(mIssuesSolved)
    {
        mIssuesSolved = false;
        emit refreshFilter();
    }
    else
    {
        if(buttonType == MessageInfo::ButtonType::STOP)
        {
            mSolvingIssuesStopped = true;
        }
        else
        {
            unBlockUi();
        }
    }
}

bool StalledIssuesModel::isSolvingIssues() const
{
    return mSolvingIssues.load();
}

void StalledIssuesModel::startSolvingIssues()
{
    mSolvingIssues = true;
    blockUi();
}

void StalledIssuesModel::finishSolvingIssues(StalledIssuesCreator::IssuesCount count, bool sendMessage)
{
    mSolvingIssues = false;
    mIssuesSolved = true;

    sendFinishSolvingMessage(count, sendMessage);
    emit stalledIssuesSolvingFinished();
}

void StalledIssuesModel::sendFinishSolvingMessage(
    StalledIssuesCreator::IssuesCount count, bool sendMessage)
{
    if(sendMessage)
    {
        auto info = std::make_shared<MessageInfo>();
        info->message = issuesFixedString(count);
        info->buttonType = MessageInfo::ButtonType::OK;
        emit updateLoadingMessage(info);
    }
    else
    {
        emit updateLoadingMessage(nullptr);
    }
}

void StalledIssuesModel::sendFixingIssuesMessage(int issue, int totalIssues)
{
    auto info = std::make_shared<MessageInfo>();
    info->message = fixingIssuesString();
    info->count = issue;
    info->total = totalIssues;
    info->buttonType = MessageInfo::ButtonType::STOP;
    emit updateLoadingMessage(info);
}

void StalledIssuesModel::solveListOfIssues(const SolveListInfo &info)
{
    //Don´t block UI if the issue is being solved async
    if(!info.async)
    {
        startSolvingIssues();
    }

    Utilities::queueFunctionInObjectThread(mStalledIssuesReceiver, [this, info]() {
        if (info.startFunc)
        {
            info.startFunc();
        }

        StalledIssuesCreator::IssuesCount count;
        int issuesExternallyChanged(0);
        auto totalRows(info.indexes.size());
        foreach(auto index, info.indexes)
        {
            if (checkIfUserStopSolving())
            {
                break;
            }

            // Don´t block the UI if the issue is being solve asynchronously
            if (!info.async)
            {
                sendFixingIssuesMessage(count.currentIssueBeingSolved, totalRows);
            }

            if (mThreadFinished)
            {
                return;
            }

            auto potentialIndex = getSolveIssueIndex(index);
            mModelMutex.lockForRead();
            auto issue(mStalledIssues.at(potentialIndex.row()));
            mModelMutex.unlock();

            if (issue.getData())
            {
                if (issue.getData()->isFailed())
                {
                    mFailedStalledIssues.removeOne(issue);
                    mCountByFilterCriterion[static_cast<int>(
                        StalledIssueFilterCriterion::FAILED_CONFLICTS)]--;
                }

                if (issue.getData()->checkForExternalChanges())
                {
                    issuesExternallyChanged++;
                    count.issuesFailed++;
                }
                else
                {
                    if (info.solveFunc)
                    {
                        auto result(info.solveFunc(potentialIndex.row()));
                        if (!info.async)
                        {
                            if (result)
                            {
                                count.issuesFixed++;
                            }
                            else
                            {
                                count.issuesFailed++;
                            }
                            issueSolvingFinished(issue.getData().get(), result);
                        }
                    }
                }
            }
            count.currentIssueBeingSolved++;
        }

        if (!info.async)
        {
            if (issuesExternallyChanged > 0)
            {
                unBlockUi();
                showIssueExternallyChangedMessageBox();
            }

            if (info.finishFunc)
            {
                info.finishFunc(count.issuesFixed, issuesExternallyChanged > 0);
            }

            finishSolvingIssues(count);
        }
        else if (issuesExternallyChanged > 0)
        {
            showIssueExternallyChangedMessageBox();
            finishSolvingIssues(count, false);
        }

        // Update counters and filters
        emit stalledIssuesCountChanged();
        emit stalledIssuesChanged();
    });
}

void StalledIssuesModel::showIssueExternallyChangedMessageBox()
{
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok;
    QMap<QMessageBox::StandardButton, QString> buttonsText;
    buttonsText.insert(QMessageBox::Ok, tr("Refresh"));
    msgInfo.buttonsText = buttonsText;
    msgInfo.text = tr("The issue may have been solved externally.\nPlease, refresh the list.");
    msgInfo.finishFunc = [this](QPointer<QMessageBox>) {
        updateActiveStalledIssues();
    };

    runMessageBox(std::move(msgInfo));
}

int StalledIssuesModel::getCountByFilterCriterion(StalledIssueFilterCriterion criterion)
{
    if(criterion == StalledIssueFilterCriterion::ALL_ISSUES)
    {
        auto solvedIssuesCount(mCountByFilterCriterion.value(static_cast<int>(StalledIssueFilterCriterion::SOLVED_CONFLICTS),0));
        return (rowCount(QModelIndex()) - (solvedIssuesCount));
    }
    else
    {
        return mCountByFilterCriterion.value(static_cast<int>(criterion),0);
    }
}

//METHODS TO SOLVE ISSUES
void StalledIssuesModel::onAsyncIssueSolvingFinished(StalledIssue* issue)
{
    //Refresh UI
    if(issueSolvingFinished(issue))
    {
        emit refreshFilter();
        emit stalledIssuesChanged();
    }
}

bool StalledIssuesModel::issueSolvingFinished(const StalledIssue* issue)
{
    auto result(issueFailed(issue));
    if(!result)
    {
        result = issueSolved(issue);
    }

    emit stalledIssuesCountChanged();

    return result;
}

bool StalledIssuesModel::issueSolvingFinished(StalledIssue* issue, bool wasSuccessful)
{
    if(wasSuccessful)
    {
        issue->setIsSolved(StalledIssue::SolveType::SOLVED);
    }
    else
    {
        issue->setIsSolved(StalledIssue::SolveType::FAILED);
    }

    return issueSolvingFinished(issue);
}

bool StalledIssuesModel::issueSolved(const StalledIssue* issue)
{
    if(issue && issue->isSolved() && !issue->isPotentiallySolved())
    {
        auto issueVariant(getIssueVariantByIssue(issue));
        if(issueVariant.isValid())
        {
            mSolvedStalledIssues.append(issueVariant);
            mCountByFilterCriterion[static_cast<int>(StalledIssueFilterCriterion::SOLVED_CONFLICTS)]++;
            auto& counter = mCountByFilterCriterion[static_cast<int>(
                StalledIssue::getCriterionByReason(issue->getReason()))];
            if(counter > 0)
            {
                counter--;
            }

            return true;
        }
    }

    return false;
}

//METHODS TO SOLVE ISSUES
bool StalledIssuesModel::issueFailed(const StalledIssue* issue)
{
    if(issue && issue->isFailed())
    {
        auto issueVariant(getIssueVariantByIssue(issue));
        if(issueVariant.isValid())
        {
            mFailedStalledIssues.append(issueVariant);
            mCountByFilterCriterion[static_cast<int>(StalledIssueFilterCriterion::FAILED_CONFLICTS)]++;

            return true;
        }
    }

    return false;
}

void StalledIssuesModel::chooseSideManually(bool remote, const QModelIndexList& list)
{
    auto resolveIssue = [this, remote](int row) -> bool
    {
        auto result(false);
        auto item(getStalledIssueByRow(row));
        if(item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose ||
           item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose)
        {
            if(auto issue = item.convert<LocalOrRemoteUserMustChooseStalledIssue>())
            {
                result = remote ? issue->chooseRemoteSide() : issue->chooseLocalSide();
                if(result)
                {
                    MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_LOCALREMOTE_SOLVED_MANUALLY);
                }
            }
        }

        return result;
    };

    SolveListInfo info(list, resolveIssue);
    solveListOfIssues(info);
}

void StalledIssuesModel::chooseBothSides(const QModelIndexList& list)
{
    auto resolveIssue = [this](int row) -> bool
    {
        auto result(false);
        auto item(getStalledIssueByRow(row));
        if(item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose ||
            item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose)
        {
            if(auto issue = item.convert<LocalOrRemoteUserMustChooseStalledIssue>())
            {
                result = issue->chooseBothSides();
            }

            if(result)
            {
                MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_LOCALREMOTE_SOLVED_MANUALLY);
            }
        }

        return result;
    };

    SolveListInfo info(list, resolveIssue);
    solveListOfIssues(info);
}


void StalledIssuesModel::chooseRemoteForBackups(const QModelIndexList& list)
{
    mSyncsToDisable.clear();

    auto resolveIssue = [this](int row) -> bool
    {
        auto result(false);
        auto item(getStalledIssueByRow(row));
        if(item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose ||
           item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose)
        {
            if(!item.consultData()->syncIds().isEmpty())
            {
                auto sync = SyncInfo::instance()->getSyncSettingByTag(item.consultData()->firstSyncId());
                if(sync && !mSyncsToDisable.contains(sync))
                {
                    mSyncsToDisable.append(sync);
                }
                result = true;
            }
        }

        return result;
    };

    auto finishFunc = [this](int, bool)
    {
        foreach(auto& sync, mSyncsToDisable)
        {
            SyncController::instance().setSyncToDisabled(sync);
        }
    };

    SolveListInfo info(list, resolveIssue);
    info.finishFunc = finishFunc;
    solveListOfIssues(info);
}

void StalledIssuesModel::semiAutoSolveLocalRemoteIssues(const QModelIndexList& list)
{
    auto resolveIssue = [this](int row) -> bool
    {
        auto result(false);
        auto item(getStalledIssueByRow(row));
        auto localRemoteIssue = item.convert<LocalOrRemoteUserMustChooseStalledIssue>();
        if(localRemoteIssue)
        {
            result = localRemoteIssue->chooseLastMTimeSide();

            if(result)
            {
                MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_LOCALREMOTE_SOLVED_SEMI_AUTOMATICALLY);
            }
        }
        return result;
    };

    SolveListInfo info(list, resolveIssue);
    solveListOfIssues(info);
}

void StalledIssuesModel::ignoreItems(const QModelIndexList& list)
{
    auto ignoredItemsBySync = new QMap<mega::MegaHandle, QList<IgnoredStalledIssue::IgnoredPath>>();

    auto resolveIssue = [this, ignoredItemsBySync](int row) -> bool
    {
        auto item(getStalledIssueByRow(row));
        if(auto ignorableItem = StalledIssue::convert<IgnoredStalledIssue>(item.getData()))
        {
            if(!item.getData()->syncIds().isEmpty())
            {
                std::shared_ptr<mega::MegaSync> sync(
                    MegaSyncApp->getMegaApi()->getSyncByBackupId(item.getData()->firstSyncId()));
                if(sync)
                {
                    auto folderPath(
                        QDir::toNativeSeparators(QString::fromUtf8(sync->getLocalFolder())));
                    if(MegaIgnoreManager::isValid(folderPath))
                    {
                        auto& items = (*ignoredItemsBySync)[item.getData()->firstSyncId()];
                        auto ignoredItems = ignorableItem->getIgnoredFiles();
                        foreach(auto& ignoredItem, ignoredItems)
                        {
                            auto findRepeatedPath =
                                [ignoredItem](const IgnoredStalledIssue::IgnoredPath& check) -> bool {
                                return ignoredItem.pathSide == IgnoredStalledIssue::IgnoredPath::IgnorePathSide::REMOTE &&
                                       ignoredItem.path == check.path;
                            };

                            if(std::find_if(items.cbegin(), items.cend(), findRepeatedPath) ==
                                items.cend())
                            {
                                items.append(ignoredItem);
                                MegaSyncApp->getStatsEventHandler()->sendEvent(
                                    AppStatsEvents::EventType::SI_IGNORE_SOLVED_MANUALLY);
                            }
                        }

                        return true;
                    }
                }
            }
        }

        return false;
    };

    auto issuesToFix(list.size());

    auto finishFunc = [this, ignoredItemsBySync, issuesToFix](int issuesFixed, bool externallyModified)
    {
        foreach(auto syncId,  ignoredItemsBySync->keys())
        {
            std::shared_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(syncId));
            if(sync)
            {
                auto folderPath(QDir::toNativeSeparators(QString::fromUtf8(sync->getLocalFolder())));
                MegaIgnoreManager manager(folderPath, false);

                auto ignoredFiles(ignoredItemsBySync->value(syncId));
                foreach(auto ignoredPath, ignoredFiles)
                {
                    QDir dir;
                    if(ignoredPath.pathSide == IgnoredStalledIssue::IgnoredPath::IgnorePathSide::REMOTE)
                    {
                        dir.setPath(QString::fromUtf8(sync->getLastKnownMegaFolder()));
                    }
                    else
                    {
                        dir.setPath(folderPath);

                    }
                    manager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                        dir.relativeFilePath(ignoredPath.path), ignoredPath.target);
                }

                manager.applyChanges();
            }
        }

        if(!externallyModified && issuesFixed < issuesToFix)
        {
            showIgnoreItemsError(issuesFixed == 0);
        }

        delete ignoredItemsBySync;
    };


    SolveListInfo info(list, resolveIssue);
    info.finishFunc = finishFunc;
    solveListOfIssues(info);
}

void StalledIssuesModel::ignoreAllSimilarIssues()
{
    //Double check needed?
    QModelIndexList list;
    auto totalRows(rowCount(QModelIndex()));
    for(int row = 0; row < totalRows; ++row)
    {
        auto item = getStalledIssueByRow(row);
        if(item.convert<IgnoredStalledIssue>())
        {
            if(!item.getData()->isSolved())
            {
                list.append(index(row, 0));
            }
        }
    }

    ignoreItems(list);
}

void StalledIssuesModel::ignoreSymLinks()
{
    QList<mega::MegaHandle> involvedSyncs;
    QList<mega::MegaHandle> involvedFailedToIgnoreSyncs;

    QModelIndexList list;
    auto totalRows(rowCount(QModelIndex()));
    for(int row = 0; row < totalRows; ++row)
    {
        auto item = getStalledIssueByRow(row);
        if(auto ignorableIssue = item.convert<IgnoredStalledIssue>())
        {
            if(ignorableIssue->isSymLink() && !item.getData()->isSolved())
            {
                if(!item.getData()->syncIds().isEmpty())
                {
                    auto syncId(item.getData()->firstSyncId());
                    if(!involvedSyncs.contains(syncId))
                    {
                        involvedSyncs.append(syncId);

                        std::unique_ptr<mega::MegaSync> sync(
                            MegaSyncApp->getMegaApi()->getSyncByBackupId(syncId));
                        if(sync)
                        {
                            auto folderPath(QDir::toNativeSeparators(
                                QString::fromUtf8(sync->getLocalFolder())));
                            if(!MegaIgnoreManager::isValid(folderPath))
                            {
                                involvedFailedToIgnoreSyncs.append(syncId);
                            }
                        }
                        else
                        {
                            involvedFailedToIgnoreSyncs.append(syncId);
                        }
                    }
                }

                list.append(index(row, 0));
            }
        }
    }

    auto startIssue = [list, involvedSyncs, involvedFailedToIgnoreSyncs]()
    {
        std::unique_ptr<mega::MegaSyncList>syncs(MegaSyncApp->getMegaApi()->getSyncs());
        for (int i = 0; i < syncs->size(); ++i)
        {
            auto folderPath(QDir::toNativeSeparators(QString::fromUtf8(syncs->get(i)->getLocalFolder())));
            if(MegaIgnoreManager::isValid(folderPath))
            {
                MegaIgnoreManager manager(folderPath, false);
                manager.addIgnoreSymLinksRule();
                manager.applyChanges();
            }
        }
    };

    auto resolveIssue = [this, involvedFailedToIgnoreSyncs](int row) -> bool
    {
        auto item(getStalledIssueByRow(row));
        if(!item.getData()->syncIds().isEmpty())
        {
            return !involvedFailedToIgnoreSyncs.contains(item.getData()->firstSyncId());
        }

        return false;
    };

    auto finishIssue = [this, list](int issuesFixed, bool externallyModified)
    {
        if(issuesFixed < list.size())
        {
            if(!externallyModified)
            {
                showIgnoreItemsError(issuesFixed == 0);
            }

            if(issuesFixed == 0)
            {
                emit updateLoadingMessage(nullptr);
            }
        }
        else if(issuesFixed > 0)
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_IGNORE_ALL_SYMLINK);
        }
    };

    SolveListInfo info(list, resolveIssue);
    info.startFunc = startIssue;
    info.finishFunc = finishIssue;
    solveListOfIssues(info);
}

void StalledIssuesModel::showIgnoreItemsError(bool allFailed)
{
    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok;
    msgInfo.text = allFailed ? tr("Some issues can't be fixed.\nVerify the permissions of the .megaignore file on your local sync folder locations.")
                             : tr("Issues can't be fixed.\nVerify the permissions of the .megaignore on file your local sync folder locations.");

    runMessageBox(std::move(msgInfo));
}

void StalledIssuesModel::fixFingerprint(const QModelIndexList& list)
{
    mFingerprintIssuesToFix.clear();

    auto finishIssue = [this](int, bool)
    {
        mFingerprintIssuesSolver.solveIssues(mFingerprintIssuesToFix);
    };

    auto resolveIssue = [this](int row) -> bool
    {
        auto item(getStalledIssueByRow(row));
        mFingerprintIssuesToFix.append(item);

        MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_FINGERPRINT_MISSING_SOLVED_MANUALLY);
        return true;
    };

    SolveListInfo info(list, resolveIssue);
    info.finishFunc = finishIssue;
    solveListOfIssues(info);
}

void StalledIssuesModel::fixFolderMatchedAgainstFile(const QModelIndexList& list)
{
    auto resolveIssue = [this](int row) -> bool
    {
        auto item(getStalledIssueByRow(row));

        if(auto folderAgainstFile = std::dynamic_pointer_cast<FolderMatchedAgainstFileIssue>(item.getData()))
        {
            return folderAgainstFile->solveIssue();
        }
        return false;
    };

    SolveListInfo info(list, resolveIssue);
    solveListOfIssues(info);
}

void StalledIssuesModel::fixMoveOrRenameCannotOccur(const QModelIndexList& indexes, MoveOrRenameIssueChosenSide side)
{
    auto resolveIssue = [this, side](int row) -> bool
    {
        auto item(getStalledIssueByRow(row));
        if(item.consultData()->syncIds().isEmpty())
        {
            return false;
        }

        if(auto moveOrRemoveIssue = std::dynamic_pointer_cast<MoveOrRenameCannotOccurIssue>(item.getData()))
        {
            auto solver(new MoveOrRenameMultiStepIssueSolver(moveOrRemoveIssue));
            mStalledIssuesReceiver->addMultiStepIssueSolver<MoveOrRenameCannotOccurIssue>(solver);

            moveOrRemoveIssue->solveIssue(side);

        }
        return true;
    };

    SolveListInfo info(indexes, resolveIssue);
    info.async = true;
    solveListOfIssues(info);
}

void StalledIssuesModel::semiAutoSolveNameConflictIssues(const QModelIndexList& list, uint option)
{
    auto resolveIssue = [this, option](int row) -> bool
    {
        auto result(false);
        auto item(getStalledIssueByRow(row));
        if(!item.getData()->checkForExternalChanges())
        {
            if(item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced)
            {
                if(auto nameConflict = item.convert<NameConflictedStalledIssue>())
                {
                    result = nameConflict->semiAutoSolveIssue(static_cast<NameConflictedStalledIssue::ActionsSelected>(option));

                    if(result)
                    {
                        MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_NAMECONFLICT_SOLVED_SEMI_AUTOMATICALLY);
                    }
                }
            }
        }

        return result;
    };

    SolveListInfo info(list, resolveIssue);
    solveListOfIssues(info);
}

bool StalledIssuesModel::solveLocalConflictedNameByRemove(int conflictIndex, const QModelIndex& index)
{
    auto areAllSolved(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue.convert<NameConflictedStalledIssue>())
    {
        areAllSolved = nameConflict->solveLocalConflictedNameByRemove(conflictIndex);
        if(areAllSolved)
        {
            issueSolvingFinished(issue.getData().get(), true);
            finishConflictManually();
        }
    }

    return areAllSolved;
}

bool StalledIssuesModel::solveLocalConflictedNameByRename(const QString& renameTo, const QString& renameFrom, int conflictIndex, const QModelIndex& index)
{
    auto areAllSolved(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue.convert<NameConflictedStalledIssue>())
    {
        areAllSolved = nameConflict->solveLocalConflictedNameByRename(conflictIndex, renameTo, renameFrom);
        if(areAllSolved)
        {
            issueSolvingFinished(issue.getData().get(), true);
            finishConflictManually();
        }
    }

    return areAllSolved;
}

void StalledIssuesModel::solveLocalConflictedNameFailed(int conflictIndex, const QModelIndex& index, const QString& error)
{
    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue.convert<NameConflictedStalledIssue>())
    {
        nameConflict->setLocalFailed(conflictIndex, error);
    }
}

bool StalledIssuesModel::checkForExternalChanges(const QModelIndex& index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    auto result = issue.getData()->checkForExternalChanges();
    if(result)
    {
        showIssueExternallyChangedMessageBox();
    }

    return result;
}

bool StalledIssuesModel::solveCloudConflictedNameByRemove(int conflictIndex, const QModelIndex& index)
{
    auto areAllSolved(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue.convert<NameConflictedStalledIssue>())
    {
        areAllSolved = nameConflict->solveCloudConflictedNameByRemove(conflictIndex);
        if(areAllSolved)
        {
            issueSolvingFinished(issue.getData().get(), true);
            finishConflictManually();
        }
    }

    return areAllSolved;
}

bool StalledIssuesModel::solveCloudConflictedNameByRename(const QString& renameTo, const QString& renameFrom, int conflictIndex, const QModelIndex& index)
{
    auto areAllSolved(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue.convert<NameConflictedStalledIssue>())
    {
        areAllSolved = nameConflict->solveCloudConflictedNameByRename(conflictIndex, renameTo, renameFrom);
        if(areAllSolved)
        {
            issueSolvingFinished(issue.getData().get(), true);
            finishConflictManually();
        }
    }

    return areAllSolved;
}

void StalledIssuesModel::solveCloudConflictedNameFailed(int conflictIndex, const QModelIndex& index, const QString& error)
{
    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue.convert<NameConflictedStalledIssue>())
    {
        nameConflict->setCloudFailed(conflictIndex, error);
    }
}

void StalledIssuesModel::finishConflictManually()
{
    blockUi();
    //Only was issue was fixed
    StalledIssuesCreator::IssuesCount count;
    count.issuesFixed = 1;
    finishSolvingIssues(count);
}
