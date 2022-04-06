#ifndef STALLEDISSUEFILEPATH_H
#define STALLEDISSUEFILEPATH_H

#include <QWidget>
#include "StalledIssueBaseDelegateWidget.h"

namespace Ui {
class StalledIssueFilePath;
}

class StalledIssueFilePath : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit StalledIssueFilePath(QWidget *parent = nullptr);
    ~StalledIssueFilePath();

    void refreshUi();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::StalledIssueFilePath *ui;
};

#endif // STALLEDISSUEFILEPATH_H
