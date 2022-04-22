#ifndef STALLEDISSUEHEADER_H
#define STALLEDISSUEHEADER_H

#include "megaapi.h"
#include "StalledIssue.h"
#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>
#include <QStyleOptionViewItem>

#include "ui_StalledIssueHeader.h"

class StalledIssueHeader : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:

    static const int BODY_INDENT;
    static const int ARROW_INDENT;
    static const int ICON_INDENT;

    explicit StalledIssueHeader(QWidget *parent = nullptr);
    ~StalledIssueHeader();

    void expand(bool state) override;
    void showAction();

signals:
    void actionClicked();

protected:
    virtual void refreshCaseUi() = 0;
    Ui::StalledIssueHeader *ui;

private:
    void refreshUi() override;
};

#endif // STALLEDISSUEHEADER_H
