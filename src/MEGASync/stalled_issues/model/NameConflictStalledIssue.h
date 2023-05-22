#ifndef NAMECONFLICTSTALLEDISSUE_H
#define NAMECONFLICTSTALLEDISSUE_H

#include <StalledIssue.h>
#include <MegaApplication.h>
#include <FileFolderAttributes.h>

class NameConflictedStalledIssue : public StalledIssue
{
public:
    struct ConflictedNameInfo
    {
        enum class SolvedType
        {
            REMOVE = 0,
            RENAME,
            SOLVED_BY_OTHER_SIDE,
            UNSOLVED
        };

        QString conflictedName;
        QString conflictedPath;
        QString renameTo;
        SolvedType solved;
        std::shared_ptr<FileFolderAttributes>  itemAttributes;

        ConflictedNameInfo(const QFileInfo& fileInfo, std::shared_ptr<FileFolderAttributes> attributes)
            :conflictedName(fileInfo.fileName()),
              conflictedPath(fileInfo.filePath()),
              solved(SolvedType::UNSOLVED),
              itemAttributes(attributes)
        {}

        bool operator==(const ConflictedNameInfo &data)
        {
            return conflictedName == data.conflictedName;
        }
        bool isSolved() const {return solved != SolvedType::UNSOLVED;}
    };

    struct NameConflictData
    {
        //Not const?? Depending on who is the liable to update the GUI -> From the SDK or from the MEGASync itself?
        StalledIssueDataPtr data;
        QList<ConflictedNameInfo> conflictedNames;
        bool isCloud;

        bool isEmpty() const { return conflictedNames.isEmpty();}
    };

    NameConflictedStalledIssue(){}
    NameConflictedStalledIssue(const NameConflictedStalledIssue& tdr);
    NameConflictedStalledIssue(const mega::MegaSyncStall *stallIssue);

    void fillIssue(const mega::MegaSyncStall *stall) override;

    NameConflictData getNameConflictLocalData() const;
    NameConflictData getNameConflictCloudData() const;

    bool solveLocalConflictedName(const QString& name, int conflictIndex, ConflictedNameInfo::SolvedType type);
    bool solveCloudConflictedName(const QString& name, int conflictIndex, ConflictedNameInfo::SolvedType type);

    bool solveCloudConflictedNameByRename(int conflictIndex, const QString& renameTo);
    bool solveLocalConflictedNameByRename(const QString& name, int conflictIndex, const QString& renameTo);

    static QStringList convertConflictedNames(bool cloud, const mega::MegaSyncStall *stall);

    void updateIssue(const mega::MegaSyncStall *stallIssue) override;

private:
    bool checkAndSolveConflictedNamesSolved(QList<ConflictedNameInfo>& conflicts);

    using StalledIssue::getLocalData;
    using StalledIssue::getCloudData;

    QList<ConflictedNameInfo> mCloudConflictedNames;
    QList<ConflictedNameInfo> mLocalConflictedNames;

};

Q_DECLARE_METATYPE(NameConflictedStalledIssue)

#endif // NAMECONFLICTSTALLEDISSUE_H
