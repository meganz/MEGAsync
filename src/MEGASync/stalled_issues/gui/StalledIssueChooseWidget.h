#ifndef STALLEDISSUECHOOSEWIDGET_H
#define STALLEDISSUECHOOSEWIDGET_H

#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>

namespace Ui {
class StalledIssueChooseWidget;
}

class StalledIssueChooseWidget : public StalledIssueBaseDelegateWidget
{
    Q_OBJECT

public:
    explicit StalledIssueChooseWidget(QWidget *parent = nullptr);
    ~StalledIssueChooseWidget();

    void refreshUi() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::StalledIssueChooseWidget *ui;
};

#endif // STALLEDISSUECHOOSEWIDGET_H
