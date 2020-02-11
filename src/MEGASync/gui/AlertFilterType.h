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

    enum
    {
        ALL_TYPES = 0,
        TYPE_CONTACTS = 1,
        TYPE_SHARES = 2,
        TYPE_PAYMENTS = 3
    };

    explicit AlertFilterType(QWidget *parent = 0);
    ~AlertFilterType();

    void setActualFilter(int type);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);

private:
    Ui::AlertFilterType *ui;
};

#endif // ALERTFILTERTYPE_H
