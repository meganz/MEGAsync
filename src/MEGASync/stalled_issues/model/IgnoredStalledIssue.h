#ifndef SYMLINKSTALLEDISSUE_H
#define SYMLINKSTALLEDISSUE_H

#include "megaapi.h"
#include "MegaIgnoreRules.h"
#include "StalledIssue.h"

#include <optional>

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

    // False when the item's name cannot be written as an exclusion, so the resolution is not
    // offered instead of being offered and then failing. It answers only that question: when the
    // sync is not available the name is not what stands in the way, and the attempt is left to
    // fail through the usual paths.
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

    QList<IgnoredPath> getIgnoredFiles() const
    {
        return mIgnoredPaths;
    }

    // The pattern the rule for this path is written with: relative to the sync root, as the
    // .megaignore in that root needs it. The only string whose writability is worth testing, so
    // every caller that writes one of these rules builds it from here.
    static QString ignoreRulePattern(const IgnoredPath& ignoredPath, const mega::MegaSync& sync);

protected:
    QList<IgnoredPath> mIgnoredPaths;

private:
    mega::MegaSyncStall::SyncPathProblem mLinkType;
    // Decided from mIgnoredPaths, so it is reset whenever those are filled again. Unset while the
    // sync it needs is not available.
    mutable std::optional<bool> mCanBeIgnored;
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
