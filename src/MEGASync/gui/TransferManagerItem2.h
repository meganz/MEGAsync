#ifndef TRANSFERMANAGERITEM2_H
#define TRANSFERMANAGERITEM2_H

#include "TransferItem2.h"

#include <QDateTime>

namespace Ui {
class TransferManagerItem;
}

class TransferManagerItem2 : public TransferItem2
{
public:
        TransferManagerItem2();

    void updateUi(Ui::TransferManagerItem* ui) const;
    void setupUi(Ui::TransferManagerItem* ui, QWidget* view) const;


private:
    QString mActiveStatus;
};

Q_DECLARE_METATYPE(TransferManagerItem2);

#endif // TRANSFERMANAGERITEM2_H
