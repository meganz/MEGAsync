#ifndef MEGAAPIPOSIXHTTPIO_H
#define MEGAAPIPOSIXHTTPIO_H

#include "mega/posix/meganet.h"

namespace mega {

class MegaProxySettings
{
public:
    enum ProxyType {NONE = 0, AUTO = 1, CUSTOM = 2};

    MegaProxySettings();
    void setProxyType(int proxyType);
    void setProxyURL(string *proxyURL);
    void setCredentials(string *username, string *password);
    int getProxyType();
    string getProxyURL();
    bool credentialsNeeded();
    string getUsername();
    string getPassword();

protected:
    int proxyType;
    string proxyURL;
    string username;
    string password;
};

class MegaApiCurlHttpIO : public CurlHttpIO
{
protected:
    string proxyUsername;
    string proxyPassword;

public:
    void setProxy(MegaProxySettings *proxySettings);
    void post(HttpReq* req, const char* data, unsigned len);
};


}

#endif // MEGAAPIPOSIXHTTPIO_H
