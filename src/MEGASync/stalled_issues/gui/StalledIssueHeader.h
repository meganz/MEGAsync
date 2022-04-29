#ifndef STALLEDISSUEHEADER_H
#define STALLEDISSUEHEADER_H

#include "megaapi.h"
#include "StalledIssue.h"
#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>
#include <QStyleOptionViewItem>
#include <QFutureWatcher>

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

protected:
    virtual void refreshCaseUi() = 0;
    Ui::StalledIssueHeader *ui;

    void ignoreFile();

protected slots:
    virtual void on_actionButton_clicked(){}

private slots:
    void onIgnoreFileFinished();

private:
    void refreshUi() override;

    QFutureWatcher<void> mIgnoreWatcher;
};

#endif // STALLEDISSUEHEADER_H
