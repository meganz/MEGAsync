#ifndef ALERTITEM_H
#define ALERTITEM_H

#include <QWidget>
#include "megaapi.h"
#include <QFutureWatcher>
#include <memory>

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

    QString getHeadingString();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

signals:
    void refreshAlertItem(unsigned item);

protected:
    void changeEvent(QEvent *event);

private:
    void setAvatar(mega::MegaUserAlert *alert);
    QString formatRichString(QString str);

private:
    Ui::AlertItem *ui;
    mega::MegaApi *megaApi;
    QString notificationHeading;
    std::unique_ptr<mega::MegaNode> alertNode;
    std::unique_ptr<mega::MegaUserAlert> alertUser;
    QFutureWatcher<mega::MegaNode*> getAlertNodeWatcher;
};

#endif // ALERTITEM_H
