#ifndef STALLEDISSUEFACTORY_H
#define STALLEDISSUEFACTORY_H

#include <StalledIssue.h>
#include <MultiStepIssueSolver.h>

class MoveOrRenameCannotOccurFactory;

class StalledIssuesFactory
{
public:
    StalledIssuesFactory(){}
    virtual ~StalledIssuesFactory() = default;

    virtual StalledIssueVariant createIssue(const mega::MegaSyncStall* stall) = 0;
    virtual void clear() = 0;
};

enum class UpdateType
{
    NONE,
    UI,
    EVENT,
    AUTO_SOLVE
};

class StalledIssuesCreator : public QObject
{
        Q_OBJECT
public:
    StalledIssuesCreator();

    void createIssues(mega::MegaSyncStallList* issues, UpdateType updateType, QPointer<MultiStepIssueSolverBase> multiStepIssueSolver);

    StalledIssuesVariantList issues() const;

    void setMoveOrRenameSyncIdBeingFixed(const mega::MegaHandle& syncId, int chosenSide);

signals:
    void solvingIssues(int current, int total);
    void moveOrRenameCannotOccurFound();

protected:
    void clear();

    StalledIssuesVariantList mIssues;

private:
    std::shared_ptr<MoveOrRenameCannotOccurFactory> mMoveOrRenameCannotOccurFactory;
};

#endif // STALLEDISSUEFACTORY_H
