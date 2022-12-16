#ifndef TRANSFERNOTIFICATIONMESSAGEBUILDER_H
#define TRANSFERNOTIFICATIONMESSAGEBUILDER_H

#include "TransferMetadata.h"

#include <QObject>

class TransferNotificationMessageBuilder : public QObject
{
public:
    TransferNotificationMessageBuilder(const TransferMetaData* _data);

    QString buildTitle();
    QString buildMessage();

private:
    QString buildFileAndFolderMessage();
    QString buildFileMessage();
    QString buildFolderMessage();
    QString buildCancelStringPart();

    const TransferMetaData* data;
};

#endif // TRANSFERNOTIFICATIONMESSAGEBUILDER_H
