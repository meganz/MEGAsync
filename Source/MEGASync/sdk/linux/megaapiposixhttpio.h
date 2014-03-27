#ifndef MEGAAPIPOSIXHTTPIO_H
#define MEGAAPIPOSIXHTTPIO_H

#include "mega/posix/meganet.h"
#include "MegaProxySettings.h"

namespace mega {

class MegaApiCurlHttpIO : public CurlHttpIO
{
protected:
    bool proxyEnabled;
    string proxy;
    string proxyUsername;
    string proxyPassword;

public:
    MegaApiCurlHttpIO();
    void setProxy(MegaProxySettings *proxySettings);
    MegaProxySettings *getAutoProxySettings();
    void post(HttpReq* req, const char* data, unsigned len);
};


}

#endif // MEGAAPIPOSIXHTTPIO_H
