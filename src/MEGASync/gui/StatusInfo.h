#ifndef STATUSINFO_H
#define STATUSINFO_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class StatusInfo;
}

class StatusInfo : public QWidget
{
    Q_OBJECT

public:
    enum class TRANSFERS_STATES
    {
        STATE_STARTING,
        STATE_PAUSED,
        STATE_WAITING,
        STATE_INDEXING,
        STATE_UPDATED,
        STATE_SYNCING,
        STATE_TRANSFERRING,
        STATE_FAILED,
    };

    explicit StatusInfo(QWidget *parent = 0);
    ~StatusInfo();

    void setState(TRANSFERS_STATES state);
    void update();

    TRANSFERS_STATES getState();
    void setOverQuotaState(bool oq);

    static QIcon scanningIcon(int &index);

protected:
    void changeEvent(QEvent * event);

private slots:
    void scanningAnimationStep();

private:
    void setFailedText();

    Ui::StatusInfo *ui;
    TRANSFERS_STATES mState;
    bool mIsOverQuota;
    QTimer mScanningTimer;
    int mScanningAnimationIndex;
};

#endif // STATUSINFO_H
