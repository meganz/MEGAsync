#ifndef MEGAUSERALERTFACADE_H
#define MEGAUSERALERTFACADE_H

#include <QObject>

#include <memory>
#include <iostream>

#include "megaapi.h"

class MegaUserAlertFacade : public QObject, public mega::MegaUserAlert
{
    Q_OBJECT

public:
    MegaUserAlertFacade() = default;
    MegaUserAlertFacade(mega::MegaUserAlert* megaUserAlert, QObject *parent = nullptr);
    ~MegaUserAlertFacade() override
    {
        std::cout << "~MegaUserAlertFacade called" << std::endl;
    };

    MegaUserAlertFacade& operator=(MegaUserAlertFacade&& megaUserAlert)
    {
        mMegaUserAlert.reset(megaUserAlert.mMegaUserAlert.release());
        mEmail = megaUserAlert.mEmail;
        megaUserAlert.mEmail.clear();

        return *this;
    }

    const char* getEmail() const override;
    void setEmail(QString email);
    bool isValid() const;
    void reset(mega::MegaUserAlert* alert);

signals:
    void emailChanged();

private:
    std::unique_ptr<mega::MegaUserAlert> mMegaUserAlert;
    std::string mEmail;
};

#endif // MEGAUSERALERTFACADE_H
