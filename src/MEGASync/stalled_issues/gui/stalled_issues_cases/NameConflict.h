#ifndef NAMECONFLICT_H
#define NAMECONFLICT_H

#include <StalledIssueBaseDelegateWidget.h>
#include <NameConflictStalledIssue.h>
#include <StalledIssuesUtilities.h>
#include "StalledIssueActionTitle.h"

namespace Ui {
class NameConflict;
}

class NameConflictTitle : public StalledIssueActionTitle
{
    Q_OBJECT

public:
    explicit NameConflictTitle(int index, const QString &conflictedName, QWidget* parent = nullptr);
    ~NameConflictTitle() = default;

    void initTitle(const QString &conflictedName);
    int getIndex() const;

private:
    int mIndex;
};

class NameConflict : public QWidget
{
    Q_OBJECT

public:
    explicit NameConflict(QWidget *parent = nullptr);
    ~NameConflict();

    void updateUi(NameConflictedStalledIssue::NameConflictData data);
    void setDisabled();

    void setDelegate(QPointer<StalledIssueBaseDelegateWidget> newDelegate);

signals:
    void refreshUi();
    void allSolved();

private slots:
    void onActionClicked(int actionId);

private:
    void removeConflictedNameWidget(QWidget *widget);

    Ui::NameConflict *ui;
    NameConflictedStalledIssue::NameConflictData mData;
    StalledIssuesUtilities mUtilities;
    QPointer<StalledIssueBaseDelegateWidget> mDelegate;
};

#endif // NAMECONFLICT_H
