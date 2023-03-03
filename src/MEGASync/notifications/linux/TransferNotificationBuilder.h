#ifndef TRANSFERNOTIFICATIONBUILDER_H
#define TRANSFERNOTIFICATIONBUILDER_H

#include "notifications/TransferNotificationBuilderBase.h"

class TransferNotificationBuilder : public TransferNotificationBuilderBase
{
public:
    TransferNotificationBuilder(const std::shared_ptr<TransferMetaData>& data);
    ~TransferNotificationBuilder() override = default;

protected:
    QString getImagePath() override;
};

#endif // TRANSFERNOTIFICATIONBUILDER_H
