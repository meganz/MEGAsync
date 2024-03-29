#ifndef ALERTITEM_H
#define ALERTITEM_H

#include <QWidget>
#include <QFutureWatcher>

#include <memory>

#include "megaapi.h"

namespace Ui {
class AlertItem;
}

namespace UserAttributes{
class FullName;
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
    void onUserEmailReady(QString email);

    QString getHeadingString();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

signals:
    void refreshAlertItem(unsigned item);

protected:
    void changeEvent(QEvent *event);

private slots:
    void onAttributesReady();

private:
    QString formatRichString(QString str);
    QString getUserFullName(mega::MegaUserAlert *alert);
    void requestFullName(const char* email);
    void requestEmail(mega::MegaUserAlert* alert);

private:
    Ui::AlertItem *ui;
    mega::MegaApi *megaApi;
    QString mNotificationHeading;
    std::unique_ptr<mega::MegaNode> mAlertNode;
    std::unique_ptr<mega::MegaUserAlert> mAlertUser;
    std::shared_ptr<const UserAttributes::FullName> mFullNameAttributes;
    QFutureWatcher<mega::MegaNode*> mAlertNodeWatcher;
};

#endif // ALERTITEM_H
