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
    static const int HEIGHT;

    explicit StalledIssueHeader(QWidget *parent = nullptr);
    ~StalledIssueHeader();

    void expand(bool state) override;

    void showAction(const QString& actionButtonText);

    void setLeftTitleText(const QString& text);
    void addFileName();
    void setRightTitleText(const QString& text);

    void setTitleDescriptionText(const QString& text);

protected:
    virtual void refreshCaseUi() = 0;

    void ignoreFile();
    QString fileName();

protected slots:
    virtual void on_actionButton_clicked(){}

private slots:
    void onIgnoreFileFinished();

private:
    bool eventFilter(QObject *watched, QEvent *event) override;

    Ui::StalledIssueHeader *ui;
    void refreshUi() override;

    QFutureWatcher<void> mIgnoreWatcher;
};

#endif // STALLEDISSUEHEADER_H
