#ifndef STALLEDISSUEHEADER_H
#define STALLEDISSUEHEADER_H

#include "megaapi.h"
#include "StalledIssue.h"
#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>
#include <QStyleOptionViewItem>

namespace Ui {
class StalledIssueHeader;
}

class StalledIssueHeader : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit StalledIssueHeader(QWidget *parent = nullptr);
    ~StalledIssueHeader();

protected:
    void refreshUi() override;

private:
    Ui::StalledIssueHeader *ui;

    void refreshUiByStalledReason();
};

#endif // STALLEDISSUEHEADER_H
