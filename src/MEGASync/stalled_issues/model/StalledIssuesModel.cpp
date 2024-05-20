#include "StalledIssuesModel.h"

#include "MegaApplication.h"
#include <StalledIssuesDelegateWidgetsCache.h>
#include "NameConflictStalledIssue.h"
#include "MoveOrRenameCannotOccurIssue.h"
#include <IgnoredStalledIssue.h>
#include <LocalOrRemoteUserMustChooseStalledIssue.h>
#include <QMegaMessageBox.h>
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <MultiStepIssueSolver.h>
#include <syncs/control/MegaIgnoreManager.h>
#include "StatsEventHandler.h"

#include <QSortFilterProxyModel>

StalledIssuesReceiver::StalledIssuesReceiver(QObject* parent) : QObject(parent), mega::MegaRequestListener()
{
}

void StalledIssuesReceiver::updateStalledIssues(UpdateType type)
{
    QMutexLocker lock(&mCacheMutex);
    mUpdateType = type;
    MegaSyncApp->getMegaApi()->getMegaSyncStallList(nullptr);
}

void StalledIssuesReceiver::onRequestFinish(mega::MegaApi*, mega::MegaRequest* request, mega::MegaError*)
{
    if (request->getType() == ::mega::MegaRequest::TYPE_GET_SYNC_STALL_LIST)
    {
        QMutexLocker lock(&mCacheMutex);
        mStalledIssues.clear();
        IgnoredStalledIssue::clearIgnoredSyncs();

        connect(&mIssueCreator, &StalledIssuesCreator::solvingIssues, this, &StalledIssuesReceiver::solvingIssues);

        mIssueCreator.createIssues(request->getMegaSyncStallList(), mUpdateType, mMultiStepIssueSolversByReason);
        mStalledIssues = mIssueCreator.issues();

        StalledIssuesBySyncFilter::resetFilter();

        if(mUpdateType == UpdateType::UI)
        {
            emit stalledIssuesReady(mStalledIssues);
        }
    }
}

const int StalledIssuesModel::ADAPTATIVE_HEIGHT_ROLE = Qt::UserRole;
const int EVENT_REQUEST_DELAY = 600000; /*10 minutes*/

StalledIssuesModel::StalledIssuesModel(QObject* parent)
    : QAbstractItemModel(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mIsStalled(false)
    , mIsStalledChanged(false)
    , mRawInfoVisible(false)
{
    mStalledIssuesThread = new QThread();
    mStalledIssuesReceiver = new StalledIssuesReceiver();

    mRequestListener = new mega::QTMegaRequestListener(mMegaApi, mStalledIssuesReceiver);
    mStalledIssuesReceiver->moveToThread(mStalledIssuesThread);
    mRequestListener->moveToThread(mStalledIssuesThread);
    mMegaApi->addRequestListener(mRequestListener);

    mGlobalListener = new mega::QTMegaGlobalListener(mMegaApi,this);
    mMegaApi->addGlobalListener(mGlobalListener);

    connect(mStalledIssuesReceiver, &StalledIssuesReceiver::solvingIssues, this, [this](int issueCounter, int totalIssues)
    {
        auto info = std::make_shared<MessageInfo>();
        info->message = tr("Processing issues");
        info->buttonType = MessageInfo::ButtonType::None;
        info->count = issueCounter;
        info->total = totalIssues;
        emit updateLoadingMessage(info);
    }, Qt::QueuedConnection);

    mStalledIssuesThread->start();

    connect(mStalledIssuesReceiver, &StalledIssuesReceiver::stalledIssuesReady,
            this, &StalledIssuesModel::onProcessStalledIssues,
            Qt::QueuedConnection);

    connect(&mEventTimer,&QTimer::timeout, this, &StalledIssuesModel::onSendEvent);
    mEventTimer.setSingleShot(true);
}

bool StalledIssuesModel::issuesRequested() const
{
    return mIssuesRequested.load();
}

void StalledIssuesModel::onGlobalSyncStateChanged(mega::MegaApi* api)
{
    auto isSyncStalled(api->isSyncStalled());
    auto isSyncStalledChanged(api->isSyncStalledChanged());

    if(isSyncStalled && (mIsStalled != isSyncStalled || isSyncStalledChanged != mIsStalledChanged)
        && mStalledIssuesReceiver->multiStepIssueSolveActive())
    {
        mStalledIssuesReceiver->updateStalledIssues(UpdateType::AUTO_SOLVE);
    }
    else
    {
        if (isSyncStalled && mStalledIssues.size() == mSolvedStalledIssues.size() &&
            mIsStalled != isSyncStalled)
        {
            //For Smart mode -> resolve problems as soon as they are received
            updateStalledIssues();
        }

        emit stalledIssuesChanged();
    }

    mIsStalledChanged = isSyncStalledChanged;
    mIsStalled = isSyncStalled;
}

StalledIssuesModel::~StalledIssuesModel()
{
    delete mRequestListener;
    delete mGlobalListener;

    mThreadFinished = true;

    mStalledIssuesThread->quit();
    mStalledIssuesReceiver->deleteLater();
}

void StalledIssuesModel::onProcessStalledIssues(StalledIssuesVariantList issuesReceived)
{
    if(!issuesReceived.isEmpty() && !mEventTimer.isActive())
    {
        mEventTimer.start(EVENT_REQUEST_DELAY);
    }

    Utilities::queueFunctionInObjectThread(mStalledIssuesReceiver, [this, issuesReceived]()
    {
        reset();
        mModelMutex.lockForWrite();

        blockSignals(true);

        auto totalRows = rowCount(QModelIndex());
        auto rowsToBeInserted(static_cast<int>(issuesReceived.size()));

        if(rowsToBeInserted > 0)
        {
            beginInsertRows(QModelIndex(), totalRows, totalRows + rowsToBeInserted - 1);

            for (auto it = issuesReceived.begin(); it != issuesReceived.end();)
            {
                if(mThreadFinished)
                {
                    return;
                }

                StalledIssueVariant issue(*it);
                mStalledIssues.append(issue);
                mStalledIssuesByOrder.insert(issue.consultData().get(), rowCount(QModelIndex()) - 1);
                mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason((*it).consultData()->getReason()))]++;

                //Connect issue signals
                connect(issue.getData().get(),
                    &StalledIssue::asyncIssueBeingSolved,
                    this,
                    [this]()
                    {
                        QString solveMessage(tr("Issue being solved."));
                        finishSolvingIssues(0, true, solveMessage);
                    });

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

                 StalledIssueVariant issue(*it);
                 mStalledIssues.append(issue);
                 mStalledIssuesByOrder.insert(issue.consultData().get(), rowCount(QModelIndex()) - 1);
                 mCountByFilterCriterion[static_cast<int>(StalledIssueFilterCriterion::SOLVED_CONFLICTS)]++;

                 it++;
             }

             endInsertRows();
        }

        blockSignals(false);
        mModelMutex.unlock();

        mIssuesRequested = false;

        emit stalledIssuesCountChanged();
        emit stalledIssuesReceived();
        emit stalledIssuesChanged();
    });
}

void StalledIssuesModel::onSendEvent()
{
    if(Preferences::instance()->stalledIssuesEventLastDate() != QDate::currentDate())
    {
        Preferences::instance()->updateStalledIssuesEventLastDate();

        mStalledIssuesReceiver->updateStalledIssues(UpdateType::EVENT);
    }
}

void StalledIssuesModel::runMessageBox(QMegaMessageBox::MessageBoxInfo info)
{
    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();
    info.parent = dialog ? dialog->getDialog() : nullptr;

    //Run the messagebox in the mGUI thread)
    Utilities::queueFunctionInAppThread([info]()
    {
        QMegaMessageBox::warning(info);
    });
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

void StalledIssuesModel::updateStalledIssues()
{
    if(!mIssuesRequested && !mSolvingIssues)
    {
        blockUi();
        mIssuesRequested = true;
        mStalledIssuesReceiver->updateStalledIssues(UpdateType::UI);
    }
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
                mega::MegaNode *node = copiedNodes->get(i);
                if (node->getChanges() & mega::MegaNode::CHANGE_TYPE_PARENT)
                {
                    std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
                    if(parentNode && parentNode->getType() == mega::MegaNode::TYPE_FILE)
                    {
                        for(int row = 0; row < rowCount(QModelIndex()); ++row)
                        {
                            mModelMutex.lockForWrite();
                            auto item = mStalledIssues.at(row);
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
                            mModelMutex.unlock();
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
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = MegaSyncApp->getMEGAString();
            msgInfo.textFormat = Qt::RichText;
            msgInfo.buttons = QMessageBox::Ok;
            QMap<QMessageBox::StandardButton, QString> buttonsText;
            buttonsText.insert(QMessageBox::Ok, tr("Refresh"));
            msgInfo.buttonsText = buttonsText;
            msgInfo.text = tr("Some external changes were detected. Please, refresh the view.");
            msgInfo.finishFunc = [this](QPointer<QMessageBox>) {
                updateStalledIssues();
            };
            runMessageBox(std::move(msgInfo));
        }
    }
}

Qt::DropActions StalledIssuesModel::supportedDropActions() const
{
    return Qt::IgnoreAction;
}

bool StalledIssuesModel::hasChildren(const QModelIndex& parent) const
{
    auto stalledIssueItem = static_cast<StalledIssue*>(parent.internalPointer());
    if (stalledIssueItem)
    {
        return false;
    }

    return true;
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

    auto row = mStalledIssuesByOrder.value(stalledIssueItem->consultData().get(),-1);
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
    mLastSolvedStalledIssue = StalledIssueVariant();
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

void StalledIssuesModel::blockUi()
{
    emit uiBlocked();
}

void StalledIssuesModel::unBlockUi()
{
    emit uiUnblocked();
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

    if(newIndex.parent().isValid())
    {
        auto row(newIndex.parent().row());
        auto newIssue(getStalledIssueByRow(row));
        auto newType = newIndex.parent().isValid() ? StalledIssue::Type::Body : StalledIssue::Type::Header;
        newIssue.getData()->UIUpdated(newType);
        newIssue.getData()->createFileWatcher();
    }
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

void StalledIssuesModel::stopSolvingIssues()
{
    if(mIssuesSolved)
    {
        mIssuesSolved = false;

        if(mLastSolvedStalledIssue.consultData() &&
            mLastSolvedStalledIssue.consultData()->refreshListAfterSolving())
        {
            updateStalledIssues();
        }
        else
        {
            emit refreshFilter();
        }
    }
    else
    {
        mSolvingIssuesStopped = true;
    }
}

void StalledIssuesModel::startSolvingIssues()
{
    mSolvingIssues = true;
    blockUi();
}

void StalledIssuesModel::finishSolvingIssues(int issuesFixed, bool sendMessage, const QString& message)
{
    mSolvingIssues = false;
    mIssuesSolved = true;

    if(sendMessage)
    {
        auto info = std::make_shared<MessageInfo>();
        info->message = message.isEmpty() ? tr("%n issues fixed", "", issuesFixed) : message;
        info->buttonType = MessageInfo::ButtonType::Ok;
        emit updateLoadingMessage(info);
    }
    else
    {
        emit updateLoadingMessage(nullptr);
    }

    emit stalledIssuesCountChanged();
    emit stalledIssuesChanged();
}

void StalledIssuesModel::sendFixingIssuesMessage(int issue, int totalIssues)
{
    auto info = std::make_shared<MessageInfo>();
    info->message = tr("Fixing issues");
    info->count = issue;
    info->total = totalIssues;
    info->buttonType = MessageInfo::ButtonType::Stop;
    emit updateLoadingMessage(info);
}

void StalledIssuesModel::solveListOfIssues(const SolveListInfo &info)
{
    startSolvingIssues();
    Utilities::queueFunctionInObjectThread(mStalledIssuesReceiver, [this, info]()
    {
       if(info.startFunc)
       {
           info.startFunc();
       }

       auto issueCounter(1);
       auto issuesFixed(0);
       auto issuesExternallyChanged(0);
       auto totalRows(info.indexes.size());
       foreach(auto index, info.indexes)
       {
           if(checkIfUserStopSolving())
           {
               break;
           }

           mModelMutex.lockForWrite();

           sendFixingIssuesMessage(issueCounter, totalRows);

           auto potentialIndex = getSolveIssueIndex(index);
           auto issue(mStalledIssues.at(potentialIndex.row()));
           if(issue.getData())
           {
               if(issue.getData()->checkForExternalChanges())
               {
                   issuesExternallyChanged++;
               }
               else
               {
                   if(info.solveFunc && info.solveFunc(potentialIndex.row()))
                   {
                       if(!issue.getData()->isSolved() && !issue.getData()->isBeingSolved())
                       {
                           issue.getData()->setIsSolved(StalledIssue::SolveType::SOLVED);
                       }
                       issuesFixed++;
                       issueSolved(issue);
                   }
               }
           }
           issueCounter++;

           mModelMutex.unlock();
       }

       if(!info.async)
       {
           bool sendMessage(true);

           if(issuesFixed == 0)
           {
               sendMessage = false;
               unBlockUi();

               if(issuesExternallyChanged > 0)
               {
                   issuesFixed = issuesExternallyChanged;
                   showIssueExternallyChangedMessageBox();
               }
           }

           if(info.finishFunc)
           {
               info.finishFunc(issuesFixed, issuesExternallyChanged > 0);
           }

           finishSolvingIssues(issuesFixed, sendMessage, info.solveMessage);
       }
       else if(issuesExternallyChanged > 0)
       {
           showIssueExternallyChangedMessageBox();
           finishSolvingIssues(0, false, info.solveMessage);
       }
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
    msgInfo.finishFunc = [this](QPointer<QMessageBox>){
        updateStalledIssues();
    };

    runMessageBox(std::move(msgInfo));
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
void StalledIssuesModel::issueSolved(const StalledIssueVariant& issue)
{
    if(issue.consultData()->isSolved() && !issue.consultData()->isPotentiallySolved())
    {
        mLastSolvedStalledIssue = issue;
        mSolvedStalledIssues.append(issue);
        mCountByFilterCriterion[static_cast<int>(StalledIssueFilterCriterion::SOLVED_CONFLICTS)]++;
        auto& counter = mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason(issue.consultData()->getReason()))];
        if(counter > 0)
        {
            counter--;
        }
    }
}

void StalledIssuesModel::solveAllIssues()
{
    auto resolveIssue = [this](int row) -> bool
    {
        auto item = mStalledIssues.at(row);
        if(item.consultData()->isAutoSolvable())
        {
            return item.getData()->autoSolveIssue();
        }
        return false;
    };

    QModelIndexList list;
    auto totalRows(rowCount(QModelIndex()));
    for(int row = 0; row < totalRows; ++row)
    {
        list.append(index(row,0));
    }

    SolveListInfo info(list, resolveIssue);
    solveListOfIssues(info);
}

void StalledIssuesModel::chooseSideManually(bool remote, const QModelIndexList& list)
{
    auto resolveIssue = [this, remote](int row) -> bool
    {
        auto item = mStalledIssues.at(row);
        if(item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose ||
           item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose)
        {
            if(auto issue = item.convert<LocalOrRemoteUserMustChooseStalledIssue>())
            {
                remote ? issue->chooseRemoteSide() : issue->chooseLocalSide();
                if(item.consultData()->isSolved())
                {
                    MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_LOCALREMOTE_SOLVED_MANUALLY);
                    return true;
                }
            }
        }

        return false;
    };

    SolveListInfo info(list, resolveIssue);
    solveListOfIssues(info);
}

void StalledIssuesModel::chooseBothSides(const QModelIndexList& list)
{
    std::shared_ptr<QStringList> namesUsed(new QStringList());

    auto resolveIssue = [this, namesUsed](int row) -> bool
    {
        auto item = mStalledIssues.at(row);
        if(item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose ||
            item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose)
        {
            if(auto issue = item.convert<LocalOrRemoteUserMustChooseStalledIssue>())
            {
                issue->chooseBothSides(namesUsed.get());
            }

            if(item.consultData()->isSolved())
            {
                MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_LOCALREMOTE_SOLVED_MANUALLY);
                return true;
            }
        }

        return false;
    };

    SolveListInfo info(list, resolveIssue);
    solveListOfIssues(info);
}


void StalledIssuesModel::chooseRemoteForBackups(const QModelIndexList& list)
{
    mSyncsToDisable.clear();

    auto resolveIssue = [this](int row) -> bool
    {
        auto item = mStalledIssues.at(row);
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
                return true;
            }
        }

        return false;
    };

    auto finishFunc = [this](int, bool)
    {
        foreach(auto& sync, mSyncsToDisable)
        {
            SyncController controller;
            controller.setSyncToDisabled(sync);
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
        auto issue(mStalledIssues.at(row));
        auto localRemoteIssue = issue.convert<LocalOrRemoteUserMustChooseStalledIssue>();
        if(localRemoteIssue)
        {
            localRemoteIssue->chooseLastMTimeSide();

            if(localRemoteIssue->isSolved())
            {
                MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_LOCALREMOTE_SOLVED_SEMI_AUTOMATICALLY);
                return true;
            }
        }
        return false;
    };

    SolveListInfo info(list, resolveIssue);
    solveListOfIssues(info);
}

void StalledIssuesModel::ignoreItems(const QModelIndexList& list, bool isSymLink)
{
    mIgnoredItemsBySync.clear();

    auto resolveIssue = [this](int row) -> bool
    {
        auto item = mStalledIssues.at(row);
        if(!item.getData()->syncIds().isEmpty())
        {
            std::shared_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(item.getData()->firstSyncId()));
            if(sync)
            {
                auto folderPath(QDir::toNativeSeparators(QString::fromUtf8(sync->getLocalFolder())));
                if(MegaIgnoreManager::isValid(folderPath))
                {
                    auto& items = mIgnoredItemsBySync[item.getData()->firstSyncId()];
                    auto ignoredItems = item.getData()->getIgnoredFiles();
                    foreach(auto& ignoredItem, ignoredItems)
                    {
                        if(!items.contains(ignoredItem))
                        {
                            items.append(ignoredItem);
                            MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_IGNORE_SOLVED_MANUALLY);
                        }
                    }

                    return true;
                }
            }
        }

        return false;
    };

    QModelIndexList auxList(list);
    if(auxList.isEmpty())
    {
        auto totalRows(rowCount(QModelIndex()));
        for(int row = 0; row < totalRows; ++row)
        {
            auto item = getStalledIssueByRow(row);
            auto isCompatible(isSymLink ? item.getData()->isSymLink()
                                        : (item.getData()->canBeIgnored() && !item.getData()->isSymLink()));
            if(!item.getData()->isSolved() &&
               isCompatible)
            {
                auxList.append(index(row,0));
            }
        }
    }

    auto issuesToFix(auxList.size());

    auto finishFunc = [this, isSymLink, issuesToFix](int issuesFixed, bool externallyModified)
    {
        foreach(auto syncId,  mIgnoredItemsBySync.keys())
        {
            std::shared_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(syncId));
            if(sync)
            {
                auto folderPath(QDir::toNativeSeparators(QString::fromUtf8(sync->getLocalFolder())));
                MegaIgnoreManager manager(folderPath, false);

                QDir dir(folderPath);

                auto ignoredFiles(mIgnoredItemsBySync.value(syncId));
                foreach(auto file, ignoredFiles)
                {
                    isSymLink ? manager.addIgnoreSymLinkRule(dir.relativeFilePath(file))
                              : manager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE, dir.relativeFilePath(file));
                }

                manager.applyChanges();
            }
        }

        if(!externallyModified && issuesFixed < issuesToFix)
        {
            showIgnoreItemsError(issuesFixed == 0);
        }
    };


    SolveListInfo info(auxList, resolveIssue);
    info.finishFunc = finishFunc;
    solveListOfIssues(info);
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
        if(item.getData()->isSymLink() &&
           !item.getData()->isSolved())
        {
            if(!item.getData()->syncIds().isEmpty())
            {
                auto syncId(item.getData()->firstSyncId());
                if(!involvedSyncs.contains(syncId))
                {
                    involvedSyncs.append(syncId);

                    std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(syncId));
                    if(sync)
                    {
                        auto folderPath(QDir::toNativeSeparators(QString::fromUtf8(sync->getLocalFolder())));
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

            list.append(index(row,0));
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
        auto item = mStalledIssues.at(row);
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
        auto issue(mStalledIssues.at(row));
        mFingerprintIssuesToFix.append(issue);

        MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_FINGERPRINT_MISSING_SOLVED_MANUALLY);
        return true;
    };

    SolveListInfo info(list, resolveIssue);
    info.finishFunc = finishIssue;
    solveListOfIssues(info);
}

void StalledIssuesModel::fixMoveOrRenameCannotOccur(const QModelIndexList& indexes, MoveOrRenameIssueChosenSide side)
{
    auto resolveIssue = [this, side](int row) -> bool
    {
        auto issue(mStalledIssues.at(row));
        if(issue.consultData()->syncIds().isEmpty())
        {
            return false;
        }

        if(auto moveOrRemoveIssue = std::dynamic_pointer_cast<MoveOrRenameCannotOccurIssue>(issue.getData()))
        {
            auto solver(new MoveOrRenameMultiStepIssueSolver(moveOrRemoveIssue));

            DesktopNotifications::NotificationInfo startNotificationInfo;
            startNotificationInfo.title = tr("Sync stalls");
            startNotificationInfo.message = tr("(TEMPORARY) We have started solving your issues for sync X");
            solver->setStartNotification(startNotificationInfo);

            DesktopNotifications::NotificationInfo finishNotificationInfo;
            finishNotificationInfo.title = tr("Sync stalls");
            finishNotificationInfo.message = tr("(TEMPORARY) We have finished solving your issues for sync X");
            //Add action to run the widget to show the errors
            solver->setFinishNotification(finishNotificationInfo);

            mStalledIssuesReceiver->addMultiStepIssueSolver<MoveOrRenameCannotOccurIssue>(
                mega::MegaSyncStall::MoveOrRenameCannotOccur, solver);
            moveOrRemoveIssue->setIsSolved(StalledIssue::SolveType::BEING_SOLVED);
            moveOrRemoveIssue->solveIssue(side);

        }
        return true;
    };

    SolveListInfo info(indexes, resolveIssue);
    info.async = true;
    solveListOfIssues(info);
}

void StalledIssuesModel::semiAutoSolveNameConflictIssues(const QModelIndexList& list, int option)
{
    auto resolveIssue = [this, option](int row) -> bool
    {
        auto item = mStalledIssues.at(row);
        if(!item.getData()->checkForExternalChanges())
        {
            if(item.consultData()->getReason() == mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced)
            {
                if(auto nameConflict = item.convert<NameConflictedStalledIssue>())
                {
                    nameConflict->semiAutoSolveIssue(option);
                    if(item.consultData()->isSolved())
                    {
                        MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_NAMECONFLICT_SOLVED_SEMI_AUTOMATICALLY);
                        return true;
                    }
                }
            }
        }

        return false;
    };

    SolveListInfo info(list, resolveIssue);
    solveListOfIssues(info);
}

bool StalledIssuesModel::solveLocalConflictedNameByRemove(int conflictIndex, const QModelIndex& index)
{
    auto result(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue.convert<NameConflictedStalledIssue>())
    {
        result = nameConflict->solveLocalConflictedNameByRemove(conflictIndex);
        issueSolved(issue);

        if(nameConflict->isSolved())
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_NAMECONFLICT_SOLVED_MANUALLY);
            finishConflictManually();
        }

    }

    return result;
}

bool StalledIssuesModel::solveLocalConflictedNameByRename(const QString& renameTo, int conflictIndex, const QModelIndex& index)
{
    auto result(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue.convert<NameConflictedStalledIssue>())
    {
        result = nameConflict->solveLocalConflictedNameByRename(conflictIndex, renameTo);
        issueSolved(issue);

        if(nameConflict->isSolved())
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_NAMECONFLICT_SOLVED_MANUALLY);
            finishConflictManually();
        }

    }

    return result;
}

bool StalledIssuesModel::checkForExternalChanges(const QModelIndex& index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    return issue.getData()->checkForExternalChanges();
}

bool StalledIssuesModel::solveCloudConflictedNameByRemove(int conflictIndex, const QModelIndex& index)
{
    auto result(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue.convert<NameConflictedStalledIssue>())
    {
        result = nameConflict->solveCloudConflictedNameByRemove(conflictIndex);
        issueSolved(issue);

        if(nameConflict->isSolved())
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_NAMECONFLICT_SOLVED_MANUALLY);
            finishConflictManually();
        }

    }

    return result;
}

bool StalledIssuesModel::solveCloudConflictedNameByRename(const QString& renameTo, int conflictIndex, const QModelIndex& index)
{
    auto result(false);

    auto potentialIndex = getSolveIssueIndex(index);

    auto issue(mStalledIssues.at(potentialIndex.row()));
    if(auto nameConflict = issue.convert<NameConflictedStalledIssue>())
    {
        result = nameConflict->solveCloudConflictedNameByRename(conflictIndex, renameTo);
        issueSolved(issue);

        if(nameConflict->isSolved())
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_NAMECONFLICT_SOLVED_MANUALLY);
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
