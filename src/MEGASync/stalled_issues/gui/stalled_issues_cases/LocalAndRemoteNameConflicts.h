#ifndef LOCALANDREMOTENAMECONFLICTS_H
#define LOCALANDREMOTENAMECONFLICTS_H

#include "StalledIssueBaseDelegateWidget.h"

namespace Ui {
class LocalAndRemoteNameConflicts;
}

class LocalAndRemoteNameConflicts : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit LocalAndRemoteNameConflicts(QWidget *parent = nullptr);
    ~LocalAndRemoteNameConflicts();

    void refreshUi() override;

private:
    Ui::LocalAndRemoteNameConflicts *ui;
};

#endif // LOCALANDREMOTENAMECONFLICTS_H
