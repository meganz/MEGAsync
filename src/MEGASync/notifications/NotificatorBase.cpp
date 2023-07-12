// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NotificatorBase.h"
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

using namespace mega;

MegaNotificationBase::MegaNotificationBase()
{
    mTitle = MegaSyncApp->getMEGAString();
    mText = MegaSyncApp->getMEGAString();
    mExpirationTime = 10000;
    mStyle = -1;
    mType = NotificatorBase::Information;
    mId = -1;

    qRegisterMetaType<MegaNotificationBase::CloseReason>("MegaNotificationBase::CloseReason");
    qRegisterMetaType<MegaNotificationBase::Action>("MegaNotificationBase::Action");

    connect(this, &MegaNotificationBase::closed, this, &MegaNotificationBase::deleteLater, Qt::QueuedConnection);
    connect(this, &MegaNotificationBase::activated, this, &MegaNotificationBase::deleteLater, Qt::QueuedConnection);
}

int MegaNotificationBase::getStyle() const
{
    return mStyle;
}

void MegaNotificationBase::setStyle(int value)
{
    mStyle = value;
}

QStringList MegaNotificationBase::getActions() const
{
    return actions;
}

void MegaNotificationBase::setActions(const QStringList &value)
{
    actions = value;
}

int MegaNotificationBase::getType() const
{
    return mType;
}

void MegaNotificationBase::setType(int value)
{
    mType = value;
}

int64_t MegaNotificationBase::getId() const
{
    return mId;
}

void MegaNotificationBase::setId(const int64_t &value)
{
    mId = value;
}

QVariant MegaNotificationBase::getData() const
{
    return mData;
}

void MegaNotificationBase::setData(const QVariant &value)
{
    mData = value;
}

void MegaNotificationBase::emitLegacyNotificationActivated()
{
    emit activated(Action::legacy);
}

QString MegaNotificationBase::getTitle() const
{
    return mTitle;
}

void MegaNotificationBase::setTitle(const QString &value)
{
    mTitle = value;
}

QString MegaNotificationBase::getText() const
{
    return mText;
}

void MegaNotificationBase::setText(const QString &value)
{
    mText = value;
}

QString MegaNotificationBase::getSource() const
{
    return mSource;
}

void MegaNotificationBase::setSource(const QString &value)
{
    mSource = value;
}

int MegaNotificationBase::getExpirationTime() const
{
    return mExpirationTime;
}

void MegaNotificationBase::setExpirationTime(int value)
{
    mExpirationTime = value;
}

QString MegaNotificationBase::getImagePath() const
{
    return mImagePath;
}

void MegaNotificationBase::setImagePath(const QString &value)
{
    mImagePath = value;
}

////////////////NOTIFICATOR BASE
///
NotificatorBase::NotificatorBase(const QString &programName, QSystemTrayIcon *trayicon, QObject *parent) :
    QObject(parent),
    mProgramName(programName),
    mMode(None),
    mTrayIcon(trayicon),
    mCurrentNotification(nullptr)
{
}

void NotificatorBase::notifySystray(Class cls, const QString &title, const QString &text, int millisTimeout, bool forceQt)
{
    if (!forceQt)
    {
        if (mCurrentNotification)
        {
            mCurrentNotification->deleteLater();
        }
        mCurrentNotification = NULL;
    }

    QSystemTrayIcon::MessageIcon sicon = QSystemTrayIcon::NoIcon;
    switch(cls) // Set icon based on class
    {
    case Information:
        sicon = QSystemTrayIcon::Information;
        break;

    case Warning:
        sicon = QSystemTrayIcon::Warning;
        break;

    case Critical:
        sicon = QSystemTrayIcon::Critical;
        break;

    default:
        break;
    }
    mTrayIcon->showMessage(title, text, sicon, millisTimeout);
}

void NotificatorBase::notifySystray(MegaNotificationBase *notification)
{
    if (!notification)
    {
        return;
    }

    notifySystray((NotificatorBase::Class)notification->getType(), notification->getText(),
                  notification->getSource(), notification->getExpirationTime());
}

void NotificatorBase::onMessageClicked()
{
    if (mCurrentNotification)
    {
        mCurrentNotification->emitLegacyNotificationActivated();
        mCurrentNotification = NULL;
    }
}

void NotificatorBase::notify(Class cls, const QString &title, const QString &text, int millisTimeout)
{
    switch(mMode)
    {
        case QSystemTray:
            notifySystray(cls, title, text, millisTimeout);
            break;
        default:
        {
            QMegaMessageBox::MessageBoxInfo info;
            info.title = title;
            info.text = text;

            switch(cls) // Set icon based on class
            {
                case Information: QMegaMessageBox::information(info); break;
                case Warning: QMegaMessageBox::warning(info); break;
                case Critical: QMegaMessageBox::critical(info); break;
            }

            break;
        }
    }
}

void NotificatorBase::notify(MegaNotificationBase *notification)
{
    if (mMode == QSystemTray)
    {
        notifySystray(notification);
    }
    else
    {
        notify((Class)notification->getType(), notification->getTitle(), notification->getText(), notification->getExpirationTime());
    }
}
