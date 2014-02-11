#ifndef MEGAPROXYSETTINGS_H
#define MEGAPROXYSETTINGS_H

#include <string>

class MegaProxySettings
{
public:
    enum ProxyType {NONE = 0, AUTO = 1, CUSTOM = 2};

    MegaProxySettings();
    void setProxyType(int proxyType);
    void setProxyURL(std::string *proxyURL);
    void setCredentials(std::string *username, std::string *password);
    int getProxyType();
    std::string getProxyURL();
    bool credentialsNeeded();
    std::string getUsername();
    std::string getPassword();

protected:
    int proxyType;
    std::string proxyURL;
    std::string username;
    std::string password;
};

#endif // MEGAPROXYSETTINGS_H
