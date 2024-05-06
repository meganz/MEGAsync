#ifndef STALLEDISSUEFACTORY_H
#define STALLEDISSUEFACTORY_H

#include <StalledIssue.h>
#include <AutoRefreshStalledIssuesByCondition.h>

class MoveOrRenameCannotOccurFactory;

class StalledIssuesFactory
{
public:
    StalledIssuesFactory(){}
    virtual ~StalledIssuesFactory() = default;

    virtual StalledIssueVariant createIssue(const mega::MegaSyncStall* stall) = 0;
    virtual void clear() = 0;
};

class StalledIssuesCreator : public QObject
{
        Q_OBJECT
public:
    StalledIssuesCreator();

    void createIssues(mega::MegaSyncStallList* issues, QPointer<AutoRefreshByConditionBase> autoRefreshDetector);

    StalledIssuesVariantList issues() const;

    void setMoveOrRenameSyncIdBeingFixed(const mega::MegaHandle& syncId, int chosenSide);
    void setIsEventRequest(bool newIsEventRequest);

signals:
    void solvingIssues(int current, int total);
    void moveOrRenameCannotOccurFound();

protected:
    void clear();

    StalledIssuesVariantList mIssues;

private:
    std::shared_ptr<MoveOrRenameCannotOccurFactory> mMoveOrRenameCannotOccurFactory;
    bool mIsEventRequest;
};

#endif // STALLEDISSUEFACTORY_H
