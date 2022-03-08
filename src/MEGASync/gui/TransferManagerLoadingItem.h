#ifndef TRANSFERMANAGERLOADINGITEM_H
#define TRANSFERMANAGERLOADINGITEM_H

#include <QWidget>

namespace Ui {
class TransferManagerLoadingItem;
}

class TransferManagerLoadingItem : public QWidget
{
    Q_OBJECT

public:
    explicit TransferManagerLoadingItem(QWidget *parent = nullptr);
    ~TransferManagerLoadingItem();

    static QSize widgetSize();

private:
    Ui::TransferManagerLoadingItem *ui;
};

#endif // TRANSFERMANAGERLOADINGITEM_H
