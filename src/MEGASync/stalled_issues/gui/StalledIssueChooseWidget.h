#ifndef STALLEDISSUECHOOSEWIDGET_H
#define STALLEDISSUECHOOSEWIDGET_H

#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>

namespace Ui {
class StalledIssueChooseWidget;
}

class StalledIssueChooseWidget : public QFrame
{
    Q_OBJECT

public:
    explicit StalledIssueChooseWidget(QWidget *parent = nullptr);
    ~StalledIssueChooseWidget();

    void setData(StalledIssueDataPtr data, const QString &fileName);
    const StalledIssueDataPtr& data();

signals:
    void chooseButtonClicked();

private:
    Ui::StalledIssueChooseWidget *ui;
    StalledIssueDataPtr mData;
};

#endif // STALLEDISSUECHOOSEWIDGET_H
