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
    DONT_UPLOAD
};


class DuplicatedNodeInfo : public QObject
{

    Q_OBJECT

public:
    DuplicatedNodeInfo();

    std::shared_ptr<mega::MegaNode> checkNameNode(const QString& nodeName, std::shared_ptr<mega::MegaNode> parentNode);

    const std::shared_ptr<mega::MegaNode> &getParentNode() const;
    void setParentNode(const std::shared_ptr<mega::MegaNode> &newParentNode);

    const std::shared_ptr<mega::MegaNode> &getRemoteConflictNode() const;
    void setRemoteConflictNode(const std::shared_ptr<mega::MegaNode> &newRemoteConflictNode);

    const QString &getLocalPath() const;
    void setLocalPath(const QString &newLocalPath);

    NodeItemType getSolution() const;
    void setSolution(NodeItemType newSolution);

    const QString& getNewName() const;
    const QString& getName() const;

    bool hasConflict() const;
    void setHasConflict(bool newHasConflict);

    bool isLocalFile() const;
    bool isRemoteFile() const;

    const QDateTime& getNodeModifiedTime() const;

    const QDateTime &getLocalModifiedTime() const;
    void setLocalModifiedTime(const QDateTime &newLocalModifiedTime);

    bool haveDifferentType() const;

signals:
    void localModifiedDateUpdated();

private:
    std::shared_ptr<mega::MegaNode> mParentNode;
    std::shared_ptr<mega::MegaNode> mRemoteConflictNode;
    QString mLocalPath;
    NodeItemType mSolution;
    QString mNewName;
    QString mName;
    bool mIsLocalFile;
    bool mHasConflict;
    bool mHaveDifferentType;
    QDateTime mNodeModifiedTime;
    QDateTime mLocalModifiedTime;

    void initNewName();
};

#endif // DUPLICATEDNODEINFO_H
