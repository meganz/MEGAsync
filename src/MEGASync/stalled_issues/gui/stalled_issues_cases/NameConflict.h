#ifndef NAMECONFLICT_H
#define NAMECONFLICT_H

#include <StalledIssueBaseDelegateWidget.h>
#include <StalledIssue.h>
#include <StalledIssuesUtilities.h>

namespace Ui {
class NameConflict;
}

class NameConflict : public QWidget
{
    Q_OBJECT

public:
    explicit NameConflict(QWidget *parent = nullptr);
    ~NameConflict();

    void updateUi(NameConflictedStalledIssue::NameConflictData data);

signals:
    void actionFinished();

private slots:
    void onActionClicked(int actionId);

private:
    Ui::NameConflict *ui;
    NameConflictedStalledIssue::NameConflictData mData;
    StalledIssuesUtilities mUtilities;
};

#endif // NAMECONFLICT_H
