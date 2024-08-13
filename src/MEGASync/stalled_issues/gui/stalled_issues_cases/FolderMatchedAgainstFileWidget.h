#ifndef FOLDERMATCHEDAGAINSFILEWIDGET_H
#define FOLDERMATCHEDAGAINSFILEWIDGET_H

#include "StalledIssueBaseDelegateWidget.h"

namespace Ui {
class FolderMatchedAgainstFileWidget;
}

class StalledIssueChooseWidget;

class FolderMatchedAgainstFileWidget : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit FolderMatchedAgainstFileWidget(QWidget *parent = nullptr);
    ~FolderMatchedAgainstFileWidget();

    void refreshUi() override;

private:

    Ui::FolderMatchedAgainstFileWidget *ui;
};

#endif // FOLDERMATCHEDAGAINSFILEWIDGET_H
