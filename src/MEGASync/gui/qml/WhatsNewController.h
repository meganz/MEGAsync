#ifndef WHATS_NEW_CONTROLLER_H
#define WHATS_NEW_CONTROLLER_H

#include "UpdatesList.h"

#include <QAbstractListModel>

class WhatsNewController
{
public:
    enum UpdateModelRoles
    {
        TITLE = Qt::UserRole + 1,
        DESCRIPTION,
        IMAGE
    };

    explicit WhatsNewController();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex& index, int role = TITLE) const;

    QString acceptButtonText();
    void acceptButtonClicked();

private:
    void init();

private:
    UpdatesList mUpdatesModel;
};
#endif // WHATS_NEW_CONTROLLER_H
