#ifndef ALERTFILTERTYPE_H
#define ALERTFILTERTYPE_H

#include "QFilterAlertsModel.h"

#include <QWidget>

namespace Ui {
class AlertFilterType;
}

class AlertFilterType : public QWidget
{
    Q_OBJECT

public:
    explicit AlertFilterType(QWidget *parent = 0);
    ~AlertFilterType();

    void setActualFilter(QFilterAlertsModel::FilterType type);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);
    void changeEvent(QEvent *event);

private:
    Ui::AlertFilterType *ui;
    QFilterAlertsModel::FilterType mType;

};

#endif // ALERTFILTERTYPE_H
