#ifndef ALERTFILTERTYPE_H
#define ALERTFILTERTYPE_H

#include <QWidget>

namespace Ui {
class AlertFilterType;
}

class AlertFilterType : public QWidget
{
    Q_OBJECT

public:

    enum ALERT_TYPE
    {
        ALL_TYPES = 0,
        TYPE_CONTACTS = 1,
        TYPE_SHARES = 2,
        TYPE_PAYMENTS = 3,
    };

    explicit AlertFilterType(QWidget *parent = 0);
    ~AlertFilterType();

    void setActualFilter(ALERT_TYPE type);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);
    void changeEvent(QEvent *event);

private:
    Ui::AlertFilterType *ui;
    ALERT_TYPE mType;
};

#endif // ALERTFILTERTYPE_H
