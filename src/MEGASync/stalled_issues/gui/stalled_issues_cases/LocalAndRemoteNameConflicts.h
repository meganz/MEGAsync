#ifndef LOCALANDREMOTENAMECONFLICTS_H
#define LOCALANDREMOTENAMECONFLICTS_H

#include "StalledIssueBaseDelegateWidget.h"

namespace Ui {
class LocalAndRemoteNameConflicts;
}

class LocalAndRemoteNameConflicts : public StalledIssueBaseDelegateWidget
{
    static const QString FILES_DESCRIPTION;
    static const QString FOLDERS_DESCRIPTION;
    static const QString FILES_AND_FOLDERS_DESCRIPTION;

    Q_OBJECT

public:
    explicit LocalAndRemoteNameConflicts(QWidget *parent = nullptr);
    ~LocalAndRemoteNameConflicts();

public slots:
    void refreshUi() override;

private:
    Ui::LocalAndRemoteNameConflicts *ui;
};

#endif // LOCALANDREMOTENAMECONFLICTS_H
