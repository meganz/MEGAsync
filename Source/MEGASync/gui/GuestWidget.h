#ifndef GUESTWIDGET_H
#define GUESTWIDGET_H

#include <QWidget>

namespace Ui {
class GuestWidget;
}

class GuestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GuestWidget(QWidget *parent = 0);
    ~GuestWidget();

private:
    Ui::GuestWidget *ui;
};

#endif // GUESWIDGET_H
