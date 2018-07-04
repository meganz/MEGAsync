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

    enum {
        STATE_STARTING,
        STATE_PAUSED,
        STATE_WAITING,
        STATE_INDEXING,
        STATE_UPDATED
    };

public:
    explicit StatusInfo(QWidget *parent = 0);
    ~StatusInfo();

    void setState(int state);
    void setOverQuotaState(bool oq);
signals:
    void clicked();

protected:
    void changeEvent(QEvent * event);
    bool eventFilter(QObject *obj, QEvent *e);

private slots:
    void scanningAnimationStep();

private:
    Ui::StatusInfo *ui;
    int state;
    bool isHovered;
    bool isOverQuota;
    QTimer scanningTimer;
    int scanningAnimationIndex;
};

#endif // STATUSINFO_H
