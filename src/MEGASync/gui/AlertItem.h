#ifndef ALERTITEM_H
#define ALERTITEM_H

#include <QWidget>
#include "megaapi.h"

namespace Ui {
class AlertItem;
}

class AlertItem : public QWidget
{
    Q_OBJECT

public:
    explicit AlertItem(QWidget *parent = 0);
    ~AlertItem();

    void setAlertData(mega::MegaUserAlert *alert);
    void setAlertType(int type);
    void setAlertHeading(mega::MegaUserAlert *alert);
    void setAlertContent(mega::MegaUserAlert *alert);
    void setAlertTimeStamp(int64_t ts);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

private:
    void setAvatar(mega::MegaUserAlert *alert);
    QString formatRichString(QString str);

private:
    Ui::AlertItem *ui;
    mega::MegaApi *megaApi;
};

#endif // ALERTITEM_H
