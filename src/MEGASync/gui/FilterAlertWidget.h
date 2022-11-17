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
    static const int RED_BUBBLE_MARGIN;
public:
    explicit FilterAlertWidget(QWidget *parent = 0);
    ~FilterAlertWidget();

    void setUnseenNotifications(long long all = 0, long long contacts = 0, long long shares = 0, long long payment = 0);
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
