#ifndef MEGAAPIWINHTTPIO_H
#define MEGAAPIWINHTTPIO_H

#include "mega/win32/meganet.h"

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

class MegaApiWinHttpIO : public WinHttpIO
{
protected:
    string proxyUsername;
    string proxyPassword;

public:
    void setProxy(MegaProxySettings *proxySettings);
    void post(HttpReq* req, const char* data, unsigned len);
};

}

#endif // MEGAAPIWINHTTPIO_H
