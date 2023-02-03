// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Notificator.h"

#include <QApplication>
#include <QByteArray>
#include <QIcon>
#include <QImageWriter>
#include "gui/QMegaMessageBox.h"
#include "Utilities.h"
#include <QMetaType>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTemporaryFile>
#include <QVariant>
#include <memory>

#include "MegaApplication.h"

#include <ApplicationServices/ApplicationServices.h>
#include "macx/NotificationHandler.h"

using namespace mega;

QHash<int64_t, MegaNotification*> Notificator::notifications;

Notificator::Notificator(const QString &programName, QSystemTrayIcon *trayicon, QObject *parent) :
    NotificatorBase(programName, nullptr, parent)
{
    mMode = UserNotificationCenter;
}

void Notificator::notify(Class cls, const QString &title, const QString &text, int millisTimeout)
{
    switch(mMode)
    {
    case UserNotificationCenter:
    {
        MegaNotification *n = new MegaNotification();
        if (title == tr("MEGAsync"))
        {
            n->setTitle(QString::fromUtf8("MEGA"));
        }
        else
        {
            n->setTitle(title);
        }
        n->setText(text);
        n->setExpirationTime(millisTimeout);
        n->setType(cls);
        NotificationHandler::instance()->showNotification(n);
        break;
    }
    default:
        NotificatorBase::notify(cls, title, text, millisTimeout);
        break;
    }
}

void Notificator::notify(MegaNotification *notification)
{
    if (mMode == UserNotificationCenter)
    {
        NotificationHandler::instance()->showNotification(notification);
        return;
    }
    else
    {
        NotificatorBase::notify(notification);
    }
}

MegaNotification::MegaNotification()
    : MegaNotificationBase()
{
}

QStringList MegaNotification::getActions() const
{
    if(NotificationHandler::instance()->acceptsMultipleSelection())
    {
        return actions;
    }
    else if(!actions.isEmpty())
    {
        return QStringList() << actions.first();
    }
    else
    {
        return QStringList();
    }
}
