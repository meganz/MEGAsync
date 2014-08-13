#ifndef QTMEGAAPI_H
#define QTMEGAAPI_H

#include "megaapi.h"

class QTMegaApi : public mega::MegaApi
{
public:
    QTMegaApi(const char *appKey, const char *basePath = NULL, const char *userAgent = NULL);

protected:
    virtual void fetchnodes_result(mega::error e);
    virtual void syncupdate_treestate(mega::LocalNode *l);
};

#endif // QTMEGAAPI_H
