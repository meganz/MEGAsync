#ifndef MEGAAPIWINHTTPIO_H
#define MEGAAPIWINHTTPIO_H

#include "mega/win32/meganet.h"
#include "MegaProxySettings.h"

namespace mega {

class MegaApiWinHttpIO : public WinHttpIO
{
protected:
    string proxyUsername;
    string proxyPassword;

public:
    void setProxy(MegaProxySettings *proxySettings);
    MegaProxySettings *getAutoProxySettings();
    void post(HttpReq* req, const char* data, unsigned len);
};

}

#endif // MEGAAPIWINHTTPIO_H
