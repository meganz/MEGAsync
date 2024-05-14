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

    virtual std::shared_ptr<StalledIssue> createIssue(const mega::MegaSyncStall* stall) = 0;
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

    void createIssues(mega::MegaSyncStallList* issues, UpdateType updateType,
        const QMultiMap<mega::MegaSyncStall::SyncStallReason, QPointer<MultiStepIssueSolverBase>>& multiStepIssueSolversByReason);

    StalledIssuesVariantList issues() const;

signals:
    void solvingIssues(int current, int total);

protected:
    void clear();

    StalledIssuesVariantList mIssues;

private:
    QPointer<MultiStepIssueSolverBase> getMultiStepIssueSolverByStall(const QMultiMap<mega::MegaSyncStall::SyncStallReason, QPointer<MultiStepIssueSolverBase>>& multiStepIssueSolversByReason,
        std::shared_ptr<StalledIssue> issue);
    std::shared_ptr<MoveOrRenameCannotOccurFactory> mMoveOrRenameCannotOccurFactory;
};

#endif // STALLEDISSUEFACTORY_H
