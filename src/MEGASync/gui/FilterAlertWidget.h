#ifndef FILTERALERTWIDGET_H
#define FILTERALERTWIDGET_H

#include <QWidget>
#include "QFilterAlertsModel.h"

namespace Ui {
class FilterAlertWidget;
}

class FilterAlertWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilterAlertWidget(QWidget *parent = 0);
    ~FilterAlertWidget();

    void setUnseenNotifications(int all = 0, int contacts = 0, int shares = 0, int payment = 0);
    void reset();

private slots:
    void on_bAll_clicked();
    void on_bContacts_clicked();
    void on_bShares_clicked();
    void on_bPayment_clicked();

signals:
    void onFilterClicked(int);

private:
    Ui::FilterAlertWidget *ui;

protected:
    void changeEvent(QEvent *event);
};

#endif // FILTERALERTWIDGET_H
