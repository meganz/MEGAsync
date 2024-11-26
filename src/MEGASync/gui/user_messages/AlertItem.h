#ifndef ALERT_ITEM_H
#define ALERT_ITEM_H

#include "UserMessageWidget.h"

#include "megaapi.h"

#include <QFutureWatcher>
#include <QPointer>

#include <memory>

namespace Ui
{
class AlertItem;
}

namespace UserAttributes
{
class FullName;
}

class UserAlert;

class AlertItem : public UserMessageWidget
{
    Q_OBJECT

public:
    explicit AlertItem(QWidget* parent = 0);
    ~AlertItem();

    void setData(UserMessage* data) override;
    UserMessage* getData() const override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;
    bool eventFilter(QObject* obj, QEvent* event) override;

    void setAlertHeading(UserAlert* alert);
    void setAlertContent(UserAlert* alert);
    void setAlertTimeStamp(int64_t ts);
    void contactEmailChanged();
    QString getHeadingString();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    void onAttributesReady();
    void updateAlertData();

private:
    Ui::AlertItem* mUi;
    mega::MegaApi* mMegaApi;
    QString mNotificationHeading;
    QPointer<UserAlert> mAlertData;
    std::shared_ptr<const UserAttributes::FullName> mFullNameAttributes;

    void updateAlertType();
    QString formatRichString(const QString& str);
    QString getUserFullName();
    void requestFullName();
    void setAvatarEmail();
    void processIncomingPendingContactClick();
    void processIncomingContactChangeOrAcceptedClick();
    void processShareOrTakedownClick();
    void processPaymentClick();
};

#endif // ALERT_ITEM_H
