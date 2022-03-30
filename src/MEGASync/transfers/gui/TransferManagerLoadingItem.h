#ifndef TRANSFERMANAGERLOADINGITEM_H
#define TRANSFERMANAGERLOADINGITEM_H

#include <QAbstractItemView>

namespace Ui {
class TransferManagerLoadingItem;
}

class TransferManagerLoadingItem : public QWidget
{
    Q_OBJECT

public:
    explicit TransferManagerLoadingItem(QAbstractItemView *parent = nullptr);
    ~TransferManagerLoadingItem();

    static QSize widgetSize();

private:
    Ui::TransferManagerLoadingItem *ui;
};

#endif // TRANSFERMANAGERLOADINGITEM_H
