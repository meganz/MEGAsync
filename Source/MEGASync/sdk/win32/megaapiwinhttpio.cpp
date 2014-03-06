#include "megaapiwinhttpio.h"

#include <QString>
#include "control/Preferences.h"
#include "control/Utilities.h"

namespace mega {

void MegaApiWinHttpIO::setProxy(MegaProxySettings *proxySettings)
{
    LOG("Set proxy");
    if((proxySettings->getProxyType() == MegaProxySettings::CUSTOM) &&
        (proxySettings->credentialsNeeded()))
    {
        LOG("Auth proxy");
        proxyUsername = proxySettings->getUsername();
        proxyPassword = proxySettings->getPassword();
    }
    else
    {
        LOG("No Auth");
        proxyUsername.clear();
        proxyPassword.clear();
    }

    // create the session handle using the default settings.
    if(proxySettings->getProxyType() == MegaProxySettings::NONE)
    {
        LOG("No Proxy");
        WINHTTP_PROXY_INFO proxyInfo;
        proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
        proxyInfo.lpszProxy = WINHTTP_NO_PROXY_NAME;
        proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
        WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));
    }
    else if(proxySettings->getProxyType() == MegaProxySettings::CUSTOM)
    {
        LOG("Custom Proxy");
        string proxyURL = proxySettings->getProxyURL();
        LOG(QString::fromWCharArray((LPWSTR)proxyURL.data()));

        WINHTTP_PROXY_INFO proxyInfo;
        proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
        proxyInfo.lpszProxy = (LPWSTR)proxyURL.data();
        proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
        WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));
    }
    else
    {
        LOG("Auto Proxy");
        Preferences *preferences = Preferences::instance();
        preferences->setProxyServer(QString());
        MegaProxySettings *proxySettings = getAutoProxySettings();

        if(proxySettings->getProxyType() == MegaProxySettings::CUSTOM)
        {
            LOG("Custom Proxy");
            string proxyURL = proxySettings->getProxyURL();
            WINHTTP_PROXY_INFO proxyInfo;
            proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
            proxyInfo.lpszProxy = (LPWSTR)proxyURL.data();
            proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
            WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));

            QString ieProxy = QString::fromWCharArray((LPWSTR)proxyURL.data());
            QStringList params = ieProxy.split(QChar::fromAscii(':'));
            preferences->setProxyServer(params[0]);
            if(params.size()==2)
            {
                preferences->setProxyServer(params[0]);
                preferences->setProxyPort(params[1].toInt());
            }
            LOG(preferences->proxyHostAndPort());
        }
        else
        {
            LOG("No Proxy");
            WINHTTP_PROXY_INFO proxyInfo;
            proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
            proxyInfo.lpszProxy = WINHTTP_NO_PROXY_NAME;
            proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
            WinHttpSetOption(hSession, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));
        }
        delete proxySettings;
    }
}

MegaProxySettings *MegaApiWinHttpIO::getAutoProxySettings()
{
    LOG("Get Auto Proxy");
    MegaProxySettings *proxySettings = new MegaProxySettings();
    proxySettings->setProxyType(MegaProxySettings::NONE);

    WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig = {0};
    if(WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig) == TRUE)
    {
        LOG("ieProxy");
        if(ieProxyConfig.lpszProxy)
        {
            LOG("ieProxy available");
            string proxyURL;
            proxySettings->setProxyType(MegaProxySettings::CUSTOM);
            int len = lstrlen(ieProxyConfig.lpszProxy);
            proxyURL.assign((const char *)ieProxyConfig.lpszProxy, (len+1) * sizeof(wchar_t));

            //Only save one proxy
            for(int i=0; i<len; i++)
            {
                wchar_t *character = (wchar_t *)(proxyURL.data()+(i*sizeof(wchar_t)));
                if(((*character) == L' ') || ((*character) == L';'))
                {
                    proxyURL.resize(i*sizeof(wchar_t));
                    len = i;
                    proxyURL.append("",1);
                    break;
                }
            }

            //Remove protocol prefix, if any
            for(int i=len-1; i>=0; i--)
            {
                wchar_t *character = (wchar_t *)(proxyURL.data()+(i*sizeof(wchar_t)));
                if((*character) == L'/')
                {
                    proxyURL = proxyURL.substr((i+1)*sizeof(wchar_t));
                    break;
                }
            }

            proxySettings->setProxyURL(&proxyURL);

        }
        else if((ieProxyConfig.lpszAutoConfigUrl) || (ieProxyConfig.fAutoDetect == TRUE))
        {
            WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;
            if(ieProxyConfig.lpszAutoConfigUrl)
            {
                LOG("Auto-config Proxy");
                LOG(QString::fromWCharArray(ieProxyConfig.lpszAutoConfigUrl));

                autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
                autoProxyOptions.lpszAutoConfigUrl = ieProxyConfig.lpszAutoConfigUrl;
                autoProxyOptions.dwAutoDetectFlags = 0;
            }
            else
            {
                LOG("Auto detect");
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
                LOG("Proxy for URL");
                if(proxyInfo.lpszProxy)
                {
                    LOG("Valid proxy for URL");
                    string proxyURL;
                    proxySettings->setProxyType(MegaProxySettings::CUSTOM);
                    proxyURL.assign((const char *)proxyInfo.lpszProxy, (lstrlen(proxyInfo.lpszProxy)+1) * sizeof(wchar_t));
                    proxySettings->setProxyURL(&proxyURL);
                }
            }
        }
    }

    if(ieProxyConfig.lpszProxy) GlobalFree(ieProxyConfig.lpszProxy);
    if(ieProxyConfig.lpszProxyBypass) GlobalFree(ieProxyConfig.lpszProxyBypass);
    if(ieProxyConfig.lpszAutoConfigUrl) GlobalFree(ieProxyConfig.lpszAutoConfigUrl);

    return proxySettings;
}

void MegaApiWinHttpIO::post(HttpReq* req, const char* data, unsigned len)
{
    if (debug)
    {
        cout << "POST target URL: " << req->posturl << endl;

        if (req->binary)
        {
            cout << "[sending " << req->out->size() << " bytes of raw data]" << endl;
        }
        else
        {
            cout << "Sending: " << *req->out << endl;
        }
    }

    WinHttpContext* httpctx;

    WCHAR szURL[8192];
    WCHAR szHost[256];
    URL_COMPONENTS urlComp = { sizeof urlComp };

    urlComp.lpszHostName = szHost;
    urlComp.dwHostNameLength = sizeof szHost / sizeof *szHost;
    urlComp.dwUrlPathLength = (DWORD)-1;
    urlComp.dwSchemeLength = (DWORD)-1;

    httpctx = new WinHttpContext;

    httpctx->httpio = this;
    httpctx->req = req;
    req->httpiohandle = (void*)httpctx;

    if (MultiByteToWideChar(CP_UTF8, 0, req->posturl.c_str(), -1, szURL,
                            sizeof szURL / sizeof *szURL)
            && WinHttpCrackUrl(szURL, 0, 0, &urlComp))
    {
        if (( httpctx->hConnect = WinHttpConnect(hSession, szHost, urlComp.nPort, 0)))
        {
            httpctx->hRequest = WinHttpOpenRequest(httpctx->hConnect, L"POST",
                                                   urlComp.lpszUrlPath, NULL,
                                                   WINHTTP_NO_REFERER,
                                                   WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                   ( urlComp.nScheme == INTERNET_SCHEME_HTTPS )
                                                   ? WINHTTP_FLAG_SECURE
                                                   : 0);

            if (httpctx->hRequest)
            {
                if(proxyUsername.size())
                {
                    WinHttpSetCredentials(httpctx->hRequest, WINHTTP_AUTH_TARGET_PROXY,
                                  WINHTTP_AUTH_SCHEME_BASIC,
                                  (LPWSTR)proxyUsername.data(), (LPWSTR)proxyPassword.data(), NULL);
                }

                WinHttpSetTimeouts(httpctx->hRequest, 0, 20000, 20000, 300000);

                WinHttpSetStatusCallback(httpctx->hRequest, asynccallback,
                                         WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE
                                         | WINHTTP_CALLBACK_FLAG_READ_COMPLETE
                                         | WINHTTP_CALLBACK_FLAG_HEADERS_AVAILABLE
                                         | WINHTTP_CALLBACK_FLAG_REQUEST_ERROR
                                         | WINHTTP_CALLBACK_FLAG_SECURE_FAILURE
                                         | WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE
                                         | WINHTTP_CALLBACK_FLAG_WRITE_COMPLETE
                                         | WINHTTP_CALLBACK_FLAG_HANDLES,
                                         0);

                LPCWSTR pwszHeaders = ( req->type == REQ_JSON )
                                      ? L"Content-Type: application/json"
                                      : L"Content-Type: application/octet-stream";

                // data is sent in HTTP_POST_CHUNK_SIZE instalments to ensure
                // semi-smooth UI progress info
                httpctx->postlen = data ? len : req->out->size();
                httpctx->postdata = data ? data : req->out->data();
                httpctx->postpos = ( httpctx->postlen < HTTP_POST_CHUNK_SIZE )
                                   ? httpctx->postlen
                                   : HTTP_POST_CHUNK_SIZE;

                if (WinHttpSendRequest(httpctx->hRequest, pwszHeaders,
                                       wcslen(pwszHeaders),
                                       (LPVOID)httpctx->postdata,
                                       httpctx->postpos,
                                       httpctx->postlen,
                                       (DWORD_PTR)httpctx))
                {
                    req->status = REQ_INFLIGHT;
                    return;
                }
            }
        }
        else
        {
            httpctx->hRequest = NULL;
        }
    }
    else
    {
        httpctx->hRequest = NULL;
        httpctx->hConnect = NULL;
    }

    cout << "REQ_FAILURE  WinHttpIO::post()" << endl;
    req->status = REQ_FAILURE;
}

}
