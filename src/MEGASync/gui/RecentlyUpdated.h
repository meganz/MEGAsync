#ifndef RECENTLYUPDATED_H
#define RECENTLYUPDATED_H

#include <QWidget>
#include "QTMegaTransferListener.h"
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

namespace Ui {
class RecentlyUpdated;
}

class RecentlyUpdated : public QWidget, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    typedef enum {
        COLLAPSED = 0,
        EXPANDED,
    }VisualMode;

    explicit RecentlyUpdated(QWidget *parent = 0);
    ~RecentlyUpdated();

    void setVisualMode(int mode);

    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e);


private:
    Ui::RecentlyUpdated *ui;

private slots:
    void on_cRecentlyUpdated_stateChanged(int arg1);

signals:
    void onRecentlyUpdatedClicked(int mode);
};

#endif // RECENTLYUPDATED_H
