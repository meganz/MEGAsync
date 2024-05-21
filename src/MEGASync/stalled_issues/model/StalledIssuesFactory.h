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

    virtual std::shared_ptr<StalledIssue> createIssue(MultiStepIssueSolverBase* solver, const mega::MegaSyncStall* stall) = 0;
    virtual void clear(){}
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

    void createIssues(mega::MegaSyncStallList* issues,
        UpdateType updateType);

    StalledIssuesVariantList issues() const;
    bool multiStepIssueSolveActive() const;
    void addMultiStepIssueSolver(MultiStepIssueSolverBase* issue);

signals:
    void solvingIssues(int current, int total);

protected:
    void clear();

    StalledIssuesVariantList mPendingIssues;
    StalledIssuesVariantList mBeingSolvedIssues;
    StalledIssuesVariantList mSolvedIssues;

private:
    QPointer<MultiStepIssueSolverBase> getMultiStepIssueSolverByStall(const mega::MegaSyncStall* stall);

    QMultiMap<mega::MegaSyncStall::SyncStallReason,
        MultiStepIssueSolverBase*> mMultiStepIssueSolversByReason;
    std::shared_ptr<MoveOrRenameCannotOccurFactory> mMoveOrRenameCannotOccurFactory;
};

#endif // STALLEDISSUEFACTORY_H
