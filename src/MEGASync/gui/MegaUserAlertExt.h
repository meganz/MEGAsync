#ifndef MEGAUSERALERTEXT_H
#define MEGAUSERALERTEXT_H

#include <QObject>

#include <memory>
#include <iostream>

#include "megaapi.h"

class MegaUserAlertExt : public QObject
{
    Q_OBJECT

public:
    MegaUserAlertExt() = default;
    MegaUserAlertExt(mega::MegaUserAlert* megaUserAlert, QObject *parent = nullptr);

    MegaUserAlertExt& operator=(MegaUserAlertExt&& megaUserAlert)
    {
        //mMegaUserAlert.reset(megaUserAlert.mMegaUserAlert.release());
        mMegaUserAlert = megaUserAlert.mMegaUserAlert;
        megaUserAlert.mMegaUserAlert = nullptr;

        mEmail = megaUserAlert.mEmail;
        megaUserAlert.mEmail.clear();

        return *this;
    }

    const char* getEmail() const;
    void setEmail(QString email);
    bool isValid() const;
    void reset(mega::MegaUserAlert* alert);
    mega::MegaUserAlert* getMegaUserAlert() const
    {
        return mMegaUserAlert;
    }

    virtual unsigned getId() const;
    virtual bool getSeen() const;
    virtual int getType() const;
    virtual int64_t getTimestamp(unsigned index) const;

signals:
    void emailChanged();

private:
    //std::unique_ptr<mega::MegaUserAlert> mMegaUserAlert;
    mega::MegaUserAlert* mMegaUserAlert = nullptr;
    std::string mEmail;
};

#endif // MEGAUSERALERTEXT_H
