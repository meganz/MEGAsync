#ifndef STALLEDISSUE_H
#define STALLEDISSUE_H

#include <megaapi.h>

#include <QSharedData>
#include <QObject>
#include <QFileInfo>
#include <QSize>
#include <QDebug>

#include <memory>

enum class StalledIssueFilterCriterion
{
    ALL_ISSUES = 0,
    NAME_CONFLICTS,
    ITEM_TYPE_CONFLICTS,
    OTHER_CONFLICTS,
};

namespace UserAttributes{
class FullName;
}

class StalledIssueData : public QSharedData
{
public:
    struct Path
    {
        QString path;
        mega::MegaSyncStall::SyncPathProblem mPathProblem = mega::MegaSyncStall::SyncPathProblem::NoProblem;

        Path(){}
        bool isEmpty() const {return path.isEmpty() && mPathProblem == mega::MegaSyncStall::SyncPathProblem::NoProblem;}
    };

    StalledIssueData(std::unique_ptr<mega::MegaSyncStall> originalstall);
    StalledIssueData(const StalledIssueData&);
    StalledIssueData();
    ~StalledIssueData(){}

    const Path& getPath() const;
    const Path& getMovePath() const;
    bool isCloud() const;

    QString getFilePath() const;
    QString getMoveFilePath() const;

    QString getNativeFilePath() const;
    QString getNativeMoveFilePath() const;

    QString getNativePath() const;
    QString getNativeMovePath() const;

    QString getFileName() const;

    const QString& getUserFirstName() const;

    bool isEqual(const mega::MegaSyncStall *stall) const;

    bool isSolved() const;
    void setIsSolved(bool newIsSolved);

    void checkTrailingSpaces(QString& name) const;

    std::shared_ptr<mega::MegaNode> getNode() const;

    std::shared_ptr<mega::MegaSyncStall> original;

private:
    friend class StalledIssue;
    friend class NameConflictedStalledIssue;

    Path mMovePath;
    Path mPath;

    bool mIsCloud;
    bool mIsSolved;

    QString mUserEmail;
    std::shared_ptr<const UserAttributes::FullName> mUserFullName;

    mutable std::shared_ptr<mega::MegaNode> mRemoteNode;

};

Q_DECLARE_TYPEINFO(StalledIssueData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(StalledIssueData)

using StalledIssueDataPtr = QExplicitlySharedDataPointer<const StalledIssueData>;
using StalledIssuesDataList = QList<StalledIssueDataPtr>;

Q_DECLARE_METATYPE(StalledIssueDataPtr)
Q_DECLARE_METATYPE(StalledIssuesDataList)

class StalledIssue
{
public:
    StalledIssue(){}
    //StalledIssue(const StalledIssue& tdr) : mDetectedMEGASide(tdr.mDetectedMEGASide), mLocalData(tdr.mLocalData), mCloudData(tdr.mCloudData), mReason(tdr.getReason()), mIsSolved(tdr.mIsSolved)  {}
    StalledIssue(const mega::MegaSyncStall *stallIssue);

    const StalledIssueDataPtr consultLocalData() const;
    const StalledIssueDataPtr consultCloudData() const;

    mega::MegaSyncStall::SyncStallReason getReason() const;
    QString getFileName(bool preferCloud) const;
    static StalledIssueFilterCriterion getCriterionByReason(mega::MegaSyncStall::SyncStallReason reason);

    bool operator==(const StalledIssue &data);

    virtual void updateIssue(const mega::MegaSyncStall *stallIssue);

    bool isSolved() const;
    void setIsSolved(bool isCloud);

    bool canBeIgnored() const;
    QStringList getIgnoredFiles() const;

    bool mDetectedMEGASide = false;

    uint8_t hasFiles() const;
    uint8_t hasFolders() const;

    enum SizeType
    {
        Header = 0,
        Body
    };

    QSize getDelegateSize(SizeType type) const;
    void setDelegateSize(const QSize &newDelegateSize, SizeType type);

    const std::shared_ptr<mega::MegaSyncStall> &getOriginalStall() const;

protected:
    bool initCloudIssue(const mega::MegaSyncStall *stallIssue);
    const QExplicitlySharedDataPointer<StalledIssueData>& getLocalData() const;
    QExplicitlySharedDataPointer<StalledIssueData> mLocalData;

    bool initLocalIssue(const mega::MegaSyncStall *stallIssue);
    const QExplicitlySharedDataPointer<StalledIssueData>& getCloudData() const;
    QExplicitlySharedDataPointer<StalledIssueData> mCloudData;

    void setIsFile(const QString& path, bool isLocal);

    virtual void fillIssue(const mega::MegaSyncStall *stall);
    void endFillingIssue();

    std::shared_ptr<mega::MegaSyncStall> originalStall;
    mega::MegaSyncStall::SyncStallReason mReason = mega::MegaSyncStall::SyncStallReason::NoReason;
    bool mIsSolved = false;
    uint8_t mFiles = 0;
    uint8_t mFolders = 0;
    QStringList mIgnoredPaths;
    QSize mHeaderDelegateSize;
    QSize mBodyDelegateSize;
};

Q_DECLARE_METATYPE(StalledIssue)

class StalledIssueVariant
{
public:
    StalledIssueVariant(){}
    StalledIssueVariant(const StalledIssueVariant& tdr) : mData(tdr.mData) {}
    StalledIssueVariant(const std::shared_ptr<StalledIssue> data)
        : mData(data)
    {}

    const std::shared_ptr<const StalledIssue> consultData() const
    {
        return mData;
    }

    void updateData(const mega::MegaSyncStall *stallIssue)
    {
        mData->updateIssue(stallIssue);
    }

    void reset()
    {
        mData.reset();
    }

    bool operator==(const StalledIssueVariant &issue)
    {
        return issue.mData == this->mData;
    }

    StalledIssueVariant& operator=(const StalledIssueVariant& other) = default;

    QSize getDelegateSize(StalledIssue::SizeType type) const
    {
        return mData->getDelegateSize(type);
    }
    void setDelegateSize(const QSize &newDelegateSize, StalledIssue::SizeType type)
    {
        mData->setDelegateSize(newDelegateSize, type);
    }

private:
    friend class StalledIssuesModel;

    const std::shared_ptr<StalledIssue> &getData() const
    {
        return mData;
    }

    std::shared_ptr<StalledIssue> mData;
};

Q_DECLARE_METATYPE(StalledIssueVariant)

using StalledIssuesVariantList = QList<std::shared_ptr<StalledIssueVariant>>;
Q_DECLARE_METATYPE(StalledIssuesVariantList)

#endif // STALLEDISSUE_H
