#ifndef GUESTWIDGET_H
#define GUESTWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include "ActiveTransfer.h"

namespace Ui {
class GuestWidget;
}

class MegaApplication;
class GuestWidget : public QWidget
{
    Q_OBJECT

public:
    enum {
        CREATE_ACCOUNT_CLICKED = 1,
        LOGIN_CLICKED
    };

    explicit GuestWidget(QWidget *parent = 0);
    ~GuestWidget();

signals:
    void actionButtonClicked(int button);

private slots:
    void on_bLogin_clicked();
    void on_bCreateAccount_clicked();
    void on_bSettings_clicked();

private:
    Ui::GuestWidget *ui;
    QPushButton *overlayIdle, *overlayPaused;
    MegaApplication *app;
};

#endif // GUESWIDGET_H
