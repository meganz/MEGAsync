#ifndef FILTERALERTWIDGET_H
#define FILTERALERTWIDGET_H

#include "UserMessageTypes.h"

#include <QWidget>

namespace Ui
{
class FilterAlertWidget;
}

class FilterAlertWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilterAlertWidget(QWidget* parent = 0);
    ~FilterAlertWidget();

    void setUnseenNotifications(long long all = 0,
                                long long contacts = 0,
                                long long shares = 0,
                                long long payment = 0);
    void reset();
    MessageType getCurrentFilter() const;

private:
    void prepareEventFilters();

private slots:
    void on_bAll_clicked();
    void onBContactsClicked();
    void onBSharesClicked();
    void onBPaymentClicked();

signals:
    void filterClicked(MessageType);

private:
    Ui::FilterAlertWidget* mUi;
    MessageType mCurrentFilter;

protected:
    bool event(QEvent* event) override;
    bool eventFilter(QObject* o, QEvent* e) override;
};

#endif // FILTERALERTWIDGET_H
