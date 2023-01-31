#include "TransferNotificationBuilder.h"

#include <megaapi.h>

#include <QString>

using namespace mega;

/*
 * TODO: AFTER THE NOTIFICATIONS REVAMP, CHECK IF THIS CLASS IS STILL NEEDED.
 * THIS CLASS HAS BEEN CREATED TO SHOW THAT IF THE NOTIFICATIONS ARE DIFFERENT ON EACH OS
 * IT IS BETTER TO CREATE A CLASS ´
 */

TransferNotificationBuilder::TransferNotificationBuilder(const std::shared_ptr<TransferMetaData> &data)
    : TransferNotificationBuilderBase(data)
{
}
