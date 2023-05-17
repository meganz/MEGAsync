#ifndef STALLEDISSUECHOOSEWIDGET_H
#define STALLEDISSUECHOOSEWIDGET_H

#include "StalledIssueBaseDelegateWidget.h"

#include <QWidget>
#include <QGraphicsOpacityEffect>
#include <QPointer>

namespace Ui {
class StalledIssueChooseWidget;
}

class StalledIssueChooseWidget : public QFrame
{
    Q_OBJECT

public:
    explicit StalledIssueChooseWidget(QWidget *parent = nullptr);
    ~StalledIssueChooseWidget();

    void updateUi(StalledIssueDataPtr data);
    const StalledIssueDataPtr& data();

    void setIssueSolved(bool newIssueSolved);

signals:
    void chooseButtonClicked(int id);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onActionClicked(int button_id);

private:
    void setDisabled(bool solved);

    Ui::StalledIssueChooseWidget *ui;
    StalledIssueDataPtr mData;
    bool mIsSolved;
    bool mPreviousSolveState;
    QPointer<QGraphicsOpacityEffect> mDisableEffect;
};

#endif // STALLEDISSUECHOOSEWIDGET_H
