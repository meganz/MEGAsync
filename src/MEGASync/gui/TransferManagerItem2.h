#ifndef TRANSFERMANAGERITEM2_H
#define TRANSFERMANAGERITEM2_H

#include "TransferItem2.h"

#include <QDateTime>

namespace Ui {
class TransferManagerItem;
}

class TransferManagerItem2 : public QWidget
{
        Q_OBJECT

public:
    explicit TransferManagerItem2(QWidget *parent = 0);

    void updateUi(const TransferItem2 transferItem);
//    void setupUi(const TransferItem2 transferItem);

    private:
        Ui::TransferManagerItem *mUi;
};

#endif // TRANSFERMANAGERITEM2_H
