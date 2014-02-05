#include "megaapiposixhttpio.h"

#include <QString>
#include "control/Preferences.h"

namespace mega {

void MegaApiCurlHttpIO::setProxy(MegaProxySettings *proxySettings)
{
}


void MegaApiCurlHttpIO::post(HttpReq* req, const char* data, unsigned len)
{
    // XXX: do we need to set Proxy here ?
    CurlHttpIO::post(req, data, len);
}

MegaProxySettings *MegaApiCurlHttpIO::getAutoProxySettings()
{
    MegaProxySettings *proxySettings = new MegaProxySettings();
    proxySettings->setProxyType(MegaProxySettings::NONE);
    return proxySettings;
}

}
