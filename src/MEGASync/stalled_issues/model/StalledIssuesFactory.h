#ifndef STALLEDISSUEFACTORY_H
#define STALLEDISSUEFACTORY_H

#include <StalledIssue.h>

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
    StalledIssuesCreator(bool isEventRequest);
    StalledIssuesCreator();

    void createIssues(mega::MegaSyncStallList* issues);

    StalledIssuesVariantList issues() const;

signals:
    void solvingIssues(int current, int total);

protected:
    void clear();

    StalledIssuesVariantList mIssues;

private:
    std::shared_ptr<MoveOrRenameCannotOccurFactory> mMoveOrRenameCannotOccurFactory;
    static bool mIsEventRequest;
};

#endif // STALLEDISSUEFACTORY_H
