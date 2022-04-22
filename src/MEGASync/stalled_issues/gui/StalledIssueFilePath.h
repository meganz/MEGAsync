#ifndef STALLEDISSUEFILEPATH_H
#define STALLEDISSUEFILEPATH_H

#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>
#include <QLabel>

namespace Ui {
class StalledIssueFilePath;
}

class StalledIssueFilePath : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit StalledIssueFilePath(QWidget *parent = nullptr);
    ~StalledIssueFilePath();

    void fillFilePath();
    void fillMoveFilePath();

    void setIndent(int indent);

protected:
    void refreshUi() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void fillPathName(const StalledIssueDataPtr& data, QLabel *label);

    Ui::StalledIssueFilePath *ui;
};

#endif // STALLEDISSUEFILEPATH_H
