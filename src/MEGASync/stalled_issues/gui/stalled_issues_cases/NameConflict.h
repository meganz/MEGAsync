#ifndef NAMECONFLICT_H
#define NAMECONFLICT_H

#include <StalledIssueBaseDelegateWidget.h>
#include <StalledIssue.h>
#include <StalledIssuesUtilities.h>
#include "StalledIssueActionTitle.h"

namespace Ui {
class NameConflict;
}

class NameConflictTitle : public StalledIssueActionTitle
{
    Q_OBJECT

public:
    explicit NameConflictTitle(QWidget* parent = nullptr);
    ~NameConflictTitle() = default;

    void initTitle();
};

class NameConflict : public QWidget
{
    Q_OBJECT

public:
    explicit NameConflict(QWidget *parent = nullptr);
    ~NameConflict();

    void updateUi(NameConflictedStalledIssue::NameConflictData data);

signals:
    void refreshUi();
    void conflictSolved();

private slots:
    void onActionClicked(int actionId);

private:
    void removeConflictedNameWidget(QWidget *widget);

    Ui::NameConflict *ui;
    NameConflictedStalledIssue::NameConflictData mData;
    StalledIssuesUtilities mUtilities;
};

#endif // NAMECONFLICT_H
