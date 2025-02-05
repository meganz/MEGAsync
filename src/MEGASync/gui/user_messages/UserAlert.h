#ifndef USER_ALERT_H
#define USER_ALERT_H

#include "megaapi.h"
#include "UserMessage.h"
#include "UserMessageTypes.h"

#include <memory>

class UserAlert : public UserMessage
{
    Q_OBJECT

public:
    UserAlert() = delete;
    UserAlert(mega::MegaUserAlert* megaUserAlert, QObject* parent = nullptr);
    ~UserAlert();

    UserAlert& operator=(UserAlert&& megaUserAlert);

    QString getEmail() const;
    void setEmail(QString email);
    bool isValid() const;
    void reset(mega::MegaUserAlert* alert);

    bool sort(UserMessage* checkWith) const override;
    bool isRowAccepted(MessageType type) const override;

    bool isSeen() const override;

    virtual bool getRelevant() const;
    virtual int getType() const;
    virtual mega::MegaHandle getUserHandle() const;
    virtual int64_t getTimestamp(unsigned index) const;
    virtual int64_t getNumber(unsigned index) const;
    virtual mega::MegaHandle getNodeHandle() const;
    virtual const char* getString(unsigned index) const;
    virtual const char* getTitle() const;
    virtual MessageType getMessageType() const;

    std::shared_ptr<mega::MegaNode> getAlertNode() const;

signals:
    void emailChanged();

private:
    std::unique_ptr<mega::MegaUserAlert> mMegaUserAlert;
    MessageType mMessageType;
    QString mEmail;

    void init();
    void initAlertType();

};

#endif // USER_ALERT_H
