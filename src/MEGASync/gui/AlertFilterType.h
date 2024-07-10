#ifndef ALERTFILTERTYPE_H
#define ALERTFILTERTYPE_H

#include "NotificationAlertTypes.h"

#include <QWidget>

namespace Ui {
class AlertFilterType;
}

class AlertFilterType : public QWidget
{
    Q_OBJECT

public:
    explicit AlertFilterType(QWidget* parent = 0);
    ~AlertFilterType();

    void setActualFilter(AlertType type);
    bool allFilterHasBeenSelected() const;
    void resetAllFilterHasBeenSelected();

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event);
    void changeEvent(QEvent *event);

private:
    Ui::AlertFilterType* ui;
    AlertType mType;
    bool mAllFilterHasBeenSelected;

};

#endif // ALERTFILTERTYPE_H
