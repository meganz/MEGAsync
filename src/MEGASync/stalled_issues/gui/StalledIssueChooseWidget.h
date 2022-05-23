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

    void setData(StalledIssueDataPtr data);
    const StalledIssueDataPtr& data();

    void setIndent();

    void setIssueSolved(bool newIssueSolved);

signals:
    void chooseButtonClicked(int id);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onActionClicked(int button_id);

private:
    Ui::StalledIssueChooseWidget *ui;
    StalledIssueDataPtr mData;
    bool mIsSolved;
    bool mPreviousSolveState;
};

#endif // STALLEDISSUECHOOSEWIDGET_H
