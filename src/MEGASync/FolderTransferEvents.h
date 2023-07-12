#ifndef FOLDERTRANSFEREVENT_H
#define FOLDERTRANSFEREVENT_H

#include <megaapi.h>
#include <QString>

struct FolderTransferUpdateEvent
{
    int stage;
    uint32_t foldercount;
    uint32_t createdfoldercount;
    uint32_t filecount;
    QString transferName;
    std::string appData;
};

#endif // FOLDERTRANSFEREVENT_H
