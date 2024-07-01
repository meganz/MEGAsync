#ifndef SYMLINKSTALLEDISSUE_H
#define SYMLINKSTALLEDISSUE_H

#include <StalledIssue.h>
#include <megaapi.h>

class IgnoredStalledIssue : public StalledIssue
{
public:
    IgnoredStalledIssue(const mega::MegaSyncStall *stallIssue);
    ~IgnoredStalledIssue(){}

    bool autoSolveIssue() override;
    bool isAutoSolvable() const override;

    bool isSymLink() const override;
    bool isSpecialLink() const override;

    bool checkForExternalChanges() override;

    static void clearIgnoredSyncs();

private:
    static QMap<mega::MegaHandle, bool> mSymLinksIgnoredInSyncs;
};

#endif // SYMLINKSTALLEDISSUE_H
