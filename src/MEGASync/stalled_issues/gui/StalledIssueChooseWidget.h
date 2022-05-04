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

    void setIndent();

signals:
    void chooseButtonClicked();

protected:
    void paintEvent(QPaintEvent *) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::StalledIssueChooseWidget *ui;
    StalledIssueDataPtr mData;
    QString mFileName;
};

#endif // STALLEDISSUECHOOSEWIDGET_H
