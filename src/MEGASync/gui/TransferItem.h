#ifndef TRANSFERITEM_H
#define TRANSFERITEM_H

#include <QWidget>

namespace Ui {
class TransferItem;
}

class TransferItem : public QWidget
{
    Q_OBJECT

public:
    explicit TransferItem(QWidget *parent = 0);
    ~TransferItem();

private:
    Ui::TransferItem *ui;

};

Q_DECLARE_METATYPE(TransferItem*)

#endif // TRANSFERITEM_H
