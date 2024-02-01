#ifndef ALERTITEM_H
#define ALERTITEM_H

#include <QWidget>
#include <QFutureWatcher>

#include <memory>

#include "megaapi.h"
#include "MegaUserAlertExt.h"

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

    void setAlertData(MegaUserAlertExt* alert);
    void setAlertType(int type);
    void setAlertHeading(MegaUserAlertExt* alert);
    void setAlertContent(MegaUserAlertExt* alert);
    void setAlertTimeStamp(int64_t ts);
    void contactEmailChanged();
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
    QString getUserFullName();
    void requestFullName();
    void requestEmail();

private:
    Ui::AlertItem *ui;
    mega::MegaApi *megaApi;
    QString mNotificationHeading;
    MegaUserAlertExt* mAlertUser;
    std::unique_ptr<mega::MegaNode> mAlertNode;
    std::shared_ptr<const UserAttributes::FullName> mFullNameAttributes;
    QFutureWatcher<mega::MegaNode*> mAlertNodeWatcher;
};

#endif // ALERTITEM_H
