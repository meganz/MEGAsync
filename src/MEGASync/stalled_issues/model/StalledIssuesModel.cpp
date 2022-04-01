#include "StalledIssuesModel.h"

#include "MegaApplication.h"

StalledIssuesReceiver::StalledIssuesReceiver(QObject *parent) : QObject(parent), mega::MegaTransferListener()
{

}

void StalledIssuesReceiver::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
{
    if (auto ptr = request->getMegaSyncProblems())
    {
        std::string problemText;

        if (mega::MegaSyncNameConflictList* cl = ptr->nameConflicts())
        {
            for (int i = 0; i < cl->size(); ++i)
            {
                mega::MegaSyncNameConflict* c = cl->get(i);

                problemText += "Conflicting names.\n";
                if (mega::MegaStringList* cn = c->cloudNames())
                {
                    for (int j = 0; j < cn->size(); ++j)
                    {
                        problemText += "  " + std::string(cn->get(j)) + "\n";
                    }
                }
                if (auto cp = c->cloudPath())
                {
                    if (cp && *cp)
                    {
                        problemText += "  at cloud path: " + std::string(cp) + "\n";
                    }
                }
                if (mega::MegaStringList* ln = c->localNames())
                {
                    for (int j = 0; j < ln->size(); ++j)
                    {
                        problemText += "  " + std::string(ln->get(j)) + "\n";
                    }
                }
                if (auto lp = c->localPath())
                {
                    if (lp && *lp)
                    {
                        problemText += "  at local path: " + std::string(lp) + "\n";
                    }
                }

                problemText += "\n";
            }
        }

        if (mega::MegaSyncStallList* sl = ptr->stalls())
        {
            for (int i = 0; i < sl->size(); ++i)
            {
                auto stall = sl->get(i);

                if (stall->isCloud())
                {
                    problemText += "Cloud side issue: " + std::string(stall->reasonString()) + "\n";
                    problemText += "  " + std::string(stall->indexPath()) + "\n";
                    if (stall->cloudPath() && strlen(stall->cloudPath()))
                    {
                        problemText += "  " + std::string(stall->cloudPath()) + "\n";
                    }
                    if (stall->localPath() && strlen(stall->localPath()))
                    {
                        problemText += "  Corresponding local path: " + std::string(stall->localPath()) + "\n";
                    }
                }
                else
                {
                    problemText += "Local side issue: " + std::string(stall->reasonString()) + "\n";
                    problemText += "  " + std::string(stall->indexPath()) + "\n";
                    if (stall->localPath() && strlen(stall->localPath()))
                    {
                        problemText += "  " + std::string(stall->localPath()) + "\n";
                    }
                    if (stall->cloudPath() && strlen(stall->cloudPath()))
                    {
                        problemText += "  Corresponding cloud path: " + std::string(stall->cloudPath()) + "\n";
                    }
                }

                problemText += "\n";
            }
        }
    }
}

StalledIssuesModel::StalledIssuesModel(QObject *parent) : QAbstractItemModel(parent),
     mMegaApi (MegaSyncApp->getMegaApi())
{
    mStalledIssuesThread = new QThread();
    mStalledIssuedReceiver = new StalledIssuesReceiver();
    mDelegateListener = new mega::QTMegaTransferListener(mMegaApi, mStalledIssuedReceiver);
    mStalledIssuedReceiver->moveToThread(mStalledIssuesThread);
    mDelegateListener->moveToThread(mStalledIssuesThread);
    mMegaApi->addTransferListener(mDelegateListener);

    mStalledIssuesThread->start();
}

StalledIssuesModel::~StalledIssuesModel()
{
    mMegaApi->removeTransferListener(mDelegateListener);
    mStalledIssuesThread->quit();
    mStalledIssuesThread->deleteLater();
    mStalledIssuedReceiver->deleteLater();

    mDelegateListener->deleteLater();
}

Qt::DropActions StalledIssuesModel::supportedDropActions() const
{
    return Qt::IgnoreAction;
}

bool StalledIssuesModel::hasChildren(const QModelIndex &parent) const
{
    if (!parent.isValid())
    {
        return !mStalledIssues.empty();
    }

    return false;
}

int StalledIssuesModel::rowCount(const QModelIndex &parent) const
{
   if(!parent.isValid())
   {
       return mStalledIssues.size();
   }

   return 0;
}

int StalledIssuesModel::columnCount(const QModelIndex &parent) const
{
   return 1;
}

QVariant StalledIssuesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if(index.parent().isValid())
        {
            return QVariant::fromValue(StalledIssue(mStalledIssues.at(index.parent().row())));
        }
        else
        {
            return QVariant::fromValue(StalledIssue(mStalledIssues.at(index.row())));
        }
    }

    return QVariant();
}

QModelIndex StalledIssuesModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }

    auto stalledIssueItem = static_cast<const QExplicitlySharedDataPointer<StalledIssueData>*>(index.internalPointer());
    if (!stalledIssueItem)
    {
        return QModelIndex();
    }

    //auto row = mStalledIssuesByOrder.value(stalledIssueItem,-1);
    //if(row >= 0)
    {
        //return createIndex(row, 0);
    }

    return QModelIndex();
}

QModelIndex StalledIssuesModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return createIndex(0, 0, mStalledIssues.value(parent.row()));
    }
    else
    {
        return (row < rowCount(QModelIndex())) ?  createIndex(row, column) : QModelIndex();
    }
}
