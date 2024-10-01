#ifndef UPSELL_MODEL_H
#define UPSELL_MODEL_H

#include <QAbstractListModel>

#include <memory>

class UpsellController;

class UpsellModel: public QAbstractListModel
{
    Q_OBJECT

public:
    explicit UpsellModel(std::shared_ptr<UpsellController> controller, QObject* parent = nullptr);
    virtual ~UpsellModel() = default;

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    std::shared_ptr<UpsellController> mController;
};

#endif // UPSELL_MODEL_H
