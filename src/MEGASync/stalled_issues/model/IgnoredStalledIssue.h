#ifndef SYMLINKSTALLEDISSUE_H
#define SYMLINKSTALLEDISSUE_H

#include <StalledIssue.h>
#include <syncs/control/MegaIgnoreRules.h>
#include <megaapi.h>

class IgnoredStalledIssue : public StalledIssue
{
public:
    IgnoredStalledIssue(const mega::MegaSyncStall *stallIssue);
    ~IgnoredStalledIssue(){}

    bool autoSolveIssue() override;
    bool isAutoSolvable() const override;

    void fillIssue(const mega::MegaSyncStall *stall) override;

    bool isSymLink() const;
    bool isSpecialLink() const;
    bool isHardLink() const;

    bool isExpandable() const override;
    bool checkForExternalChanges() override;

    static void clearIgnoredSyncs();

    mega::MegaSyncStall::SyncPathProblem linkType() const;

    struct IgnoredPath
    {
        QString path;
        bool cloud;
        MegaIgnoreNameRule::Target target;

        IgnoredPath(const QString& newpath, bool newcloud, MegaIgnoreNameRule::Target newtarget)
            :path(newpath), cloud(newcloud), target(newtarget)
        {}
    };
    QList<IgnoredPath> getIgnoredFiles() const { return mIgnoredPaths;}

private:
    QList<IgnoredPath> mIgnoredPaths;
    mega::MegaSyncStall::SyncPathProblem mLinkType;
    static QMap<mega::MegaHandle, bool> mSymLinksIgnoredInSyncs;
};

#endif // SYMLINKSTALLEDISSUE_H
