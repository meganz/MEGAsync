#ifndef STALLEDISSUEFACTORY_H
#define STALLEDISSUEFACTORY_H

#include <StalledIssue.h>

class StalledIssuesFactory : public QObject
{
        Q_OBJECT
public:
    StalledIssuesFactory(bool isEventRequest);

    void createIssues(mega::MegaSyncStallList* issues);

    StalledIssuesVariantList issues() const;

signals:
    void solvingIssues(int current, int total);

private:
    StalledIssuesVariantList mIssues;
    bool mIsEventRequest;
};

#endif // STALLEDISSUEFACTORY_H
