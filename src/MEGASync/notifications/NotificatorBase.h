// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NOTIFICATORBASE_H
#define NOTIFICATORBASE_H

#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QMutex>
#include <QHash>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QSystemTrayIcon;
QT_END_NAMESPACE

class MegaNotificationBase : public QObject
{
    Q_OBJECT

public:
    static const QString& defaultImage;

    enum class CloseReason {
        Unknown = 0,
        UserAction,
        AppHidden,
        TimedOut
    };

    enum class Action {
        content = -1,
        firstButton = 0,
        secondButton = 1,
        legacy = 2
    };

    MegaNotificationBase();
    virtual ~MegaNotificationBase() = default;

    QString getTitle() const;
    void setTitle(const QString &value);
    QString getText() const;
    void setText(const QString &value);
    QString getSource() const;
    void setSource(const QString &value);
    int getExpirationTime() const;
    void setExpirationTime(int value);
    QString getImagePath() const;
    virtual void setImagePath(const QString &value);
    int getStyle() const;
    void setStyle(int value);
    virtual QStringList getActions() const;
    void setActions(const QStringList &value);
    int getType() const;
    void setType(int value);
    int64_t getId() const;
    void setId(const int64_t &value);
    QVariant getData() const;
    void setData(const QVariant &value);

    void emitLegacyNotificationActivated();

protected:
    QString mTitle;
    QString mText;
    QString mSource;
    int mExpirationTime;
    QString mImagePath;
    int mStyle;
    QStringList actions;
    int mType;
    int64_t mId;
    QVariant mData;

signals:
    void activated(MegaNotificationBase::Action action);
    void closed(MegaNotificationBase::CloseReason reason);
    void failed();
};

/** Cross-platform desktop notification client. */
class NotificatorBase: public QObject
{
    Q_OBJECT

public:
    /** Create a new notificator.
       @note Ownership of trayIcon is not transferred to this object.
    */
    NotificatorBase(const QString &programName, QSystemTrayIcon *trayIcon, QObject *parent);
    virtual ~NotificatorBase() = default;

    // Message class
    enum Class
    {
        Information,    /**< Informational message */
        Warning,        /**< Notify user of potential problem */
        Critical        /**< An error occurred */
    };

public slots:
    /** Show notification message.
       @param[in] cls    general message class
       @param[in] title  title shown with message
       @param[in] text   message content
       @param[in] icon   optional icon to show with message
       @param[in] millisTimeout notification timeout in milliseconds (defaults to 10 seconds)
       @note Platform implementations are free to ignore any of the provided fields except for \a text.
     */
    void notify(NotificatorBase::Class cls, const QString &title, const QString &text, int millisTimeout = 10000);
    void notify(MegaNotificationBase *notification);

protected:

    enum Mode {
        None,                       /**< Ignore informational notifications, and show a modal pop-up dialog for Critical notifications. */
        Freedesktop,                /**< Use DBus org.freedesktop.Notifications */
        QSystemTray,                /**< Use QSystemTray::showMessage */
        UserNotificationCenter      /**< Use the 10.8+ User Notification Center (Mac only) */
    };

    QString mProgramName;
    Mode mMode;
    QSystemTrayIcon *mTrayIcon;
    QPointer<MegaNotificationBase> mCurrentNotification;

    virtual void notifySystray(Class cls, const QString &title, const QString &text, int millisTimeout, bool forceQt = false);
    virtual void notifySystray(MegaNotificationBase *notification);

protected slots:
    void onMessageClicked();

};

#endif // NOTIFICATOR_H
