#ifndef UPDATES_MODEL_H
#define UPDATES_MODEL_H

#include "WhatsNewController.h"

#include <QAbstractListModel>

#include <memory>

class UpdatesModel: public QAbstractListModel
{
    Q_OBJECT

public:
    explicit UpdatesModel(std::shared_ptr<WhatsNewController> controller,
                          QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    std::shared_ptr<WhatsNewController> mUpdatesController;
};
#endif // UPDATES_MODEL_H
