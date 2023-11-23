#ifndef STALLEDISSUE_H
#define STALLEDISSUE_H

#include <FileFolderAttributes.h>

#include <megaapi.h>

#include <QSharedData>
#include <QObject>
#include <QFileInfo>
#include <QSize>
#include <QDebug>

#include <QFileSystemWatcher>

#include <memory>

enum class StalledIssueFilterCriterion
{
    ALL_ISSUES = 0,
    NAME_CONFLICTS,
    ITEM_TYPE_CONFLICTS,
    OTHER_CONFLICTS,
    SOLVED_CONFLICTS
};

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

    StalledIssueData();
    virtual ~StalledIssueData() = default;

    const Path& getPath() const;
    const Path& getMovePath() const;
    virtual bool isCloud() const {return false;}

    virtual bool isFile() const {return false; }

    QString getFilePath() const;
    QString getMoveFilePath() const;

    QString getNativeFilePath() const;
    QString getNativeMoveFilePath() const;

    QString getNativePath() const;
    QString getNativeMovePath() const;

    QString getFileName() const;

    const std::shared_ptr<const FileFolderAttributes> getAttributes() const {return mAttributes;}
    std::shared_ptr<FileFolderAttributes> getAttributes() {return mAttributes;}

    void checkTrailingSpaces(QString& name) const;

    template <class Type>
    QExplicitlySharedDataPointer<const Type> convert() const
    {
        return QExplicitlySharedDataPointer<const Type>(dynamic_cast<const Type*>(this));
    }

    virtual void initFileFolderAttributes()
    {}

protected:
    friend class StalledIssue;
    friend class NameConflictedStalledIssue;

    Path mMovePath;
    Path mPath;

    std::shared_ptr<FileFolderAttributes> mAttributes;
};

Q_DECLARE_TYPEINFO(StalledIssueData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(StalledIssueData)

using StalledIssueDataPtr = QExplicitlySharedDataPointer<const StalledIssueData>;
using StalledIssuesDataList = QList<StalledIssueDataPtr>;

Q_DECLARE_METATYPE(StalledIssueDataPtr)
Q_DECLARE_METATYPE(StalledIssuesDataList)

//CLOUD DATA
class CloudStalledIssueData : public StalledIssueData
{
public:



    CloudStalledIssueData()
        : StalledIssueData(),
          mPathHandle(mega::INVALID_HANDLE),
          mMovePathHandle(mega::INVALID_HANDLE)
    {}

    ~CloudStalledIssueData(){}

    bool isCloud() const override
    {
        return true;
    }

    bool isFile() const override
    {
        auto node(getNode());
        if(node)
        {
            return node->isFile();
        }

        return StalledIssueData::isFile();
    }

    std::shared_ptr<mega::MegaNode> getNode(bool refresh = false) const;

    void initFileFolderAttributes() override
    {
        if(mPathHandle != mega::INVALID_HANDLE)
        {
            mAttributes = std::make_shared<RemoteFileFolderAttributes>(mPathHandle, nullptr, false);
        }
        else
        {
            mAttributes = std::make_shared<RemoteFileFolderAttributes>(mPath.path, nullptr, false);
        }
    }

    std::shared_ptr<RemoteFileFolderAttributes> getFileFolderAttributes() const
    {
        return std::dynamic_pointer_cast<RemoteFileFolderAttributes>(mAttributes);
    }

    mega::MegaHandle getPathHandle() const;
    mega::MegaHandle getMovePathHandle() const;

    void setPathHandle(mega::MegaHandle newPathHandle);

private:
    friend class StalledIssue;
    friend class NameConflictedStalledIssue;

    mutable std::shared_ptr<mega::MegaNode> mRemoteNode;

    mega::MegaHandle mPathHandle;
    mega::MegaHandle mMovePathHandle;
};

Q_DECLARE_TYPEINFO(CloudStalledIssueData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(CloudStalledIssueData)

using CloudStalledIssueDataPtr = QExplicitlySharedDataPointer<const CloudStalledIssueData>;
using CloudStalledIssuesDataList = QList<CloudStalledIssueDataPtr>;

Q_DECLARE_METATYPE(CloudStalledIssueDataPtr)
Q_DECLARE_METATYPE(CloudStalledIssuesDataList)

//LOCAL DATA
class LocalStalledIssueData : public StalledIssueData
{
public:

    LocalStalledIssueData()
        : StalledIssueData()
    {

    }
    ~LocalStalledIssueData(){}

    bool isCloud() const override
    {
        return false;
    }

    bool isFile() const override
    {
        QFileInfo info(mPath.path);
        if(info.exists())
        {
            return info.isFile();
        }

        return StalledIssueData::isFile();
    }

    void initFileFolderAttributes() override
    {
        mAttributes = std::make_shared<LocalFileFolderAttributes>(mPath.path, nullptr);
    }

    std::shared_ptr<LocalFileFolderAttributes> getFileFolderAttributes() const
    {
        return std::dynamic_pointer_cast<LocalFileFolderAttributes>(mAttributes);
    }
};

Q_DECLARE_TYPEINFO(LocalStalledIssueData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(LocalStalledIssueData)

using LocalStalledIssueDataPtr = QExplicitlySharedDataPointer<const LocalStalledIssueData>;
using LocalStalledIssueDataList = QList<LocalStalledIssueDataPtr>;

Q_DECLARE_METATYPE(LocalStalledIssueDataPtr)
Q_DECLARE_METATYPE(LocalStalledIssueDataList)

struct UploadTransferInfo;
struct DownloadTransferInfo;

class StalledIssue
{
    class FileSystemSignalHandler : public QObject
    {
    public:
        FileSystemSignalHandler(StalledIssue* issue)
            :mIssue(issue)
        {}

        ~FileSystemSignalHandler()
        {}

        void createFileWatcher()
        {
            if(!mFileWatcher)
            {
                auto deleter = [](QFileSystemWatcher* object){
                    object->deleteLater();
                };

                mFileWatcher = std::shared_ptr<QFileSystemWatcher>(new QFileSystemWatcher(mIssue->getLocalFiles()), deleter);
                connect(mFileWatcher.get(), &QFileSystemWatcher::fileChanged, this, [this](const QString&){
                    mIssue->resetUIUpdated();
#ifdef Q_OS_LINUX
                    auto paths = mIssue->getLocalFiles();
                    foreach(auto& path, paths)
                    {
                        if (QFile::exists(path))
                        {
                            mFileWatcher->addPath(path);
                        }
                    }
#endif
                });
            }
        }

        void removeFileWatcher()
        {
            mFileWatcher.reset();
        }

    private:
        StalledIssue* mIssue;
        std::shared_ptr<QFileSystemWatcher> mFileWatcher;
    };

public:
    StalledIssue(const mega::MegaSyncStall *stallIssue);
    virtual ~StalledIssue(){}

    const LocalStalledIssueDataPtr consultLocalData() const;
    const CloudStalledIssueDataPtr consultCloudData() const;

    const QExplicitlySharedDataPointer<LocalStalledIssueData>& getLocalData();
    const QExplicitlySharedDataPointer<CloudStalledIssueData>& getCloudData();

    virtual bool containsHandle(mega::MegaHandle handle){return getCloudData() && getCloudData()->getPathHandle() == handle;}
    virtual void updateHandle(mega::MegaHandle handle){if(getCloudData()){getCloudData()->setPathHandle(handle);}}
    virtual void updateName(){}

    virtual bool checkForExternalChanges();

    mega::MegaSyncStall::SyncStallReason getReason() const;
    QString getFileName(bool preferCloud) const;
    static StalledIssueFilterCriterion getCriterionByReason(mega::MegaSyncStall::SyncStallReason reason);

    bool operator==(const StalledIssue &data);

    virtual void updateIssue(const mega::MegaSyncStall *stallIssue);

    enum SolveType
    {
        Unsolved,
        Solved,
        PotentiallySolved
    };

    bool isSolved() const;
    bool isPotentiallySolved() const;
    void setIsSolved(bool potentially);
    virtual bool autoSolveIssue(){return false;}
    virtual bool isSolvable() const {return false;}
    bool isBeingSolvedByUpload(std::shared_ptr<UploadTransferInfo> info) const;
    bool isBeingSolvedByDownload(std::shared_ptr<DownloadTransferInfo> info) const;

    bool isSymLink() const;
    bool missingFingerprint() const;
    bool canBeIgnored() const;
    virtual QStringList getLocalFiles();
    QStringList getIgnoredFiles() const;

    bool isUndecrypted() const;

    bool mDetectedMEGASide = false;

    bool isFile() const;
    uint8_t filesCount() const;
    uint8_t foldersCount() const;

    enum Type
    {
        Header = 0,
        Body
    };

    QSize getDelegateSize(Type type) const;
    void setDelegateSize(const QSize &newDelegateSize, Type type);
    void removeDelegateSize(Type type);

    const std::shared_ptr<mega::MegaSyncStall> &getOriginalStall() const;

    virtual void fillIssue(const mega::MegaSyncStall *stall);
    virtual void endFillingIssue();

    template <class Type>
    static const std::shared_ptr<const Type> convert(const std::shared_ptr<const StalledIssue> data)
    {
        return std::dynamic_pointer_cast<const Type>(data);
    }

    bool needsUIUpdate(Type type) const;
    void UIUpdated(Type type);
    void resetUIUpdated();
    virtual bool UIShowFileAttributes() const;
    void createFileWatcher();
    void removeFileWatcher();

    QList<mega::MegaHandle> syncIds() const;
    //In case there are two syncs, use the first one
    mega::MegaSync::SyncType getSyncType() const;

protected:
    bool initLocalIssue(const mega::MegaSyncStall *stallIssue);
    QExplicitlySharedDataPointer<LocalStalledIssueData> mLocalData;

    bool initCloudIssue(const mega::MegaSyncStall *stallIssue);
    QExplicitlySharedDataPointer<CloudStalledIssueData> mCloudData;

    void fillSyncId(const QString &path, bool cloud);

    void setIsFile(const QString& path, bool isLocal);

    std::shared_ptr<mega::MegaSyncStall> originalStall;
    mega::MegaSyncStall::SyncStallReason mReason = mega::MegaSyncStall::SyncStallReason::NoReason;
    QList<mega::MegaHandle> mSyncIds;
    mutable SolveType mIsSolved = SolveType::Unsolved;
    uint8_t mFiles = 0;
    uint8_t mFolders = 0;
    QStringList mIgnoredPaths;
    QSize mHeaderDelegateSize;
    QSize mBodyDelegateSize;
    QPair<bool, bool> mNeedsUIUpdate = qMakePair(false, false);
    std::shared_ptr<FileSystemSignalHandler> mFileSystemWatcher;
};

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

    QSize getDelegateSize(StalledIssue::Type type) const
    {
        return mData->getDelegateSize(type);
    }
    void setDelegateSize(const QSize &newDelegateSize, StalledIssue::Type type)
    {
        mData->setDelegateSize(newDelegateSize, type);
    }
    void removeDelegateSize(StalledIssue::Type type)
    {
        mData->removeDelegateSize(type);
    }

    template <class Type>
    const std::shared_ptr<const Type> convert() const
    {
        return StalledIssue::convert<Type>(mData);
    }

private:
    friend class StalledIssuesModel;
    friend class StalledIssuesReceiver;

    std::shared_ptr<StalledIssue> &getData()
    {
        return mData;
    }

    template <class Type>
    std::shared_ptr<Type> convert()
    {
        return std::dynamic_pointer_cast<Type>(mData);
    }

   std::shared_ptr<StalledIssue> mData;
};

Q_DECLARE_METATYPE(StalledIssueVariant)

using StalledIssuesVariantList = QList<StalledIssueVariant>;
Q_DECLARE_METATYPE(StalledIssuesVariantList)

#endif // STALLEDISSUE_H
