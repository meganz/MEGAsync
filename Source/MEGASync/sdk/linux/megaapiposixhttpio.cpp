#include "megaapiposixhttpio.h"

#include <QString>
#include "control/Preferences.h"

namespace mega {

MegaApiCurlHttpIO::MegaApiCurlHttpIO() : CurlHttpIO()
{
    proxyEnabled = false;
}

void MegaApiCurlHttpIO::setProxy(MegaProxySettings *proxySettings)
{
    if(proxySettings->getProxyType() != MegaProxySettings::CUSTOM)
    {
        //Automatic proxy still unsupported
        proxyEnabled = false;
        return;
    }

    proxyEnabled = true;
    proxy = proxySettings->getProxyURL();
    proxyUsername = proxySettings->getUsername();
    proxyPassword = proxySettings->getPassword();
}


void MegaApiCurlHttpIO::post(HttpReq* req, const char* data, unsigned len)
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

    CURL* curl;

    req->in.clear();

    if ((curl = curl_easy_init()))
    {
        curl_easy_setopt(curl, CURLOPT_URL, req->posturl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data ? data : req->out->data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data ? len : req->out->size());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent->c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, req->type == REQ_JSON ? contenttypejson : contenttypebinary);
        curl_easy_setopt(curl, CURLOPT_ENCODING, "");
        curl_easy_setopt(curl, CURLOPT_SHARE, curlsh);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)req);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, check_header);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)req);
        curl_easy_setopt(curl, CURLOPT_PRIVATE, (void*)req);
        curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, ssl_ctx_function);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

        if(proxyEnabled && proxy.size())
        {
            curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
            curl_easy_setopt(curl, CURLOPT_PROXY, proxy.c_str());
            if(proxyUsername.size())
            {
                curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, proxyUsername.c_str());
                curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, proxyPassword.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1L);
        }

        curl_multi_add_handle(curlm, curl);

        req->status = REQ_INFLIGHT;

        req->httpiohandle = (void*)curl;
    }
    else
    {
        req->status = REQ_FAILURE;
    }
}

MegaProxySettings *MegaApiCurlHttpIO::getAutoProxySettings()
{
    MegaProxySettings *proxySettings = new MegaProxySettings();
    proxySettings->setProxyType(MegaProxySettings::NONE);
    return proxySettings;
}

}
