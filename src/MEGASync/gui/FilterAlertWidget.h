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

    void enableFilters(bool contacts = true, bool shares = true, bool payment = true);
    void setUnseenNotifications(int all = 0, int contacts = 0, int shares = 0, int payment = 0);

private:
    void enableFilterContacts(bool opt);
    void enableFilterShares(bool opt);
    void enableFilterPayment(bool opt);

private slots:
    void on_bAll_clicked();
    void on_bContacts_clicked();
    void on_bShares_clicked();
    void on_bPayment_clicked();

signals:
    void onFilterClicked(int);

private:
    Ui::FilterAlertWidget *ui;
    bool isContactsAvailable, isSharesAvailable, isPaymentAvailable;
};

#endif // FILTERALERTWIDGET_H
