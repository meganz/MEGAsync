#include "megaapiwinhttpio.h"

#include <QString>
#include "control/Preferences.h"

namespace mega {

MegaProxySettings::MegaProxySettings() { proxyType = AUTO; }
void MegaProxySettings::setProxyType(int proxyType) { this->proxyType = proxyType; }
void MegaProxySettings::setProxyURL(string *proxyURL) { this->proxyURL = *proxyURL; }
void MegaProxySettings::setCredentials(string *username, string *password) { this->username = *username; this->password = *password; }
int MegaProxySettings::getProxyType() { return proxyType; }
string MegaProxySettings::getProxyURL() { return this->proxyURL; }
bool MegaProxySettings::credentialsNeeded() { return (username.size() != 0); }
string MegaProxySettings::getUsername() { return username; }
string MegaProxySettings::getPassword() { return password; }

void MegaApiWinHttpIO::setProxy(MegaProxySettings *proxySettings)
{
    if(proxySettings->credentialsNeeded())
    {
        proxyUsername = proxySettings->getUsername();
        proxyPassword = proxySettings->getPassword();
    }
    else
    {
        proxyUsername.clear();
        proxyPassword.clear();
    }

    // create the session handle using the default settings.
    if(proxySettings->getProxyType() == MegaProxySettings::NONE)
    {
        WINHTTP_PROXY_INFO proxyInfo;
        proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
        proxyInfo.lpszProxy = WINHTTP_NO_PROXY_NAME;
        proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
        WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));
    }
    else if(proxySettings->getProxyType() == MegaProxySettings::CUSTOM)
    {
        WINHTTP_PROXY_INFO proxyInfo;
        proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
        proxyInfo.lpszProxy = (LPWSTR)proxySettings->getProxyURL().data();
        proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
        WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));
    }
    else
    {
        WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig = {0};
        if(WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig) == TRUE)
        {
            if(ieProxyConfig.lpszProxy)
            {
                Preferences *preferences = Preferences::instance();
                QString ieProxy = QString::fromWCharArray(ieProxyConfig.lpszProxy);
                QStringList params = ieProxy.split(QChar::fromAscii(':'));
                preferences->setProxyServer(params[0]);
                if(params.size()>1)
                    preferences->setProxyPort(params[1].toInt());

                WINHTTP_PROXY_INFO proxyInfo;
                proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
                proxyInfo.lpszProxy = ieProxyConfig.lpszProxy;
                proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
                WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));
            }
            else
            {
                if((ieProxyConfig.lpszAutoConfigUrl) || (ieProxyConfig.fAutoDetect == TRUE))
                {
                    WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;
                    if(ieProxyConfig.lpszAutoConfigUrl)
                    {
                        autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
                        autoProxyOptions.lpszAutoConfigUrl = ieProxyConfig.lpszAutoConfigUrl;
                        autoProxyOptions.dwAutoDetectFlags = 0;
                    }
                    else
                    {
                        autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
                        autoProxyOptions.lpszAutoConfigUrl = NULL;
                        autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
                    }
                    autoProxyOptions.fAutoLogonIfChallenged = TRUE;
                    autoProxyOptions.lpvReserved = NULL;
                    autoProxyOptions.dwReserved = 0;

                    WINHTTP_PROXY_INFO proxyInfo;
                    if(WinHttpGetProxyForUrl(hSession, L"https://g.api.mega.co.nz/", &autoProxyOptions, &proxyInfo))
                    {
                        Preferences *preferences = Preferences::instance();
                        if(proxyInfo.lpszProxy)
                        {
                            QString ieProxy = QString::fromWCharArray(proxyInfo.lpszProxy);
                            QStringList params = ieProxy.split(QChar::fromAscii(':'));
                            preferences->setProxyServer(params[0]);
                            if(params.size()>1)
                                preferences->setProxyPort(params[1].toInt());
                        }
                        WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));
                    }
                    else
                    {
                        WINHTTP_PROXY_INFO proxyInfo;
                        proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
                        proxyInfo.lpszProxy = WINHTTP_NO_PROXY_NAME;
                        proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
                        WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));
                    }
                }
                else
                {
                    WINHTTP_PROXY_INFO proxyInfo;
                    proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
                    proxyInfo.lpszProxy = WINHTTP_NO_PROXY_NAME;
                    proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
                    WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));
                }
            }
        }
        else
        {
            WINHTTP_PROXY_INFO proxyInfo;
            proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
            proxyInfo.lpszProxy = WINHTTP_NO_PROXY_NAME;
            proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
            WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));
        }
        if(ieProxyConfig.lpszProxy) GlobalFree(ieProxyConfig.lpszProxy);
        if(ieProxyConfig.lpszProxyBypass) GlobalFree(ieProxyConfig.lpszProxyBypass);
        if(ieProxyConfig.lpszAutoConfigUrl) GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
    }
}

void MegaApiWinHttpIO::post(HttpReq* req, const char* data, unsigned len)
{
    if (debug)
    {
        cout << "POST target URL: " << req->posturl << endl;

        if (req->binary) cout << "[sending " << req->out->size() << " bytes of raw data]" << endl;
        else cout << "Sending: " << *req->out << endl;
    }

    WinHttpContext* httpctx;

    WCHAR szURL[8192];
    WCHAR szHost[256];
    URL_COMPONENTS urlComp = { sizeof urlComp };

    urlComp.lpszHostName = szHost;
    urlComp.dwHostNameLength = sizeof szHost/sizeof *szHost;
    urlComp.dwUrlPathLength = (DWORD)-1;
    urlComp.dwSchemeLength = (DWORD)-1;

    httpctx = new WinHttpContext;

    httpctx->httpio = this;
    httpctx->req = req;
    req->httpiohandle = (void*)httpctx;

    if (MultiByteToWideChar(CP_UTF8,0,req->posturl.c_str(),-1,szURL,sizeof szURL/sizeof *szURL) && WinHttpCrackUrl(szURL,0,0,&urlComp))
    {
        if ((httpctx->hConnect = WinHttpConnect(hSession,szHost,urlComp.nPort,0)))
        {
            httpctx->hRequest = WinHttpOpenRequest(httpctx->hConnect,L"POST",urlComp.lpszUrlPath,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,(urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);

            if (httpctx->hRequest)
            {
                if(proxyUsername.size())
                {
                    WinHttpSetCredentials(httpctx->hRequest, WINHTTP_AUTH_TARGET_PROXY,
                                  WINHTTP_AUTH_SCHEME_BASIC,
                                  (LPWSTR)proxyUsername.data(), (LPWSTR)proxyPassword.data(), NULL);
                }

                WinHttpSetTimeouts(httpctx->hRequest,0,20000,20000,1800000);

                WinHttpSetStatusCallback(httpctx->hRequest,asynccallback,WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE | WINHTTP_CALLBACK_FLAG_READ_COMPLETE | WINHTTP_CALLBACK_FLAG_HEADERS_AVAILABLE | WINHTTP_CALLBACK_FLAG_REQUEST_ERROR | WINHTTP_CALLBACK_FLAG_SECURE_FAILURE | WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE | WINHTTP_CALLBACK_FLAG_WRITE_COMPLETE | WINHTTP_CALLBACK_FLAG_HANDLES ,0);

                LPCWSTR pwszHeaders = req->type == REQ_JSON ? L"Content-Type: application/json" : L"Content-Type: application/octet-stream";

                // data is sent in HTTP_POST_CHUNK_SIZE instalments to ensure semi-smooth UI progress info
                httpctx->postlen = data ? len : req->out->size();
                httpctx->postdata = data ? data : req->out->data();
                httpctx->postpos = (httpctx->postlen < HTTP_POST_CHUNK_SIZE) ? httpctx->postlen : HTTP_POST_CHUNK_SIZE;

                if (WinHttpSendRequest(httpctx->hRequest,pwszHeaders,wcslen(pwszHeaders),(LPVOID)httpctx->postdata,httpctx->postpos,httpctx->postlen,(DWORD_PTR)httpctx))
                {
                    req->status = REQ_INFLIGHT;
                    return;
                }
            }
        }
        else httpctx->hRequest = NULL;
    }
    else
    {
        httpctx->hRequest = NULL;
        httpctx->hConnect = NULL;
    }

    req->status = REQ_FAILURE;
}

}
