#ifndef SYMLINKSTALLEDISSUE_H
#define SYMLINKSTALLEDISSUE_H

#include <StalledIssue.h>
#include <megaapi.h>

class IgnoredStalledIssue : public StalledIssue
{
public:
    IgnoredStalledIssue(const mega::MegaSyncStall *stallIssue);
    ~IgnoredStalledIssue() = default;

    bool autoSolveIssue() override;
    bool isAutoSolvable() const override;

    void fillIssue(const mega::MegaSyncStall *stall) override;

    bool isSymLink() const override;
    bool isSpecialLink() const override;

    bool isExpandable() const override;
    bool checkForExternalChanges() override;    

    static void clearIgnoredSyncs();

    struct IgnoredPath
    {
        QString path;
        enum class IgnorePathSide
        {
            LOCAL,
            REMOTE,
        };
        IgnorePathSide pathSide;

        IgnoredPath(const QString& newpath, IgnorePathSide side):path(newpath), pathSide(side)
        {}
    };
    QList<IgnoredPath> getIgnoredFiles() const { return mIgnoredPaths;}

protected:
    QList<IgnoredPath> mIgnoredPaths;

private:
    static QMap<mega::MegaHandle, bool> mSymLinksIgnoredInSyncs;
    mega::MegaSyncStall::SyncPathProblem mLinkType;
};

class CloudNodeIsBlockedIssue : public IgnoredStalledIssue
{
public:
    CloudNodeIsBlockedIssue(const mega::MegaSyncStall *stallIssue);
    ~CloudNodeIsBlockedIssue() = default;

    bool isAutoSolvable() const override;
    void fillIssue(const mega::MegaSyncStall *stall) override;
    bool showDirectoryInHyperlink() const override;
    bool isExpandable() const override;
};

#endif // SYMLINKSTALLEDISSUE_H
