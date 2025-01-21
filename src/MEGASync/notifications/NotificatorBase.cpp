// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "NotificatorBase.h"

#include "MegaApplication.h"
#include "QMegaMessageBox.h"
#include "Utilities.h"

#include <QApplication>
#include <QByteArray>
#include <QIcon>
#include <QImageWriter>
#include <QMetaType>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTemporaryFile>
#include <QVariant>

using namespace mega;

DesktopAppNotificationBase::DesktopAppNotificationBase()
{
    mTitle = MegaSyncApp->getMEGAString();
    mText = MegaSyncApp->getMEGAString();
    mExpirationTime = 10000;
    mStyle = -1;
    mType = NotificatorBase::Information;
    mId = -1;

    qRegisterMetaType<DesktopAppNotificationBase::CloseReason>("DesktopAppNotificationBase::CloseReason");
    qRegisterMetaType<DesktopAppNotificationBase::Action>("DesktopAppNotificationBase::Action");

    connect(this, &DesktopAppNotificationBase::closed, this, &DesktopAppNotificationBase::deleteLater, Qt::QueuedConnection);
    connect(this, &DesktopAppNotificationBase::activated, this, &DesktopAppNotificationBase::deleteLater, Qt::QueuedConnection);
}

int DesktopAppNotificationBase::getStyle() const
{
    return mStyle;
}

void DesktopAppNotificationBase::setStyle(int value)
{
    mStyle = value;
}

QStringList DesktopAppNotificationBase::getActions() const
{
    return actions;
}

void DesktopAppNotificationBase::setActions(const QStringList &value)
{
    actions = value;
}

int DesktopAppNotificationBase::getType() const
{
    return mType;
}

void DesktopAppNotificationBase::setType(int value)
{
    mType = value;
}

int64_t DesktopAppNotificationBase::getId() const
{
    return mId;
}

void DesktopAppNotificationBase::setId(const int64_t &value)
{
    mId = value;
}

QVariant DesktopAppNotificationBase::getData() const
{
    return mData;
}

void DesktopAppNotificationBase::setData(const QVariant &value)
{
    mData = value;
}

void DesktopAppNotificationBase::emitLegacyNotificationActivated()
{
    emit activated(Action::legacy);
}

QString DesktopAppNotificationBase::getTitle() const
{
    return mTitle;
}

void DesktopAppNotificationBase::setTitle(const QString &value)
{
    mTitle = value;
}

QString DesktopAppNotificationBase::getText() const
{
    return mText;
}

void DesktopAppNotificationBase::setText(const QString &value)
{
    mText = value;
}

QString DesktopAppNotificationBase::getSource() const
{
    return mSource;
}

void DesktopAppNotificationBase::setSource(const QString &value)
{
    mSource = value;
}

int DesktopAppNotificationBase::getExpirationTime() const
{
    return mExpirationTime;
}

void DesktopAppNotificationBase::setExpirationTime(int value)
{
    mExpirationTime = value;
}

QString DesktopAppNotificationBase::getImagePath() const
{
    return mImagePath;
}

void DesktopAppNotificationBase::setImagePath(const QString &value)
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

void NotificatorBase::notifySystray(DesktopAppNotificationBase *notification)
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

void NotificatorBase::notify(DesktopAppNotificationBase *notification)
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
