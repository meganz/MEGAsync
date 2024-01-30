#include "MegaAlertDelegate.h"
#include <QPainter>
#include "megaapi.h"
#include <QEvent>
#include <QSortFilterProxyModel>
#include <QDesktopServices>
#include <QUrl>
#include "MegaApplication.h"
#include <QtConcurrent/QtConcurrent>
#include "assert.h"
#include <QHelpEvent>
#include <QToolTip>

using namespace mega;

MegaAlertDelegate::MegaAlertDelegate(QAlertsModel *model, bool useProxyModel, QObject *parent)
    : QStyledItemDelegate(parent),
      mAlertsModel(model),
      mUseProxy(useProxyModel)
{
}

void MegaAlertDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid())
    {       

        //Map index when we are using QSortFilterProxyModel
        // if we are using QAbstractItemModel just access internalPointer casting to MegaAlert
        MegaUserAlert *alert = NULL;
        if (mUseProxy)
        {
            QModelIndex actualId = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
            if (!(actualId.isValid()))
            {
                QStyledItemDelegate::paint(painter, option, index);
                return;
            }

            alert = (MegaUserAlert *)actualId.internalPointer();
        }
        else
        {
            alert = (MegaUserAlert *)index.internalPointer();
        }

        if (!alert)
        {
            assert(false || "No alert found");
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        AlertItem *ti = mAlertsModel->alertItems[alert->getId()];
        if (!ti)
        {
            ti = new AlertItem();
            connect(ti, &AlertItem::refreshAlertItem, mAlertsModel, &QAlertsModel::refreshAlertItem);

            mAlertsModel->alertItems.insert(alert->getId(), ti);


            ti->setAlertData(alert); //Just set when created and when updated at QAlertsModel
        }

        painter->save();
        painter->translate(option.rect.topLeft());

        ti->resize(option.rect.width(), option.rect.height());

        ti->render(painter, QPoint(0, 0), QRegion(0, 0, option.rect.width(), option.rect.height()));
        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize MegaAlertDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid())
    {
        return QSize(400, 122);
    }
    else
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

bool MegaAlertDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!index.isValid())
    {
        return true;
    }

    if (QEvent::MouseButtonPress ==  event->type())
    {
        MegaUserAlert *alert = NULL;
        if (mUseProxy)
        {
            QModelIndex actualId = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
            if (!(actualId.isValid()))
            {
                return true;
            }

            alert = (MegaUserAlert *)actualId.internalPointer();
        }
        else
        {
            alert = (MegaUserAlert *)index.internalPointer();
        }

        if (!alert)
        {
            return true;
        }

        MegaApi *api = ((MegaApplication*)qApp)->getMegaApi();

        switch (alert->getType())
        {
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
                {
                    bool found = false;
                    std::unique_ptr<MegaContactRequestList> icr(api->getIncomingContactRequests());
                    if (icr)
                    {
                        for (int i = 0; i < icr->size(); i++)
                        {
                            MegaContactRequest *request = icr->get(i);
                            if (!request)
                            {
                                continue;
                            }

                            const char* email = request->getSourceEmail();
                            if (!strcmp(alert->getEmail(), email))
                            {
                                found = true;
                                Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/ipc")));
                                break;
                            }
                        }
                    }

                    if (!found)
                    {
                        Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/contacts")));
                    }

                    break;
                }

            case MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
                {

                    MegaUser *user = api->getContact(alert->getEmail());
                    if (user && user->getVisibility() == MegaUser::VISIBILITY_VISIBLE)
                    {
                        Utilities::openUrl(
                                          QUrl(QString::fromUtf8("mega://#fm/%1").arg(QString::fromUtf8(api->userHandleToBase64(user->getHandle())))));
                        delete user;
                    }
                    else
                    {
                        Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/contacts")));
                    }

                    break;
                }
            case MegaUserAlert::TYPE_NEWSHARE:
            case MegaUserAlert::TYPE_DELETEDSHARE:
            case MegaUserAlert::TYPE_NEWSHAREDNODES:
            case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
            case MegaUserAlert::TYPE_TAKEDOWN:
            case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
            // Disabled case MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
            // due to alert node is always NULL. If this behaviour changes, adapt to include update case

                {
                    MegaNode *node = api->getNodeByHandle(alert->getNodeHandle());
                    if (node)
                    {
                        Utilities::openUrl(
                                          QUrl(QString::fromUtf8("mega://#fm/%1").arg(QString::fromUtf8(node->getBase64Handle()))));
                        delete node;
                    }

                    break;
                }

            case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
            case MegaUserAlert::TYPE_PAYMENT_FAILED:
            case MegaUserAlert::TYPE_PAYMENTREMINDER:
                {
                    Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/account/plan")));
                    break;
                }

            default:
                break;

        }
    }
    return QAbstractItemDelegate::editorEvent(event, model, option, index);
}

bool MegaAlertDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!index.isValid())
    {
        return true;
    }

    if (event->type() == QEvent::ToolTip)
    {
        MegaUserAlert *alert = NULL;
        if (mUseProxy)
        {
            QModelIndex actualId = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
            if (!(actualId.isValid()))
            {
                return true;
            }

            alert = (MegaUserAlert *)actualId.internalPointer();
        }
        else
        {
            alert = (MegaUserAlert *)index.internalPointer();
        }

        if (!alert)
        {
            return QStyledItemDelegate::helpEvent(event, view, option, index);
        }

        AlertItem *ti = mAlertsModel->alertItems[alert->getId()];
        if (!ti)
        {
            return QStyledItemDelegate::helpEvent(event, view, option, index);
        }
        else
        {
            QToolTip::showText(event->globalPos(), ti->getHeadingString());
            return true;
        }
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}
