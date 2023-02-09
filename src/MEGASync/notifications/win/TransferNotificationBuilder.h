#ifndef TRANSFERNOTIFICATIONBUILDER_H
#define TRANSFERNOTIFICATIONBUILDER_H

#include "model/TransferMetaData.h"
#include "TransferNotificationBuilderBase.h"

#include <megaapi.h>
#include <QObject>

class TransferNotificationBuilder : public TransferNotificationBuilderBase
{
public:
    TransferNotificationBuilder(const std::shared_ptr<TransferMetaData>& data);
};

#endif // TRANSFERNOTIFICATIONBUILDER_H
