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
    void loginButtonClicked();
    void cancelCurrentDownload();
    void cancelAllDownloads();
    void pauseClicked();

public slots:
    void onTransferCancel(int x, int y);

private:
    Ui::GuestWidget *ui;
    QMenu *transferMenu;
    QPushButton *overlayIdle, *overlayPaused;
};

#endif // GUESWIDGET_H
