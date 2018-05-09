#ifndef STATUSINFO_H
#define STATUSINFO_H

#include <QWidget>

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
signals:
    void clicked();

protected:
    void changeEvent(QEvent * event);
    bool eventFilter(QObject *obj, QEvent *e);

private:
    Ui::StatusInfo *ui;
    int state;
    bool isHovered;
};

#endif // STATUSINFO_H
