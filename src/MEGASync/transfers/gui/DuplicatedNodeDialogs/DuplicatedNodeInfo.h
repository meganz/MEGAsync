#ifndef DUPLICATEDNODEINFO_H
#define DUPLICATEDNODEINFO_H

#include <megaapi.h>

#include <QObject>
#include <QString>
#include <QDateTime>

#include <memory>

enum class NodeItemType
{
    FOLDER_UPLOAD_AND_MERGE =0,
    FILE_UPLOAD_AND_REPLACE,
    FILE_UPLOAD_AND_UPDATE,
    UPLOAD_AND_RENAME,
    DONT_UPLOAD,
    UPLOAD
};

class DuplicatedUploadBase;

class DuplicatedNodeInfo : public QObject
{

    Q_OBJECT

public:
    DuplicatedNodeInfo(DuplicatedUploadBase* checker);

    const std::shared_ptr<mega::MegaNode> &getParentNode() const;
    void setParentNode(const std::shared_ptr<mega::MegaNode> &newParentNode);

    const std::shared_ptr<mega::MegaNode> &getConflictNode() const;
    void setConflictNode(const std::shared_ptr<mega::MegaNode> &newRemoteConflictNode);

    const QString& getSourceItemPath() const;
    void setSourceItemPath(const QString &newLocalPath);

    NodeItemType getSolution() const;
    void setSolution(NodeItemType newSolution);

    const QString& getNewName();
    const QString& getDisplayNewName();
    const QString& getName() const;
    void setName(const QString &newName);
    void setNewName(const QString &newNewName);

    bool hasConflict() const;
    void setHasConflict(bool newHasConflict);

    virtual bool sourceItemIsFile() const;
    bool conflictNodeIsFile() const;

    const QDateTime& getNodeModifiedTime() const;

    const QDateTime& getSourceItemModifiedTime() const;
    void setSourceItemModifiedTime(const QDateTime& newSourceItemModifiedTime);

    bool haveDifferentType() const;

    bool isNameConflict() const;
    void setIsNameConflict(bool newIsNameConflict);

    DuplicatedUploadBase* checker() const;

protected:
    std::shared_ptr<mega::MegaNode> mParentNode;
    std::shared_ptr<mega::MegaNode> mConflictNode;
    QString mSourcePath;
    NodeItemType mSolution;
    QString mNewName;
    QString mDisplayNewName;
    QString mName;
    bool mSourceItemIsFile;
    bool mHasConflict;
    bool mIsNameConflict;
    QDateTime mConflictNodeModifiedTime;
    QDateTime mSourceItemModifiedTime;
    DuplicatedUploadBase* mChecker;
};

class DuplicatedMoveNodeInfo : public DuplicatedNodeInfo
{
    Q_OBJECT
public:

    DuplicatedMoveNodeInfo(DuplicatedUploadBase* checker)
        : DuplicatedNodeInfo(checker)
    {}

    mega::MegaHandle getSourceItemHandle() const;
    void setSourceItemHandle(const mega::MegaHandle& sourceItemHandle);

    bool sourceItemIsFile() const override;

private:
    std::shared_ptr<mega::MegaNode> mSourceItemNode;
};

struct ConflictTypes
{
    ConflictTypes() = default;

    std::shared_ptr<mega::MegaNode> mSourceNode = nullptr;
    std::shared_ptr<mega::MegaNode> mTargetNode = nullptr;

    DuplicatedUploadBase* mFolderCheck = nullptr;
    DuplicatedUploadBase* mFileCheck = nullptr;

    QList<std::shared_ptr<DuplicatedNodeInfo>> mResolvedConflicts;
    QList<std::shared_ptr<DuplicatedNodeInfo>> mFileConflicts;
    QList<std::shared_ptr<DuplicatedNodeInfo>> mFolderConflicts;
    QList<std::shared_ptr<DuplicatedNodeInfo>> mFileNameConflicts;
    QList<std::shared_ptr<DuplicatedNodeInfo>> mFolderNameConflicts;

    bool isEmpty() const;
};

class CheckDuplicatedNodes
{
public:
    static std::shared_ptr<ConflictTypes> checkMoves(QList<mega::MegaHandle> moveHandles,
                                                     std::shared_ptr<mega::MegaNode> sourceNode,
                                                     std::shared_ptr<mega::MegaNode> targetNode);
    static std::shared_ptr<ConflictTypes> checkUploads(
        QQueue<QString>& nodePaths, std::shared_ptr<mega::MegaNode> targetNode);
};

#endif // DUPLICATEDNODEINFO_H
