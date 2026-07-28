#ifndef SYMLINKSTALLEDISSUE_H
#define SYMLINKSTALLEDISSUE_H

#include "megaapi.h"
#include "MegaIgnoreRules.h"
#include "StalledIssue.h"

class IgnoredStalledIssue : public StalledIssue
{
    Q_OBJECT

public:
    IgnoredStalledIssue(const mega::MegaSyncStall *stallIssue);
    ~IgnoredStalledIssue() = default;

    ResolutionState autoSolveIssue() override;
    bool isAutoSolvable() const override;

    void fillIssue(const mega::MegaSyncStall *stall) override;

    bool isSymLink() const;
    bool isSpecialLink() const;
    bool isHardLink() const;

    // False when ignoring the issue could not possibly work, so the resolution is not offered
    // instead of being offered and then failing.
    bool canBeIgnored() const;

    bool isExpandable() const override;
    bool checkForExternalChanges(QObject* context) override;

    static void clearIgnoredSyncs();

    mega::MegaSyncStall::SyncPathProblem linkType() const;

    struct IgnoredPath
    {
        QString path;
        enum class IgnorePathSide
        {
            LOCAL,
            REMOTE,
        };
        IgnorePathSide pathSide;
        MegaIgnoreNameRule::Target target;

        IgnoredPath(const QString& newpath, IgnorePathSide side, MegaIgnoreNameRule::Target newtarget)
            :path(newpath), pathSide(side), target(newtarget)
        {}
    };
    QList<IgnoredPath> getIgnoredFiles() const { return mIgnoredPaths;}

protected:
    QList<IgnoredPath> mIgnoredPaths;

private:
    mega::MegaSyncStall::SyncPathProblem mLinkType;
    static QMap<mega::MegaHandle, bool> mSymLinksIgnoredInSyncs;
};

class CloudNodeIsBlockedIssue : public IgnoredStalledIssue
{
    Q_OBJECT

public:
    CloudNodeIsBlockedIssue(const mega::MegaSyncStall *stallIssue);
    ~CloudNodeIsBlockedIssue() = default;

    bool isAutoSolvable() const override;
    void fillIssue(const mega::MegaSyncStall *stall) override;
    bool showDirectoryInHyperlink() const override;
    bool isExpandable() const override;
};

#endif // SYMLINKSTALLEDISSUE_H
