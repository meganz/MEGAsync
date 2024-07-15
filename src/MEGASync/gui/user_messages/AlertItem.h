#ifndef ALERT_ITEM_H
#define ALERT_ITEM_H

#include "UserAlert.h"

#include "megaapi.h"

#include <QWidget>
#include <QFutureWatcher>

#include <memory>

namespace Ui
{
class AlertItem;
}

namespace UserAttributes
{
class FullName;
}

class AlertItem : public QWidget
{
    Q_OBJECT

public:
    explicit AlertItem(UserAlert* alert, QWidget* parent = 0);
    ~AlertItem();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setAlertHeading(UserAlert* alert);
    void setAlertContent(UserAlert* alert);
    void setAlertTimeStamp(int64_t ts);
    void contactEmailChanged();
    QString getHeadingString();

signals:
    void refreshAlertItem(unsigned item);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    void onAttributesReady();

private:
    Ui::AlertItem* mUi;
    mega::MegaApi* mMegaApi;
    QString mNotificationHeading;
    UserAlert* mAlertUser;
    std::unique_ptr<mega::MegaNode> mAlertNode;
    std::shared_ptr<const UserAttributes::FullName> mFullNameAttributes;
    QFutureWatcher<mega::MegaNode*> mAlertNodeWatcher;

    void setAlertData(UserAlert* alert);
    void updateAlertType();
    QString formatRichString(const QString& str);
    QString getUserFullName();
    void requestFullName();
    void updateAlertData();
    void processIncomingPendingContactClick();
    void processIncomingContactChangeOrAcceptedClick();
    void processShareOrTakedownClick();
    void processPaymentClick();

};

#endif // ALERT_ITEM_H
