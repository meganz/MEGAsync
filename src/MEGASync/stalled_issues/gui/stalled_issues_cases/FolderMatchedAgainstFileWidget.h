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
    explicit FolderMatchedAgainstFileWidget(std::shared_ptr<mega::MegaSyncStall> originalStall, QWidget *parent = nullptr);
    ~FolderMatchedAgainstFileWidget();

    void refreshUi() override;

    static QString keepLocalSideString(const KeepSideInfo& info);
    static QString keepRemoteSideString(const KeepSideInfo& info);

private:

    Ui::FolderMatchedAgainstFileWidget *ui;
};

#endif // FOLDERMATCHEDAGAINSFILEWIDGET_H
