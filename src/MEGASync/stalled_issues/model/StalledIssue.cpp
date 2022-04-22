#include "StalledIssue.h"

#include "MegaApplication.h"

StalledIssueData::StalledIssueData(const mega::MegaSyncStall *stallIssue)
    : mIsCloud(false)
    , mIsImmediate(false)
    , mIsMissing(false)
    , mIsBlocked(false)
{
    update(stallIssue);

    qRegisterMetaType<StalledIssueDataPtr>("StalledIssueDataPtr");
    qRegisterMetaType<StalledIssuesDataList>("StalledIssuesDataList");
    qRegisterMetaType<StalledIssuesList>("StalledIssuesList");
}

void StalledIssueData::update(const mega::MegaSyncStall *stallIssue)
{
    if(stallIssue)
    {
        mIndexPath    = QString::fromUtf8(stallIssue->indexPath());
        mLocalPath    = QString::fromUtf8(stallIssue->localPath());
        mCloudPath    = QString::fromUtf8(stallIssue->cloudPath());
        mIsCloud      = stallIssue->isCloud();
        mIsImmediate  = stallIssue->isImmediate();
        mReasonString = QString::fromUtf8(stallIssue->reasonString());
    }
}

bool StalledIssueData::hasMoveInfo() const
{
    return !mMovePath.isEmpty();
}

//Conflicted Names stalled issue
ConflictedNamesStalledIssue::ConflictedNamesStalledIssue()
    : StalledIssue()
{
    mIsNameConflict = true;
}

ConflictedNamesStalledIssue::ConflictedNamesStalledIssue(const QExplicitlySharedDataPointer<StalledIssueData> &tdr)
{
    mIsNameConflict = true;
    d.append(tdr);
}


ConflictedNamesStalledIssue::ConflictedNamesStalledIssue(mega::MegaSyncNameConflict *nameConflictStallIssue)
    : StalledIssue()
{
    mIsNameConflict = true;
    auto data = StalledIssueDataPtr(new StalledIssueData());
    d.append(data);
    update(nameConflictStallIssue);
}

QStringList ConflictedNamesStalledIssue::localNames() const
{
    return mLocalNames;
}

QStringList ConflictedNamesStalledIssue::cloudNames() const
{
    return mCloudNames;
}

void ConflictedNamesStalledIssue::update(const mega::MegaSyncNameConflict* nameConflictStallIssue)
{
    if (mega::MegaStringList* cn = nameConflictStallIssue->cloudNames())
    {
        for (int j = 0; j < cn->size(); ++j)
        {
            mCloudNames.append(QString::fromUtf8(cn->get(j)));
        }
    }
    if (auto cp = nameConflictStallIssue->cloudPath())
    {
        if (cp && *cp)
        {
            getStalledIssueData()->mCloudPath = QString::fromUtf8(cp);
        }
    }
    if (mega::MegaStringList* ln = nameConflictStallIssue->localNames())
    {
        for (int j = 0; j < ln->size(); ++j)
        {
            mLocalNames.append(QString::fromUtf8(ln->get(j)));
        }
    }
    if (auto lp = nameConflictStallIssue->localPath())
    {
        if (lp && *lp)
        {
            getStalledIssueData()->mLocalPath = QString::fromUtf8(lp);
        }
    }

    mIsNameConflict = true;
}

StalledIssue::StalledIssue(const QExplicitlySharedDataPointer<StalledIssueData> &tdr, mega::MegaSyncStall::SyncStallReason reason)
{
    d.append(tdr);
    extractFileName(tdr);
    mReason = reason;
}

StalledIssue::StalledIssue(const QList<QExplicitlySharedDataPointer<StalledIssueData> > &tdr, mega::MegaSyncStall::SyncStallReason reason)
{
    d = tdr;
    if(!tdr.isEmpty())
    {
       extractFileName(tdr.first());
    }
    mReason = reason;
}

void StalledIssue::addStalledIssueData(QExplicitlySharedDataPointer<StalledIssueData> data)
{
    d.append(data);
}

void StalledIssue::extractFileName(const QExplicitlySharedDataPointer<StalledIssueData> &tdr)
{
    if(tdr->mIsCloud)
    {
        auto splittedIndexPath = tdr->mIndexPath.split(QString::fromUtf8("/"));
        mFileName = splittedIndexPath.last();
    }
    else
    {
        auto splittedIndexPath = tdr->mIndexPath.split(QString::fromUtf8("\\"));
        mFileName = splittedIndexPath.last();
    }
}

QExplicitlySharedDataPointer<StalledIssueData> StalledIssue::getStalledIssueData(int index) const
{
    return d.size() > index ? d.at(index) : QExplicitlySharedDataPointer<StalledIssueData>();
}

int StalledIssue::stalledIssuesCount() const
{
    return d.size();
}

mega::MegaSyncStall::SyncStallReason StalledIssue::getReason() const
{
    return mReason;
}

const QString& StalledIssue::getFileName() const
{
    return mFileName;
}

bool StalledIssue::isNameConflict() const
{
    return mIsNameConflict;
}

bool StalledIssue::operator==(const StalledIssue &data)
{
    if(data.stalledIssuesCount() != stalledIssuesCount())
    {
        return false;
    }

    for(int index = 0; index < stalledIssuesCount(); ++index)
    {
       if(getStalledIssueData(index) != data.getStalledIssueData(index))
       {
           return false;
       }
    }

    return true;
}
