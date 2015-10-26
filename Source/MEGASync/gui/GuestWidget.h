#ifndef GUESTWIDGET_H
#define GUESTWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include "ActiveTransfer.h"

namespace Ui {
class GuestWidget;
}

class GuestWidget : public QWidget
{
    Q_OBJECT

public:
    enum {
        LOGIN_CLICKED = 0,
        CREATE_ACCOUNT_CLICKED = 1
    };

    explicit GuestWidget(QWidget *parent = 0);
    ActiveTransfer* getTransfer();
    void setDownloadLabel(QString dString);
    void setRemainingTime(QString time);
    void hideDownloads();
    void showDownloads();
    void setIdleState(bool state);
    bool idleState();
    void setPauseState(bool state);
    bool pauseState();

    ~GuestWidget();

signals:
    void actionButtonClicked(int button);
    void cancelCurrentDownload();
    void cancelAllDownloads();
    void pauseClicked();

public slots:
    void onTransferCancel(int x, int y);

private slots:
    void on_bLogin_clicked();
    void on_bCreateAccount_clicked();

private:
    Ui::GuestWidget *ui;
    QMenu *transferMenu;
    QPushButton *overlayIdle, *overlayPaused;
    bool isIdle;
};

#endif // GUESWIDGET_H
