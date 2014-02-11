#ifndef MEGAAPIPOSIXHTTPIO_H
#define MEGAAPIPOSIXHTTPIO_H

#include "mega/posix/meganet.h"
#include "MegaProxySettings.h"

namespace mega {

class MegaApiCurlHttpIO : public CurlHttpIO
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

#endif // MEGAAPIPOSIXHTTPIO_H
