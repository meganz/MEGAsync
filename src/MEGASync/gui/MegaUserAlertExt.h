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
    MegaUserAlertExt() = delete;
    MegaUserAlertExt(mega::MegaUserAlert* megaUserAlert, QObject *parent = nullptr);
    ~MegaUserAlertExt() = default;

    MegaUserAlertExt& operator=(MegaUserAlertExt&& megaUserAlert)
    {
        mMegaUserAlert.reset(megaUserAlert.mMegaUserAlert.release());
        megaUserAlert.mMegaUserAlert = nullptr;

        mEmail = megaUserAlert.mEmail;
        megaUserAlert.mEmail.clear();

        return *this;
    }

    const char* getEmail() const;
    void setEmail(QString email);
    bool isValid() const;
    void reset(mega::MegaUserAlert* alert);

    virtual unsigned getId() const;

    virtual bool getSeen() const;
    virtual bool getRelevant() const;
    virtual int getType() const;
    virtual mega::MegaHandle getUserHandle() const;
    virtual int64_t getTimestamp(unsigned index) const;
    virtual int64_t getNumber(unsigned index) const;
    virtual mega::MegaHandle getNodeHandle() const;
    virtual const char* getString(unsigned index) const;
    virtual const char* getTitle() const;

signals:
    void emailChanged();

private:
    std::unique_ptr<mega::MegaUserAlert> mMegaUserAlert;
    std::string mEmail;
};

#endif // MEGAUSERALERTEXT_H
