#ifndef MEGASYSTEMTRAYICON_P_H
#define MEGASYSTEMTRAYICON_P_H

#include <QIcon>
#include <QRect>
#include <QString>
#include <QSystemTrayIcon>
#include <QMenu>

class QSystemTrayIconSys;
class MegaSystemTrayIcon : public QObject
{
    Q_OBJECT

public:
    MegaSystemTrayIcon();
    virtual ~MegaSystemTrayIcon();

    virtual void show();
    virtual void hide();
    virtual void setIcon(const QIcon &icon,const QIcon &iconWhite);
    virtual void setToolTip(const QString &toolTip);
    virtual void setContextMenu(QMenu *menu);
    virtual QRect geometry() const;
    virtual void showMessage(const QString &msg, const QString &title,
                             const QIcon& icon, QSystemTrayIcon::MessageIcon iconType, int secs);

    virtual bool isSystemTrayAvailable() const;
    virtual bool supportsMessages() const;
    virtual QPoint getPosition();

signals:
    void activated(QSystemTrayIcon::ActivationReason reason);
    void messageClicked();

private:
    QSystemTrayIconSys *m_sys;
};

#endif
