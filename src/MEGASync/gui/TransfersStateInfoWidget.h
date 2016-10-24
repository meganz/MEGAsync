#ifndef TRANSFERSSTATEINFOWIDGET_H
#define TRANSFERSSTATEINFOWIDGET_H

#include <QWidget>

namespace Ui {
class TransfersStateInfoWidget;
}

class TransfersStateInfoWidget : public QWidget
{
    Q_OBJECT

public:

    enum {
        NO_TRANSFERS = 0,
        NO_DOWNLOADS,
        NO_UPLOADS,
        PAUSED
    };

    explicit TransfersStateInfoWidget(QWidget *parent = 0 , int state = NO_TRANSFERS);
    ~TransfersStateInfoWidget();

    void setState(int state);
    void setBackgroundStyle(const QString &text);

private:
    Ui::TransfersStateInfoWidget *ui;
    int state;

    void changeEvent(QEvent *event);
};

#endif // TRANSFERSSTATEINFOWIDGET_H
