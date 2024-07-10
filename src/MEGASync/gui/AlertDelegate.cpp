#include "AlertDelegate.h"

#include "MegaApplication.h"

#include "megaapi.h"

#include <QPainter>
#include <QEvent>
#include <QSortFilterProxyModel>
#include <QDesktopServices>
#include <QUrl>
#include <QtConcurrent/QtConcurrent>
#include <QHelpEvent>
#include <QToolTip>

#include <cassert>

namespace
{
//constexpr int DefaultWidth = 400;
//constexpr int DEFAULT_HEIGHT = 122;
constexpr int MaxCost = 16;
}

using namespace mega;

AlertDelegate::AlertDelegate()
    : mItems(MaxCost)
{
}

QWidget* AlertDelegate::getWidget(MegaUserAlertExt* alert)
{
    if(!alert)
    {
        return nullptr;
    }

    auto alertItem = mItems[alert->getId()];
    if(alertItem)
    {
        return alertItem;
    }
    else
    {
        AlertItem* item = new AlertItem();
        item->setAlertData(alert);
        mItems.insert(alert->getId(), item);
        return mItems[alert->getId()];
    }
}
/*
void AlertDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.isValid())
    {
        NotificationAlertModelItem* item = getModelItem(index);
        if (!item)
        {
            return;
        }

        MegaUserAlertExt* alert = nullptr;//static_cast<MegaUserAlertExt*>(item->pointer);
        if (!alert)
        {
            assert(false || "No alert found");
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }

        painter->save();
        painter->translate(option.rect.topLeft());

        //handleAlertItem(alert, option.rect, painter);

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize AlertDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.isValid())
    {
        return QSize(DefaultWidth, DEFAULT_HEIGHT);
    }
    else
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}

bool AlertDelegate::editorEvent(QEvent* event, QAbstractItemModel *model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (!index.isValid())
    {
        return true;
    }

    if (QEvent::MouseButtonPress ==  event->type())
    {
        NotificationAlertModelItem* item = getModelItem(index);
        if (!item)
        {
            return true;
        }

        MegaUserAlertExt* alert = nullptr;//static_cast<MegaUserAlertExt*>(item->pointer);
        if (!alert)
        {
            return true;
        }

        MegaApi* api = MegaSyncApp->getMegaApi();

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
                        MegaContactRequest* request = icr->get(i);
                        if (!request)
                        {
                            continue;
                        }

                        const char* email = request->getSourceEmail();
                        if (alert->getEmail().toStdString().c_str() == email)
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

                MegaUser* user = api->getContact(alert->getEmail().toStdString().c_str());
                if (user && user->getVisibility() == MegaUser::VISIBILITY_VISIBLE)
                {
                    Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/%1")
                                            .arg(QString::fromUtf8(api->userHandleToBase64(user->getHandle())))));
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
                MegaNode* node = api->getNodeByHandle(alert->getNodeHandle());
                if (node)
                {
                    Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/%1")
                                            .arg(QString::fromUtf8(node->getBase64Handle()))));
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
            {
                break;
            }
        }
    }
    return QAbstractItemDelegate::editorEvent(event, model, option, index);
}

bool AlertDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (!index.isValid())
    {
        return true;
    }

    if (event->type() == QEvent::ToolTip)
    {
        NotificationAlertModelItem* item = getModelItem(index);
        if (!item)
        {
            return true;
        }

        MegaUserAlertExt* alert = nullptr;//static_cast<MegaUserAlertExt*>(item->pointer);
        if (!alert)
        {
            return QStyledItemDelegate::helpEvent(event, view, option, index);
        }

        AlertItem* ti = mAlertsModel->alertItems[alert->getId()];
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


void AlertDelegate::handleAlertItem(MegaUserAlertExt* alert, const QRect& rect, QPainter* painter) const
{
    int id = alert->getId();
    AlertItem* alertItem = mAlertsModel->alertItems[id];
    bool isNew = !alertItem;
    if (!alertItem)
    {
        alertItem = new AlertItem();
        connect(alertItem, &AlertItem::refreshAlertItem, mAlertsModel, &AlertModel::refreshAlertItem);
        alertItem->setAlertData(alert);
    }

    alertItem->resize(rect.width(), rect.height());
    alertItem->render(painter, QPoint(0, 0), QRegion(0, 0, rect.width(), rect.height()));

    if(isNew)
    {
        mAlertsModel->alertItems.insert(id, alertItem);
    }
}

NotificationAlertModelItem* AlertDelegate::getModelItem(const QModelIndex &index) const
{
    NotificationAlertModelItem* item = nullptr;
    const QSortFilterProxyModel* proxyModel = static_cast<const QSortFilterProxyModel*>(index.model());
    QModelIndex filteredIndex = proxyModel->mapToSource(index);
    if (filteredIndex.isValid())
    {
        item = static_cast<NotificationAlertModelItem*>(filteredIndex.internalPointer());
        if (item && (item->type != NotificationAlertModelItem::ALERT || !item->pointer))
        {
            item = nullptr;
        }
    }
    return item;
}
*/
