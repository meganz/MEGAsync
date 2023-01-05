#ifndef BACKUPTABLEVIEWTOOLTIPS_H
#define BACKUPTABLEVIEWTOOLTIPS_H

#include "syncs/gui/Twoways/SyncTableViewTooltips.h"

#include "UserAttributesRequests/MyBackupsHandle.h"

class BackupTableViewTooltips : public SyncTableViewTooltips
{
public:
    BackupTableViewTooltips();

private:
    virtual QString getTooltipText(const QPoint& mousePos, int columnPosX,
                           const QModelIndex& index);

    std::shared_ptr<UserAttributes::MyBackupsHandle> mMyBackupsHandleRequest;
};

#endif // BACKUPTABLEVIEWTOOLTIPS_H
