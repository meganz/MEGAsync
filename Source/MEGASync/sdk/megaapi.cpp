/*

MEGA SDK sample application for the gcc/POSIX environment, using cURL for HTTP I/O,
GNU Readline for console I/O and FreeImage for thumbnail creation

(c) 2013 by Mega Limited, Wellsford, New Zealand

Applications using the MEGA API must present a valid application key
and comply with the the rules set forth in the Terms of Service.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

 */

#define _POSIX_SOURCE
#define _LARGE_FILES
#define _LARGEFILE64_SOURCE
#define _GNU_SOURCE 1
#define _FILE_OFFSET_BITS 64

#define __DARWIN_C_LEVEL 199506L

#define USE_VARARGS
#define PREFER_STDARG
#include "mega.h"
#include "megaapi.h"

//#include <android/log.h>
//#define LOG(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "MEGA_JNI", __VA_ARGS__))

#ifdef _WIN32
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#ifdef __MACH__
#include <machine/endian.h>
#include <strings.h>
#include <sys/time.h>
#define CLOCK_MONOTONIC 0
int clock_gettime(int, struct timespec* t)
{
	struct timeval now;
	int rv = gettimeofday(&now,NULL);
	if (rv) return rv;
	t->tv_sec  = now.tv_sec;
	t->tv_nsec = now.tv_usec*1000;
	return 0;
}
ssize_t pread(int, void *, size_t, off_t) __DARWIN_ALIAS_C(pread);
ssize_t pwrite(int, const void *, size_t, off_t) __DARWIN_ALIAS_C(pwrite);
#endif

#ifndef _WIN32
int MegaCurlHttpIO::ssl_verify_callback(X509_STORE_CTX *ctx, void *arg)
{
	//verify the certificate chain.
	int ok = ((X509_verify_cert(ctx)==1) && ctx->cert);
	if(!ok) return 0;

	//get an EVP_PKEY object with the MEGA public key. 
	EVP_PKEY* evp = X509_PUBKEY_get(X509_get_X509_PUBKEY(ctx->cert));
	if(!evp) return 0;

	//modulus of the MEGA public key
	const unsigned char MEGAmodulus[] = 
	{
			0xB6, 0x61, 0xE7, 0xCF, 0x69, 0x2A, 0x84, 0x35, 0x05, 0xC3, 0x14, 0xBC, 0x95, 0xCF, 0x94, 0x33,
			0x1C, 0x82, 0x67, 0x3B, 0x04, 0x35, 0x11, 0xA0, 0x8D, 0xC8, 0x9D, 0xBB, 0x9C, 0x79, 0x65, 0xE7,
			0x10, 0xD9, 0x91, 0x80, 0xC7, 0x81, 0x0C, 0xF4, 0x95, 0xBB, 0xB3, 0x26, 0x9B, 0x97, 0xD2, 0x14,
			0x0F, 0x0B, 0xCA, 0xF0, 0x5E, 0x45, 0x7B, 0x32, 0xC6, 0xA4, 0x7D, 0x7A, 0xFE, 0x11, 0xE7, 0xB2,
			0x5E, 0x21, 0x55, 0x23, 0x22, 0x1A, 0xCA, 0x1A, 0xF9, 0x21, 0xE1, 0x4E, 0xB7, 0x82, 0x0D, 0xEB,
			0x9D, 0xCB, 0x4E, 0x3D, 0x0B, 0xE4, 0xED, 0x4A, 0xEF, 0xE4, 0xAB, 0x0C, 0xEC, 0x09, 0x69, 0xFE,
			0xAE, 0x43, 0xEC, 0x19, 0x04, 0x3D, 0x5B, 0x68, 0x0F, 0x67, 0xE8, 0x80, 0xFF, 0x9B, 0x03, 0xEA,
			0x50, 0xAB, 0x16, 0xD7, 0xE0, 0x4C, 0xB4, 0x42, 0xEF, 0x31, 0xE2, 0x32, 0x9F, 0xE4, 0xD5, 0xF4,
			0xD8, 0xFD, 0x82, 0xCC, 0xC4, 0x50, 0xD9, 0x4D, 0xB5, 0xFB, 0x6D, 0xA2, 0xF3, 0xAF, 0x37, 0x67,
			0x7F, 0x96, 0x4C, 0x54, 0x3D, 0x9B, 0x1C, 0xBD, 0x5C, 0x31, 0x6D, 0x10, 0x43, 0xD8, 0x22, 0x21,
			0x01, 0x87, 0x63, 0x22, 0x89, 0x17, 0xCA, 0x92, 0xCB, 0xCB, 0xEC, 0xE8, 0xC7, 0xFF, 0x58, 0xE8,
			0x18, 0xC4, 0xCE, 0x1B, 0xE5, 0x4F, 0x20, 0xA8, 0xCF, 0xD3, 0xB9, 0x9D, 0x5A, 0x7A, 0x69, 0xF2,
			0xCA, 0x48, 0xF8, 0x87, 0x95, 0x3A, 0x32, 0x70, 0xB3, 0x1A, 0xF0, 0xC4, 0x45, 0x70, 0x43, 0x58,
			0x18, 0xDA, 0x85, 0x29, 0x1D, 0xAF, 0x83, 0xC2, 0x35, 0xA9, 0xC1, 0x73, 0x76, 0xB4, 0x47, 0x22,
			0x2B, 0x42, 0x9F, 0x93, 0x72, 0x3F, 0x9D, 0x3D, 0xA1, 0x47, 0x3D, 0xB0, 0x46, 0x37, 0x1B, 0xFD,
			0x0E, 0x28, 0x68, 0xA0, 0xF6, 0x1D, 0x62, 0xB2, 0xDC, 0x69, 0xC7, 0x9B, 0x09, 0x1E, 0xB5, 0x47
	};

	//exponent of the MEGA public key
	const unsigned char MEGAexponent[] = {0x01, 0x00, 0x01};

	//check the length of the modulus
	int len = BN_num_bytes(evp->pkey.rsa->n);
	if(len != sizeof(MEGAmodulus))
	{
		EVP_PKEY_free(evp);
		return 0;
	}

	//convert the public modulus to a binary string	
	unsigned char *buff = new unsigned char[len];
	BN_bn2bin(evp->pkey.rsa->n, buff);

	//check the public key
	ok = (memcmp(buff, MEGAmodulus, (size_t)len) == 0);
	delete buff;

	//check the length of the exponent
	len = BN_num_bytes(evp->pkey.rsa->e);
	if(len != sizeof(MEGAexponent))
	{
		EVP_PKEY_free(evp);
		return 0;
	}

	//convert the public exponent to a binary string	
	buff = new unsigned char[len];
	BN_bn2bin(evp->pkey.rsa->e, buff);

	//check the public exponent
	ok &= (memcmp(buff, MEGAexponent, (size_t)len) == 0);
	delete buff;

	EVP_PKEY_free(evp);
	return ok;
}

CURLcode MegaCurlHttpIO::sslctx_function(CURL * curl, void * sslctx, void * parm)
{	
	X509_STORE * store;
	X509 * cert=NULL;
	BIO * bio;

	//CA in the root of the MEGA certificate.
	const char *MEGApemRoot =
			"UTN DATACorp SGC Root CA\n"
			"========================\n"
			"-----BEGIN CERTIFICATE-----\n"
			"MIIEXjCCA0agAwIBAgIQRL4Mi1AAIbQR0ypoBqmtaTANBgkqhkiG9w0BAQUFADCBkzELMAkGA1UE\n"
			"BhMCVVMxCzAJBgNVBAgTAlVUMRcwFQYDVQQHEw5TYWx0IExha2UgQ2l0eTEeMBwGA1UEChMVVGhl\n"
			"IFVTRVJUUlVTVCBOZXR3b3JrMSEwHwYDVQQLExhodHRwOi8vd3d3LnVzZXJ0cnVzdC5jb20xGzAZ\n"
			"BgNVBAMTElVUTiAtIERBVEFDb3JwIFNHQzAeFw05OTA2MjQxODU3MjFaFw0xOTA2MjQxOTA2MzBa\n"
			"MIGTMQswCQYDVQQGEwJVUzELMAkGA1UECBMCVVQxFzAVBgNVBAcTDlNhbHQgTGFrZSBDaXR5MR4w\n"
			"HAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxITAfBgNVBAsTGGh0dHA6Ly93d3cudXNlcnRy\n"
			"dXN0LmNvbTEbMBkGA1UEAxMSVVROIC0gREFUQUNvcnAgU0dDMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
			"AQ8AMIIBCgKCAQEA3+5YEKIrblXEjr8uRgnn4AgPLit6E5Qbvfa2gI5lBZMAHryv4g+OGQ0SR+ys\n"
			"raP6LnD43m77VkIVni5c7yPeIbkFdicZD0/Ww5y0vpQZY/KmEQrrU0icvvIpOxboGqBMpsn0GFlo\n"
			"wHDyUwDAXlCCpVZvNvlK4ESGoE1O1kduSUrLZ9emxAW5jh70/P/N5zbgnAVssjMiFdC04MwXwLLA\n"
			"9P4yPykqlXvY8qdOD1R8oQ2AswkDwf9c3V6aPryuvEeKaq5xyh+xKrhfQgUL7EYw0XILyulWbfXv\n"
			"33i+Ybqypa4ETLyorGkVl73v67SMvzX41MPRKA5cOp9wGDMgd8SirwIDAQABo4GrMIGoMAsGA1Ud\n"
			"DwQEAwIBxjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBRTMtGzz3/64PGgXYVOktKeRR20TzA9\n"
			"BgNVHR8ENjA0MDKgMKAuhixodHRwOi8vY3JsLnVzZXJ0cnVzdC5jb20vVVROLURBVEFDb3JwU0dD\n"
			"LmNybDAqBgNVHSUEIzAhBggrBgEFBQcDAQYKKwYBBAGCNwoDAwYJYIZIAYb4QgQBMA0GCSqGSIb3\n"
			"DQEBBQUAA4IBAQAnNZcAiosovcYzMB4p/OL31ZjUQLtgyr+rFywJNn9Q+kHcrpY6CiM+iVnJowft\n"
			"Gzet/Hy+UUla3joKVAgWRcKZsYfNjGjgaQPpxE6YsjuMFrMOoAyYUJuTqXAJyCyjj98C5OBxOvG0\n"
			"I3KgqgHf35g+FFCgMSa9KOlaMCZ1+XtgHI3zzVAmbQQnmt/VDUVHKWss5nbZqSl9Mt3JNjy9rjXx\n"
			"EZ4du5A/EkdOjtd+D2JzHVImOBwYSf0wdJrE5SIv2MCN7ZF6TACPcn9d2t0bi0Vr591pl6jFVkwP\n"
			"DPafepE39peC4N1xaf92P2BNPM/3mfnGV/TJVTl4uix5yaaIK/QI\n"
			"-----END CERTIFICATE-----\n";

	SSL_CTX *ctx = (SSL_CTX *)sslctx;

	/* get a BIO */
	bio=BIO_new_mem_buf((char *)MEGApemRoot, -1);

	/* use it to read the PEM formatted certificate from memory into an X509
	 * structure that SSL can use*/
	PEM_read_bio_X509(bio, &cert, 0, NULL);
	(void)BIO_set_close(bio, BIO_NOCLOSE);
	BIO_free(bio);

	if(cert == NULL) return CURLE_SSL_CACERT_BADFILE;

	/* get a pointer to the X509 certificate store (which may be empty!) */
	store=SSL_CTX_get_cert_store(ctx);
	if (store == NULL) { X509_free(cert); return CURLE_SSL_CACERT_BADFILE; }

	/* add the certificate to this store */
	X509_STORE_add_cert(store, cert);
	X509_free(cert);

	/* set max depth for the certificate chain */
	SSL_CTX_set_verify_depth(ctx, 3);

	/* set a custom callback to check the certificate */
	SSL_CTX_set_cert_verify_callback(ctx, MegaCurlHttpIO::ssl_verify_callback, NULL);

	/* all set to go */
	return CURLE_OK ;
}
#endif

// HttpIO implementation using libcurl
MegaCurlHttpIO::MegaCurlHttpIO()
{
#ifdef _WIN32
    InitializeCriticalSection(&csHTTP);
    EnterCriticalSection(&csHTTP);

    // create the session handle using the default settings.
    hSession = WinHttpOpen(L"MEGA Client Access Engine/1.0",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,WINHTTP_FLAG_ASYNC);

    hWakeup[0] = CreateEvent(NULL,FALSE,FALSE,NULL);
    hWakeup[1] = CreateEvent(NULL,FALSE,FALSE,NULL);

    completion = 0;
#else
	//Copy of CurlHttpIO constructor to avoid the inclusion of megaapp.h
	//and be able to compile without readline
	curl_global_init(CURL_GLOBAL_DEFAULT);

	curlm = curl_multi_init();

	curlsh = curl_share_init();
	curl_share_setopt(curlsh,CURLSHOPT_SHARE,CURL_LOCK_DATA_DNS);
	curl_share_setopt(curlsh,CURLSHOPT_SHARE,CURL_LOCK_DATA_SSL_SESSION);

	contenttypejson = curl_slist_append(NULL,"Content-Type: application/json");
	contenttypejson = curl_slist_append(contenttypejson, "Expect:");

	contenttypebinary = curl_slist_append(NULL,"Content-Type: application/octet-stream");
	contenttypebinary = curl_slist_append(contenttypebinary, "Expect:");
	////////////////////////////////////////

	//Pipe to be able to leave select() call
	if (pipe(m_pipe) < 0)
		cout << "Error creating pipe" << endl;

	if (fcntl(m_pipe[0], F_SETFL, O_NONBLOCK) < 0)
		cout << "fcntl error" << endl;
#endif

    debug = 0;
}

MegaCurlHttpIO::~MegaCurlHttpIO()
{
#ifdef _WIN32
    WinHttpCloseHandle(hSession);
    LeaveCriticalSection(&csHTTP);
#else
	curl_global_cleanup();
#endif
}

// update monotonously increasing timestamp in deciseconds
void MegaCurlHttpIO::updatedstime()
{
#ifdef _WIN32
    ds = (dstime)(GetTickCount()/100);	// FIXME: Use GetTickCount64() instead
#else
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC,&ts);
	ds = ts.tv_sec*10+ts.tv_nsec/100000000;
#endif
}

#ifdef _WIN32
// handle WinHTTP callbacks (which can be in a worker thread context)
void CALLBACK MegaCurlHttpIO::AsyncCallback(HINTERNET hInternet, DWORD_PTR dwContext,  DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
    HttpReq* req = (HttpReq*)dwContext;

    EnterCriticalSection(&((MegaCurlHttpIO*)req->httpio)->csHTTP);

    switch (dwInternetStatus)
    {
        case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
            {
                DWORD size =  *(DWORD*)lpvStatusInformation;

                if (!size)
                {
                    if (((MegaCurlHttpIO*)req->httpio)->debug)
                    {
                        if (req->binary) cout << "[received " << req->bufpos << " bytes of raw data]" << endl;
                        else cout << "Received: " << req->in.c_str() << endl;
                    }

                    req->status = req->httpstatus == 200 ? REQ_SUCCESS : REQ_FAILURE;

                    SetEvent(((MegaCurlHttpIO*)req->httpio)->hWakeup[0]);
                }
                else
                {
                    char* ptr = (char*)req->reserveput((unsigned*)&size);

                    if (!WinHttpReadData(hInternet,ptr,size,NULL)) req->httpio->cancel(req);
                }
            }
            SetEvent(((MegaCurlHttpIO*)req->httpio)->hWakeup[0]);
            break;

        case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
            if (dwStatusInformationLength)
            {
                req->completeput(dwStatusInformationLength);

                if (!WinHttpQueryDataAvailable(((WinHttpContext*)req->httpiohandle)->hRequest,NULL))
                {
                    req->httpio->cancel(req);
                    SetEvent(((MegaCurlHttpIO*)req->httpio)->hWakeup[0]);
                }
            }
            break;

        case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
            {
                DWORD statusCode = 0;
                DWORD statusCodeSize = sizeof(statusCode);

                if (!WinHttpQueryHeaders(((WinHttpContext*)req->httpiohandle)->hRequest,WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&statusCode,&statusCodeSize,WINHTTP_NO_HEADER_INDEX))
                {
                    req->httpio->cancel(req);
                    SetEvent(((MegaCurlHttpIO*)req->httpio)->hWakeup[0]);
                }
                else
                {
                    req->httpstatus = statusCode;

                    if (!WinHttpQueryDataAvailable(((WinHttpContext*)req->httpiohandle)->hRequest,NULL))
                    {
                        req->httpio->cancel(req);
                        SetEvent(((MegaCurlHttpIO*)req->httpio)->hWakeup[0]);
                    }
                }
            }
            break;

        case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
        case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
            req->httpio->cancel(req);
            SetEvent(((MegaCurlHttpIO*)req->httpio)->hWakeup[0]);
            break;

        case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
            if ((WinHttpContext*)req->httpiohandle && WinHttpReceiveResponse(((WinHttpContext*)req->httpiohandle)->hRequest,NULL) == FALSE)
            {
                req->httpio->cancel(req);
                SetEvent(((MegaCurlHttpIO*)req->httpio)->hWakeup[0]);
            }
    }

    LeaveCriticalSection(&((MegaCurlHttpIO*)req->httpio)->csHTTP);
}
#endif

// POST request to URL
void MegaCurlHttpIO::post(HttpReq* req, const char* data, unsigned len)
{
#ifdef _WIN32
    if (debug)
    {
        cout << "POST target URL: " << req->posturl << endl;

        if (req->binary) cout << "[sending " << req->out->size() << " bytes of raw data]" << endl;
        else cout << "Sending: " << *req->out << endl;
    }

    WinHttpContext* cpContext = new WinHttpContext;
    WCHAR szHost[256];
    URL_COMPONENTS urlComp;

    req->httpiohandle = (void*)cpContext;

    ZeroMemory(cpContext,sizeof(WinHttpContext));
    ZeroMemory(&urlComp,sizeof urlComp);
    urlComp.dwStructSize = sizeof urlComp;

    urlComp.lpszHostName = szHost;
    urlComp.dwHostNameLength = sizeof szHost/sizeof *szHost;
    urlComp.dwUrlPathLength = (DWORD)-1;
    urlComp.dwSchemeLength = (DWORD)-1;

    // Crack the URL.
    WCHAR szURL[1024];

    if (MultiByteToWideChar(CP_UTF8,0,req->posturl.c_str(),-1,szURL,sizeof szURL/sizeof *szURL) && WinHttpCrackUrl(szURL,0,0,&urlComp))
    {
        if ((cpContext->hConnect = WinHttpConnect(hSession,szHost,urlComp.nPort,0)))
        {
            cpContext->hRequest = WinHttpOpenRequest(cpContext->hConnect,L"POST", urlComp.lpszUrlPath,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,(urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0);

            if (cpContext->hRequest)
            {
                WinHttpSetStatusCallback(cpContext->hRequest,AsyncCallback,WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE | WINHTTP_CALLBACK_FLAG_READ_COMPLETE | WINHTTP_CALLBACK_FLAG_HEADERS_AVAILABLE | WINHTTP_CALLBACK_FLAG_REQUEST_ERROR | WINHTTP_CALLBACK_FLAG_SECURE_FAILURE | WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE,0);

                LPCWSTR pwszHeaders = REQ_JSON ? L"Content-Type: application/json" : L"Content-Type: application/octet-stream";

                if (WinHttpSendRequest(cpContext->hRequest,pwszHeaders, wcslen(pwszHeaders),(LPVOID)(data ? data : req->out->data()), data ? len : req->out->size(), data ? len : req->out->size(),(DWORD_PTR)req))
                {
                   req->status = REQ_INFLIGHT;
                   return;
                }
                else if (debug) cout << "WinHTTPSendRequest() failed" << endl;
            }
            else if (debug) cout << "WinHttpOpenRequest() failed" << endl;
        }
        else if (debug) cout << "WinHttpConnect() failed" << endl;
    }
    else if (debug) cout << "WinHttpCrackUrl(" << req->posturl << ") failed" << endl;

    req->status = REQ_FAILURE;
#else
	CURL* curl;

	req->in.clear();

	if ((curl = curl_easy_init()))
	{
		if (debug)
		{
			cout << "POST target URL: " << req->posturl << endl;

			if (req->binary) cout << "[sent " << req->out->size() << " bytes of raw data]" << endl;
			else cout << "Sent: " << *req->out << endl;
		}

		curl_easy_setopt(curl,CURLOPT_URL,req->posturl.c_str());
		curl_easy_setopt(curl,CURLOPT_POSTFIELDS,data ? data : req->out->c_str());
		curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,data ? len : req->out->size());
		curl_easy_setopt(curl,CURLOPT_USERAGENT,"MEGA Client Access Engine/1.0");
		curl_easy_setopt(curl,CURLOPT_HTTPHEADER,req->type == REQ_JSON ? contenttypejson : contenttypebinary);
		curl_easy_setopt(curl,CURLOPT_SHARE,curlsh);
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void*)req);
		curl_easy_setopt(curl,CURLOPT_PRIVATE,(void*)req);

		//Verify SSL cert and host.
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

		//Don't trust the default cacert bundle
		curl_easy_setopt(curl,CURLOPT_CAINFO, NULL);
		curl_easy_setopt(curl,CURLOPT_CAPATH, NULL);

		//Callback to load the the root certificate of the MEGA server.
		curl_easy_setopt(curl,CURLOPT_SSL_CTX_FUNCTION, MegaCurlHttpIO::sslctx_function);

		curl_multi_add_handle(curlm,curl);

		req->status = REQ_INFLIGHT;

		req->httpiohandle = (void*)curl;
	}
	else req->status = REQ_FAILURE;
#endif
}

// cancel request if pending
void MegaCurlHttpIO::cancel(HttpReq* req)
{
#ifdef _WIN32
    if (req->httpiohandle)
    {
        WinHttpContext* cpContext = (WinHttpContext*)req->httpiohandle;

        if (cpContext->hRequest)
        {
            WinHttpSetStatusCallback(cpContext->hRequest,
                    NULL,
                    0,
                    0);

            WinHttpCloseHandle(cpContext->hRequest);
        }

        if (cpContext->hConnect) WinHttpCloseHandle(cpContext->hConnect);

        req->httpstatus = 0;
        req->status = REQ_FAILURE;

        delete (WinHttpContext*)req->httpiohandle;
        req->httpiohandle = NULL;
    }
#else
	if (req->httpiohandle)
	{
		curl_multi_remove_handle(curlm,(CURL*)req->httpiohandle);
		curl_easy_cleanup((CURL*)req->httpiohandle);

		req->httpstatus = 0;
		req->status = REQ_FAILURE;
		req->httpiohandle = NULL;
    }
#endif
}

// real-time progress information on POST data
m_off_t MegaCurlHttpIO::postpos(void* handle)
{
#ifdef _WIN32
    return 0;
#else
	double bytes;
	curl_easy_getinfo(handle,CURLINFO_SIZE_UPLOAD,&bytes);
	return (m_off_t)bytes;
#endif
}

// process events
int MegaCurlHttpIO::doio()
{
#ifdef _WIN32
    int done;
    done = completion;
    completion = 0;
    return done;
#else
	CURLMsg *msg;
	int dummy;
	int done = 0;

	curl_multi_perform(curlm,&dummy);

	while ((msg = curl_multi_info_read(curlm,&dummy)))
	{
		HttpReq* req;

		if (curl_easy_getinfo(msg->easy_handle,CURLINFO_PRIVATE,(char**)&req) == CURLE_OK && req)
		{
			req->httpio = NULL;

			if (msg->msg == CURLMSG_DONE)
			{
				curl_easy_getinfo(msg->easy_handle,CURLINFO_RESPONSE_CODE,&req->httpstatus);
				if (debug)
				{
					cout << "CURLMSG_DONE with HTTP status: " << req->httpstatus << endl;
					if (req->binary) cout << "[received " << req->in.size() << " bytes of raw data]" << endl;
					else cout << "Received: " << req->in.c_str() << endl;
				}

				req->status = req->httpstatus == 200 ? REQ_SUCCESS : REQ_FAILURE;
				done = 1;
			}
			else req->status = REQ_FAILURE;
		}

		curl_multi_remove_handle(curlm,msg->easy_handle);
		curl_easy_cleanup(msg->easy_handle);
	}

	return done;
#endif
}

//Notify curl that has to wait for a new event
void MegaCurlHttpIO::notify()
{
#ifdef _WIN32
    SetEvent(hWakeup[1]);
#else
	//Force exit from select()
	write(m_pipe[1], "0", 1);
#endif
}

void MegaCurlHttpIO::waitio(dstime maxds)
{
#ifdef _WIN32
    LeaveCriticalSection(&csHTTP);
    WaitForMultipleObjects(sizeof hWakeup/sizeof *hWakeup,hWakeup,FALSE,maxds*100);
    EnterCriticalSection(&csHTTP);
#else
	int maxfd;
	fd_set rfds, wfds, efds;
	timeval tv;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	curl_multi_fdset(curlm,&rfds,&wfds,&efds,&maxfd);

	//Pipe added to rfds to be able to leave select() when needed
	FD_SET(m_pipe[0], &rfds);
	if (maxfd < m_pipe[0]) maxfd = m_pipe[0];

	long curltimeout;
	curl_multi_timeout(curlm, &curltimeout);
	if(curltimeout!=-1) maxds = (maxds<(unsigned long)(curltimeout/100)) ? maxds : (curltimeout/100) + 1;

	if (maxds+1)
	{
		dstime us = 1000000/10*maxds;

		tv.tv_sec = us/1000000;
		tv.tv_usec = us-tv.tv_sec*1000000;
	}

	select(maxfd+1,&rfds,&wfds,&efds,maxds+1 ? &tv : NULL);

	//Empty pipe
	uint8_t buf;
	while (read(m_pipe[0], &buf, 1) == 1);
#endif
}

#ifndef _WIN32
// callback for incoming HTTP payload
size_t MegaCurlHttpIO::write_data(void *ptr, size_t size, size_t nmemb, void *target)
{
	size *= nmemb;

	((HttpReq*)target)->put(ptr,size);

	return size;
}
#endif

void MegaCurlHttpIO::setDebug(bool debug)
{
	this->debug = debug;
}

bool MegaCurlHttpIO::getDebug()
{
	return this->debug;
}

CFileAccess::CFileAccess()
{
	fd = NULL;
}

CFileAccess::~CFileAccess()
{
	if (fd != NULL) fclose(fd);
}

int CFileAccess::fread(string* dst, unsigned len, unsigned pad, m_off_t pos)
{	
	dst->resize(len+pad);
	fseek(fd, pos, SEEK_SET);
	if (::fread((char*)dst->data(), 1, len, fd) == len)
	{
		memset((char*)dst->data()+len,0,pad);
		return 1;
	}

	return 0;
}

int CFileAccess::fwrite(const byte* data, unsigned len, m_off_t pos)
{
	fseek(fd, pos, SEEK_SET);
	if(::fwrite(data, 1, len, fd) == len) return 1;

	return 0;
}

int CFileAccess::fopen(const char* f, int read, int write)
{
	const char *mode = write ? (read ? "r+b" : "wb") : "rb";
	fd = ::fopen(f, mode);
	if(fd != NULL)
	{
		fseek(fd, 0L, SEEK_END);
		size = ftell(fd);
		return 1;
	}

	return 0;
}

MegaRequest::MegaRequest(int type, MegaRequestListener *listener)
{ 
	this->type = type;
	this->transfer = NULL;
	this->listener = listener;
	this->nodeHandle = UNDEF;
	this->link = NULL;
	this->parentHandle = UNDEF;
	this->userHandle = NULL;
	this->name = NULL;
	this->email = NULL;
	this->password = NULL;
	this->newPassword = NULL;
	this->privateKey = NULL;
	this->access = NULL;
	this->numRetry = 0;
	this->nextRetryDelay = 0;
	this->publicNode = NULL;
	this->numDetails = 0;
	this->file = NULL;
	this->attrType = 0;

	if(type == MegaRequest::TYPE_ACCOUNT_DETAILS) this->accountDetails = new AccountDetails();
	else this->accountDetails = NULL;
}

MegaRequest::MegaRequest(MegaTransfer *transfer)
{
	this->type = MegaRequest::TYPE_UPLOAD;
	this->transfer = transfer;
	this->listener = NULL;
	this->nodeHandle = UNDEF;
	this->link = NULL;
	this->parentHandle = UNDEF;
	this->userHandle = NULL;
	this->name = NULL;
	this->email = NULL;
	this->password = NULL;
	this->newPassword = NULL;
	this->privateKey = NULL;
	this->access = NULL;
	this->numRetry = 0;
	this->nextRetryDelay = 0;
	this->accountDetails = NULL;
	this->publicNode = NULL;
	this->numDetails = 0;
	this->file = NULL;
	this->attrType = 0;
}

MegaRequest::MegaRequest(const MegaRequest &request)
{
	this->link = NULL;
	this->name = NULL;
	this->email = NULL;
	this->password = NULL;
	this->newPassword = NULL;
	this->privateKey = NULL;
	this->access = NULL;
	this->userHandle = NULL;
	this->file = NULL;

	this->type = request.getType();
	this->setNodeHandle(request.getNodeHandle());
	this->setLink(request.getLink());
	this->setParentHandle(request.getParentHandle());
	this->setUserHandle(request.getUserHandle());
	this->setName(request.getName());
	this->setEmail(request.getEmail());
	this->setPassword(request.getPassword());
	this->setNewPassword(request.getNewPassword());
	this->setPrivateKey(request.getPrivateKey());
	this->setAccess(request.getAccess());
	this->setNumRetry(request.getNumRetry());
	this->setNextRetryDelay(request.getNextRetryDelay());
	this->numDetails = 0;
	this->setFile(request.getFile());
	this->setAttrType(request.getAttrType());

	//These fields can't be copied ATM to prevent memory leaks.
	this->transfer = NULL; //request.getTransfer();
	this->listener = NULL; //request.getListener();
	this->accountDetails = NULL; //request.getAccountDetails();
	this->publicNode = NULL;
}


MegaRequest::~MegaRequest()
{
	if(link) delete [] link;
	if(name) delete [] name;
	if(email) delete [] email;
	if(password) delete [] password;
	if(newPassword) delete [] newPassword;
	if(privateKey) delete [] privateKey;
	if(access) delete [] access;
	if(accountDetails) delete accountDetails;
	if(userHandle) delete [] userHandle;
	if(publicNode) delete publicNode;
	if(file) delete [] file;
}

MegaRequest *MegaRequest::copy()
{
	return new MegaRequest(*this);
}	

int MegaRequest::getType() const { return type; }
uint64_t MegaRequest::getNodeHandle() const { return nodeHandle; }
const char* MegaRequest::getLink() const { return link; }
uint64_t MegaRequest::getParentHandle() const { return parentHandle; }
const char* MegaRequest::getUserHandle() const { return userHandle; }
const char* MegaRequest::getName() const { return name; }
const char* MegaRequest::getEmail() const { return email; }
const char* MegaRequest::getPassword() const { return password; }
const char* MegaRequest::getNewPassword() const { return newPassword; }
const char* MegaRequest::getPrivateKey() const { return privateKey; }
const char* MegaRequest::getAccess() const { return access; }
const char* MegaRequest::getFile() const { return file; }
int MegaRequest::getAttrType() const { return attrType; }
int MegaRequest::getNumRetry() const { return numRetry; }
int MegaRequest::getNextRetryDelay() const { return nextRetryDelay; }
AccountDetails* MegaRequest::getAccountDetails() const { return accountDetails; }
int MegaRequest::getNumDetails() const { return numDetails; }
void MegaRequest::setNumDetails(int numDetails) { this->numDetails = numDetails; }

Node* MegaRequest::getPublicNode() 
{ 
	Node* temp = publicNode;
	publicNode = NULL;
	return temp;
}

void MegaRequest::setPublicNode(Node *node) { this->publicNode = node; }
void MegaRequest::setNodeHandle(handle nodeHandle) { this->nodeHandle = nodeHandle; }
void MegaRequest::setParentHandle(handle parentHandle) { this->parentHandle = parentHandle; }
void MegaRequest::setUserHandle(const char* userHandle) 
{ 
	if(this->userHandle) delete [] this->userHandle;
	this->userHandle = MegaApi::strdup(userHandle);
}

void MegaRequest::setNumRetry(int numRetry) { this->numRetry = numRetry; }
void MegaRequest::setNextRetryDelay(int nextRetryDelay) { this->nextRetryDelay = nextRetryDelay; }

void MegaRequest::setLink(const char* link) 
{
	if(this->link) delete [] this->link;
	this->link = MegaApi::strdup(link);
}
void MegaRequest::setName(const char* name) 
{ 
	if(this->name) delete [] this->name;
	this->name = MegaApi::strdup(name);
}
void MegaRequest::setEmail(const char* email) 
{
	if(this->email) delete [] this->email;
	this->email = MegaApi::strdup(email);
}
void MegaRequest::setPassword(const char* password) 
{ 
	if(this->password) delete [] this->password;
	this->password = MegaApi::strdup(password);
}
void MegaRequest::setNewPassword(const char* newPassword) 
{ 
	if(this->newPassword) delete [] this->newPassword;
	this->newPassword = MegaApi::strdup(newPassword);
}
void MegaRequest::setPrivateKey(const char* privateKey) 
{ 
	if(this->privateKey) delete [] this->privateKey;
	this->privateKey = MegaApi::strdup(privateKey);
}
void MegaRequest::setAccess(const char* access) 
{ 
	if(this->access) delete [] this->access;
	this->access = MegaApi::strdup(access);
}

void MegaRequest::setFile(const char* file) 
{ 
	if(this->file) delete [] this->file;
	this->file = MegaApi::strdup(file);
}

void MegaRequest::setAttrType(int type)
{
	this->attrType = type;
}

const char *MegaRequest::getRequestString() const
{
	switch(type)
	{
		case TYPE_LOGIN: return "login";
		case TYPE_MKDIR: return "mkdir";
		case TYPE_MOVE: return "move";	
		case TYPE_COPY: return "copy";		
		case TYPE_RENAME: return "rename";	
		case TYPE_REMOVE: return "remove";
		case TYPE_SHARE: return "share";
		case TYPE_FOLDER_ACCESS: return "folderaccess";						
		case TYPE_IMPORT_LINK: return "importlink";
		case TYPE_IMPORT_NODE: return "importnode";
		case TYPE_EXPORT: return "export";
		case TYPE_FETCH_NODES: return "fetchnodes";
		case TYPE_ACCOUNT_DETAILS: return "accountdetails";
		case TYPE_CHANGE_PW: return "changepw";
		case TYPE_UPLOAD: return "upload";
		case TYPE_LOGOUT: return "logout";
		case TYPE_FAST_LOGIN: return "fastlogin";
		case TYPE_GET_PUBLIC_NODE: return "getpublicnode";
		case TYPE_GET_ATTR_FILE: return "getattrfile";
        case TYPE_SET_ATTR_FILE: return "setattrfile";
        case TYPE_CREATE_ACCOUNT: return "createaccount";
	}
	return "unknown";
}

MegaRequestListener *MegaRequest::getListener() const { return listener; }
MegaTransfer *MegaRequest::getTransfer() const { return transfer; }

const char *MegaRequest::toString() const { return getRequestString(); }
const char *MegaRequest::__str__() const { return getRequestString(); }

MegaTransfer::MegaTransfer(int type, MegaTransferListener *listener)
{ 
	this->type = type; 
	this->slot = -1;
	this->tag = -1;
	this->path = NULL;
	this->nodeHandle = UNDEF;
	this->parentHandle = UNDEF;
	this->startPos = 0;
	this->endPos = 0;
	this->numConnections = 1;
	this->maxSpeed = 1;
	this->parentPath = NULL;
	this->listener = listener;
	this->retry = 0;
	this->maxRetries = 3;
	this->time = 0;
	this->startTime = 0;
	this->transferredBytes = 0;
	this->totalBytes = 0;
	this->fileName = NULL;
	this->base64Key = NULL;
}

int MegaTransfer::getSlot() const { return slot; }
int MegaTransfer::getTag() const { return tag; }
int MegaTransfer::getType() const { return type; }
long long MegaTransfer::getStartTime() const { return startTime; }
long long MegaTransfer::getTransferredBytes() const {return transferredBytes; }
long long MegaTransfer::getTotalBytes() const { return totalBytes; }
const char* MegaTransfer::getPath() const { return path; }
const char* MegaTransfer::getParentPath() const { return parentPath; }
handle MegaTransfer::getNodeHandle() const { return nodeHandle; }
handle MegaTransfer::getParentHandle() const { return parentHandle; }
int MegaTransfer::getNumConnections() const { return numConnections; }
long long MegaTransfer::getStartPos() const { return startPos; }
long long MegaTransfer::getEndPos() const { return endPos; }
int MegaTransfer::getMaxSpeed() const { return maxSpeed; }
int MegaTransfer::getNumRetry() const { return retry; }
int MegaTransfer::getMaxRetries() const { return maxRetries; }
long long MegaTransfer::getTime() const { return time; }
const char* MegaTransfer::getFileName() const { return fileName; }
const char* MegaTransfer::getBase64Key() const { return base64Key; }

void MegaTransfer::setSlot(int slot) { this->slot = slot; }
void MegaTransfer::setTag(int tag) { this->tag = tag; }
void MegaTransfer::setStartTime(long long startTime) { this->startTime = startTime; }
void MegaTransfer::setTransferredBytes(long long transferredBytes) { this->transferredBytes = transferredBytes; }
void MegaTransfer::setTotalBytes(long long totalBytes) { this->totalBytes = totalBytes; }
void MegaTransfer::setPath(const char* path) 
{ 
	if(this->path) delete [] this->path;
	this->path = MegaApi::strdup(path);
	if(!this->path) return;

	for(int i = strlen(path)-1; i>=0; i--)
	{
		if((path[i]=='\\') || (path[i]=='/'))
		{
			setFileName(&(path[i+1]));
		}
	}
	setFileName(path);
}
void MegaTransfer::setParentPath(const char* path) 
{ 
	if(this->parentPath) delete [] this->parentPath;
	this->parentPath =  MegaApi::strdup(path);
}

void MegaTransfer::setFileName(const char* fileName)
{
	if(this->fileName) delete [] this->fileName;
	this->fileName =  MegaApi::strdup(fileName);
}

void MegaTransfer::setBase64Key(const char* base64Key)
{
	if(this->base64Key) delete [] this->base64Key;
	this->base64Key =  MegaApi::strdup(base64Key);
}

void MegaTransfer::setNodeHandle(handle nodeHandle) { this->nodeHandle = nodeHandle; }
void MegaTransfer::setParentHandle(handle parentHandle) { this->parentHandle = parentHandle; }
void MegaTransfer::setNumConnections(int connections) { this->numConnections = connections; }
void MegaTransfer::setStartPos(long long startPos) { this->startPos = startPos; }
void MegaTransfer::setEndPos(long long endPos) { this->endPos = endPos; }
void MegaTransfer::setMaxSpeed(int maxSpeed) {this->maxSpeed = maxSpeed; }
void MegaTransfer::setNumRetry(int retry) {this->retry = retry; }
void MegaTransfer::setMaxRetries(int maxRetries) {this->maxRetries = maxRetries; }
void MegaTransfer::setTime(long long time) { this->time = time; }

const char * MegaTransfer::getTransferString() const
{

	switch(type)
	{
	case TYPE_UPLOAD:
		return "upload";
	case TYPE_DOWNLOAD:
		return "download";
	}

	return "unknown";
}

MegaTransferListener* MegaTransfer::getListener() const { return listener; }

MegaTransfer::~MegaTransfer()
{
	if(path) delete[] path;
	if(parentPath) delete[] parentPath;
	if(fileName) delete [] fileName;
}

const char * MegaTransfer::toString() const { return getTransferString(); };
const char * MegaTransfer::__str__() const { return getTransferString(); };

MegaError::MegaError(int errorCode) 
{ 
	this->errorCode = errorCode;
	this->nextAttempt = 0;
}

MegaError::MegaError(const MegaError &megaError)
{
	errorCode = megaError.getErrorCode();
	nextAttempt = megaError.getNextAttempt();
}

MegaError* MegaError::copy()
{
	return new MegaError(*this);
}

int MegaError::getErrorCode() const { return errorCode; }
const char* MegaError::getErrorString() const
{
	return MegaError::getErrorString(errorCode);
}

const char* MegaError::getErrorString(int errorCode)
{ 
	if(errorCode <= 0)
	{
		switch(errorCode)
		{
		case API_OK:
			return "No error";
		case API_EINTERNAL:
			return "Internal error";
		case API_EARGS:
			return "Invalid argument";
		case API_EAGAIN:
			return "Request failed, retrying";
		case API_ERATELIMIT:
			return "Rate limit exceeded";
		case API_EFAILED:
			return "Failed permanently";
		case API_ETOOMANY:
			return "Too many concurrent connections or transfers";
		case API_ERANGE:
			return "Out of range";
		case API_EEXPIRED:
			return "Expired";
		case API_ENOENT:
			return "Not found";
		case API_ECIRCULAR:
			return "Circular linkage detected";
		case API_EACCESS:
			return "Access denied";
		case API_EEXIST:
			return "Already exists";
		case API_EINCOMPLETE:
			return "Incomplete";
		case API_EKEY:
			return "Invalid key/Decryption error";
		case API_ESID:
			return "Bad session ID";
		case API_EBLOCKED:
			return "Blocked";
		case API_EOVERQUOTA:
			return "Over quota";
		case API_ETEMPUNAVAIL:
			return "Temporarily not available";
		case API_ETOOMANYCONNECTIONS:
			return "Connection overflow";
		case API_EWRITE:
			return "Write error";
		case API_EREAD:
			return "Read error";
		case API_EAPPKEY:
			return "Invalid application key";
		default:
			return "Unknown error";
		}
	}
	return "HTTP Error"; 
};
const char* MegaError::toString() const { return getErrorString(); } 
const char* MegaError::__str__() const { return getErrorString(); } 


bool MegaError::isTemporal() const { return (nextAttempt==0); }
long MegaError::getNextAttempt() const { return nextAttempt; }
void MegaError::setNextAttempt(long nextAttempt) { this->nextAttempt = nextAttempt; }

//Request callbacks
void MegaRequestListener::onRequestStart(MegaApi* api, MegaRequest *request)
{ cout << "onRequestStartA " << "   Type: " << request->getRequestString() << endl; }
void MegaRequestListener::onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e)
{ cout << "onRequestFinishA " << "   Type: " << request->getRequestString() << "   Error: " << e->getErrorString() << endl; }
void MegaRequestListener::onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e)
{ cout << "onRequestTemporaryError " << "   Type: " << request->getRequestString() << "   Error: " << e->getErrorString() << endl; }
MegaRequestListener::~MegaRequestListener() {}

//Transfer callbacks
void MegaTransferListener::onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e)
{ cout << "onTransferFinish.   Node:  " << transfer->getFileName() << "    Error: " << e->getErrorString() << endl; }
void MegaTransferListener::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{ cout << "onTransferUpdate.   Node:  " << transfer->getFileName() << "    Progress: " << transfer->getTransferredBytes() << endl; }
void MegaTransferListener::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e)
{ cout << "onTransferTemporaryError.   Node:  " << transfer->getFileName() << "    Error: " << e->getErrorString() << endl; }	
MegaTransferListener::~MegaTransferListener() {}

//Global callbacks
void MegaGlobalListener::onUsersUpdate(MegaApi* api, UserList *users)
{ cout << "onUsersUpdate   Users: " << users->size() << endl; }
void MegaGlobalListener::onNodesUpdate(MegaApi* api, NodeList *nodes)
{ cout << "onNodesUpdate   Nodes: " << nodes->size() << endl; }
void MegaGlobalListener::onReloadNeeded(MegaApi* api)
{ cout << "onReloadNeeded" << endl; }
MegaGlobalListener::~MegaGlobalListener() {}

//All callbacks
void MegaListener::onRequestStart(MegaApi* api, MegaRequest *request)
{ cout << "onRequestStartA " << "   Type: " << request->getRequestString() << endl; }
void MegaListener::onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e)
{ cout << "onRequestFinishB " << "   Type: " << request->getRequestString() << "   Error: " << e->getErrorString() << endl; }
void MegaListener::onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e)
{ cout << "onRequestTemporaryError " << "   Type: " << request->getRequestString() << "   Error: " << e->getErrorString() << endl; }
void MegaListener::onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e)
{ cout << "onTransferFinish.   Node:  " << transfer->getFileName() << "    Error: " << e->getErrorString() << endl; }
void MegaListener::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{ cout << "onTransferUpdate.   Name:  " << transfer->getFileName() << "    Progress: " << transfer->getTransferredBytes() << endl; }
void MegaListener::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e)
{ cout << "onTransferTemporaryError.   Name: " << transfer->getFileName() << "    Error: " << e->getErrorString() << endl; }	
void MegaListener::onUsersUpdate(MegaApi* api, UserList *users)
{ cout << "onUsersUpdate   Users: " << users->size() << endl; }
void MegaListener::onNodesUpdate(MegaApi* api, NodeList *nodes)
{ cout << "onNodesUpdate   Nodes: " << nodes->size() << endl; }
void MegaListener::onReloadNeeded(MegaApi* api)
{ cout << "onReloadNeeded" << endl; }
MegaListener::~MegaListener() {}

int TreeProcessor::processNode(Node* node){ return 0; /* Stops the processing */ }
TreeProcessor::~TreeProcessor() {}

//Entry point for the blocking thread
void *MegaApi::threadEntryPoint(void *param)
{
	MegaApi *api = (MegaApi *)param;
	api->loop();
	return 0;
}

MegaApi::MegaApi()
{
    pthread_mutex_init(&listenerMutex, NULL);
	pthread_mutex_init(&transferListenerMutex, NULL);
	pthread_mutex_init(&requestListenerMutex, NULL);
	pthread_mutex_init(&globalListenerMutex, NULL);

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&fileSystemMutex, &attr);

	curl = new MegaCurlHttpIO();
	client = new MegaClient(this, curl, NULL, "FhMgXbqb");
	maxRetries = 3;
	loginRequest = NULL;
	updatingSID = 0;
	updateSIDtime = -10000000;

	//Start blocking thread
	threadExit = 0;
	pthread_create(&thread, NULL, threadEntryPoint, this);
}

MegaApi::MegaApi(MegaListener *listener)
{
	pthread_mutex_init(&listenerMutex, NULL);
	pthread_mutex_init(&transferListenerMutex, NULL);
	pthread_mutex_init(&requestListenerMutex, NULL);
	pthread_mutex_init(&globalListenerMutex, NULL);

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&fileSystemMutex, &attr);

	addListener(listener);
	curl = new MegaCurlHttpIO();
	client = new MegaClient(this, curl, NULL, "FhMgXbqb");
	maxRetries = 3;
	loginRequest = NULL;
	updatingSID = 0;
	updateSIDtime = -10000000;

	//Start blocking thread
	threadExit = 0;
	pthread_create(&thread, NULL, threadEntryPoint, this);
}

MegaApi::~MegaApi()
{
	threadExit = 1;
	curl->notify();
	pthread_join(thread, NULL);
	delete curl;
	delete client;
	if(loginRequest) delete loginRequest;
}

bool MegaApi::isLoggedIn()
{
	pthread_mutex_lock(&fileSystemMutex);
	bool result = client->loggedin();
	pthread_mutex_unlock(&fileSystemMutex);
	return result;
}

const char* MegaApi::getMyEmail()
{
	User* u;
	pthread_mutex_lock(&fileSystemMutex);
	if (!client->loggedin() || !(u = client->finduser(client->me))) return NULL;
	const char *result = u->email.c_str();
	//TODO: Copy string?
	pthread_mutex_unlock(&fileSystemMutex);
	return result;
}

const char* MegaApi::getBase64PwKey(const char *password)
{
	if(!password) return NULL;

	byte pwkey[SymmCipher::KEYLENGTH];
	error e = client->pw_key(password,pwkey);
	if(e) return NULL;

	char* buf = new char[SymmCipher::KEYLENGTH*4/3+4];
	Base64::btoa((byte *)pwkey, SymmCipher::KEYLENGTH, buf);
	return buf;
}

const char* MegaApi::getStringHash(const char* base64pwkey, const char* inBuf)
{
	if(!base64pwkey || !inBuf) return NULL;

	char pwkey[SymmCipher::KEYLENGTH];
	Base64::atob(base64pwkey, (byte *)pwkey, sizeof pwkey);

	SymmCipher key;
	key.setkey((byte*)pwkey);

	byte strhash[SymmCipher::KEYLENGTH];
	string neBuf = inBuf;

	transform(neBuf.begin(),neBuf.end(),neBuf.begin(),::tolower);
	client->stringhash(neBuf.c_str(),strhash,&key);

	char* buf = new char[8*4/3+4];
	Base64::btoa(strhash,8,buf);
	return buf;
}

const char* MegaApi::ebcEncryptKey(const char* encryptionKey, const char* plainKey)
{
	if(!encryptionKey || !plainKey) return NULL;

	char pwkey[SymmCipher::KEYLENGTH];
	Base64::atob(encryptionKey, (byte *)pwkey, sizeof pwkey);

	SymmCipher key;
	key.setkey((byte*)pwkey);

	char plkey[SymmCipher::KEYLENGTH];
	Base64::atob(plainKey, (byte*)plkey, sizeof plkey);
	key.ecb_encrypt((byte*)plkey);

	char* buf = new char[SymmCipher::KEYLENGTH*4/3+4];
	Base64::btoa((byte*)plkey, SymmCipher::KEYLENGTH, buf);
	return buf;
}

handle MegaApi::base64ToHandle(const char* base64Handle)
{
	if(!base64Handle) return UNDEF;

	handle h = 0;
	Base64::atob(base64Handle,(byte*)&h,MegaClient::NODEHANDLE);
	return h;
}

void MegaApi::retryPendingConnections()
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_RETRY_PENDING_CONNECTIONS);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::fastLogin(const char* email, const char *stringHash, const char *base64pwkey, MegaRequestListener *listener)
{    
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_FAST_LOGIN, listener);
	request->setEmail(email);
	request->setPassword(stringHash);
	request->setPrivateKey(base64pwkey);
	requestQueue.push(request);
	curl->notify();
}


void MegaApi::login(const char *login, const char *password, MegaRequestListener *listener)
{   
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_LOGIN, listener);
	request->setEmail(login);
	request->setPassword(password);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::createAccount(const char* email, const char* password, const char* name, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_CREATE_ACCOUNT, listener);
	request->setEmail(email);
	request->setPassword(password);
	request->setName(name);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::fastCreateAccount(const char* email, const char *base64pwkey, const char* name, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_FAST_CREATE_ACCOUNT, listener);
	request->setEmail(email);
	request->setPassword(base64pwkey);
	request->setName(name);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::querySignupLink(const char* link, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_QUERY_SIGNUP_LINK, listener);
	request->setLink(link);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::confirmAccount(const char* link, const char *password, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_CONFIRM_ACCOUNT, listener);
	request->setLink(link);
	request->setPassword(password);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::fastConfirmAccount(const char* link, const char *base64pwkey, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_FAST_CONFIRM_ACCOUNT, listener);
	request->setLink(link);
	request->setPassword(base64pwkey);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::loop()
{
	while(!threadExit)
	{
		pthread_mutex_lock(&fileSystemMutex);
		sendPendingTransfers();
		sendPendingRequests();
		client->exec();
		pthread_mutex_unlock(&fileSystemMutex);

		client->wait();
	}
}


void MegaApi::createFolder(const char *name, Node *parent, MegaRequestListener *listener)
{	
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_MKDIR, listener);
	if(parent) request->setParentHandle(parent->nodehandle);
	request->setName(name);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::moveNode(Node *node, Node *newParent, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_MOVE, listener);
	if(node) request->setNodeHandle(node->nodehandle);
	if(newParent) request->setParentHandle(newParent->nodehandle);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::copyNode(Node* node, Node* target, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_COPY, listener);
	if(node) request->setNodeHandle(node->nodehandle);
	if(target) request->setParentHandle(target->nodehandle);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::renameNode(Node *node, const char *newName, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_RENAME, listener);
	if(node) request->setNodeHandle(node->nodehandle);
	request->setName(newName);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::remove(Node* node, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_REMOVE, listener);
	if(node) request->setNodeHandle(node->nodehandle);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::share(Node* node, User* user, const char *access, MegaRequestListener *listener)
{
	return share(node, user->email.c_str(), access);
}

void MegaApi::share(Node* node, const char* email, const char *access, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_SHARE, listener);
	if(node) request->setNodeHandle(node->nodehandle);
	request->setEmail(email);
	request->setAccess(access);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::folderAccess(const char* megaFolderLink, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_FOLDER_ACCESS, listener);
	request->setLink(megaFolderLink);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::importFileLink(const char* megaFileLink, Node *parent, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_IMPORT_LINK, listener);
	if(parent) request->setParentHandle(parent->nodehandle);
	request->setLink(megaFileLink);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::importPublicNode(Node *publicNode, Node* parent, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_IMPORT_NODE, listener);

	if(publicNode)
	{
		NewNode *newnode = new NewNode[1];
		string attrstring;

		// copy core properties
		*(NodeCore*)newnode = *(NodeCore*)publicNode;

		// generate encrypted attribute string
		publicNode->attrs.getjson(&attrstring);
		client->makeattr(&publicNode->key,&newnode->attrstring,attrstring.c_str());
		newnode->source = NEW_PUBLIC;
		newnode->parenthandle = UNDEF;

		request->setPublicNode((Node *)newnode);
	}

	if(parent)	request->setParentHandle(parent->nodehandle);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::getPublicNode(const char* megaFileLink, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_GET_PUBLIC_NODE, listener);
	request->setLink(megaFileLink);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::exportNode(Node *node, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_EXPORT, listener);
	if(node) request->setNodeHandle(node->nodehandle);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::fetchNodes(MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_FETCH_NODES, listener);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::getAccountDetails(MegaRequestListener *listener)
{
	getAccountDetails(1, 1, 1, 0, 0, 0, listener);
}

void MegaApi::getAccountDetails(int storage, int transfer, int pro, int transactions, int purchases, int sessions, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_ACCOUNT_DETAILS, listener);
	int numDetails = 0;
	if(storage) numDetails += 0x01;
	if(transfer) numDetails += 0x02;
	if(pro) numDetails += 0x04;
	if(transactions) numDetails += 0x08;
	if(purchases) numDetails += 0x10;
	if(sessions) numDetails += 0x20;
	request->setNumDetails(numDetails);

	requestQueue.push(request);
	curl->notify();
}

void MegaApi::changePassword(const char *oldPassword, const char *newPassword, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_CHANGE_PW, listener);
	request->setPassword(oldPassword);
	request->setNewPassword(newPassword);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::logout(MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_LOGOUT, listener);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::getNodeAttribute(Node* node, int type, char *dstFilePath, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_GET_ATTR_FILE, listener);
	request->setFile(dstFilePath);
	request->setAttrType(type);
	if(node) request->setNodeHandle(node->nodehandle);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::setNodeAttribute(Node* node, int type, char *srcFilePath, MegaRequestListener *listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_SET_ATTR_FILE, listener);
	request->setFile(srcFilePath);
	request->setAttrType(type);
	if(node) request->setNodeHandle(node->nodehandle);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::addContact(const char* email, MegaRequestListener* listener)
{
	MegaRequest *request = new MegaRequest(MegaRequest::TYPE_ADD_CONTACT, listener);
	request->setEmail(email);
	requestQueue.push(request);
	curl->notify();
}

void MegaApi::startUpload(const char* localPath, Node* parent, int connections, int maxSpeed, const char* fileName, MegaTransferListener *listener)
{
	MegaTransfer* transfer = new MegaTransfer(MegaTransfer::TYPE_UPLOAD, listener);
	transfer->setPath(localPath);
	if(parent) transfer->setParentHandle(parent->nodehandle);
	transfer->setNumConnections(connections);
	transfer->setMaxSpeed(maxSpeed);
	transfer->setMaxRetries(maxRetries);
	if(fileName) transfer->setFileName(fileName);

	transferQueue.push(transfer);
	curl->notify();
}

void MegaApi::startUpload(const char* localPath, Node* parent, MegaTransferListener *listener)
{ return startUpload(localPath, parent, 1, 0, (const char *)NULL, listener); }

void MegaApi::startUpload(const char* localPath, Node* parent, const char* fileName, MegaTransferListener *listener)
{ return startUpload(localPath, parent, 1, 0, fileName, listener); }


void MegaApi::startDownload(handle nodehandle, const char* target, int connections, long startPos, long endPos, const char* base64key, MegaTransferListener *listener)
{
	MegaTransfer* transfer = new MegaTransfer(MegaTransfer::TYPE_DOWNLOAD, listener);

	int c = target[strlen(target)-1];
	if((c=='/') || (c == '\\')) transfer->setParentPath(target);
	else transfer->setPath(target);

	transfer->setNodeHandle(nodehandle);
	transfer->setBase64Key(base64key);
	transfer->setNumConnections(connections);
	transfer->setStartPos(startPos);
	transfer->setEndPos(endPos);
	transfer->setMaxRetries(maxRetries);

	transferQueue.push(transfer);
	curl->notify();
}

void MegaApi::startDownload(Node* node, const char* target, int connections, long startPos, long endPos, const char* base64key, MegaTransferListener *listener)
{ startDownload((node != NULL) ? node->nodehandle : UNDEF,target,1,startPos,endPos,base64key,listener); }

void MegaApi::startDownload(Node* node, const char* localFolder, long startPos, long endPos, MegaTransferListener *listener)
{ startDownload((node != NULL) ? node->nodehandle : UNDEF,localFolder,1,startPos,endPos,NULL,listener); }

void MegaApi::startDownload(Node* node, const char* localFolder, MegaTransferListener *listener)
{ startDownload((node != NULL) ? node->nodehandle : UNDEF, localFolder, 1, 0, 0, NULL, listener); }

void MegaApi::startPublicDownload(Node* node, const char* localFolder, MegaTransferListener *listener)
{
	if(node)
	{
		char base64Key[Node::FILENODEKEYLENGTH*4/3+3];
		Base64::btoa((const byte*)node->nodekey.data(),Node::FILENODEKEYLENGTH,base64Key);
		startDownload(node->nodehandle, localFolder, 1, 0, 0, base64Key, listener);
		return;
	}
	startDownload(UNDEF, localFolder, 1, 0, 0, NULL, listener);
}

void MegaApi::startPublicDownload(handle nodehandle, const char *base64key, const char* localFolder, MegaTransferListener *listener)
{ startDownload(nodehandle, localFolder, 1, 0, 0, base64key, listener); }

void MegaApi::cancelTransfer(MegaTransfer *transfer)
{
	if(!transfer) return;

	int td = transfer->getSlot();
	client->tclose(td);
	string tmpfilename = transfer->getPath();
	tmpfilename.append(".tmp");
	unlink(tmpfilename.c_str());
	transferMap[transfer->getTag()] = NULL;
	delete transfer;
}

Node *MegaApi::getRootNode()
{
	pthread_mutex_lock(&fileSystemMutex);
	Node *result = client->nodebyhandle(client->rootnodes[0]);
	pthread_mutex_unlock(&fileSystemMutex);
	return result;
}

Node* MegaApi::getInboxNode()
{
	pthread_mutex_lock(&fileSystemMutex);
	Node *result = client->nodebyhandle(client->rootnodes[1]);
	pthread_mutex_unlock(&fileSystemMutex);
	return result;
}

Node* MegaApi::getRubbishNode()
{
	pthread_mutex_lock(&fileSystemMutex);
	Node *result = client->nodebyhandle(client->rootnodes[2]);
	pthread_mutex_unlock(&fileSystemMutex);
	return result;
}

Node* MegaApi::getMailNode()
{
	pthread_mutex_lock(&fileSystemMutex);
	Node *result = client->nodebyhandle(client->rootnodes[3]);
	pthread_mutex_unlock(&fileSystemMutex);
	return result;
}


bool MegaApi::userComparatorDefaultASC (User *i, User *j)
{
	if(strcasecmp(i->email.c_str(), j->email.c_str())<=0) return 1;
	return 0;
};

UserList* MegaApi::getContacts()
{
	pthread_mutex_lock(&fileSystemMutex);

	vector<User*> vUsers;
	for (user_map::iterator it = client->users.begin() ; it != client->users.end() ; it++ )
	{
		User *u = &(it->second);
		vector<User *>::iterator i = std::lower_bound(vUsers.begin(), vUsers.end(), u, MegaApi::userComparatorDefaultASC);
		vUsers.insert(i, u);
	}
	UserList *userList = new UserList(&(vUsers[0]), vUsers.size(), 1);

	pthread_mutex_unlock(&fileSystemMutex);

	return userList;
}

User* MegaApi::getContact(const char* email)
{
	pthread_mutex_lock(&fileSystemMutex);
	User *user = client->finduser(email, 0);
	pthread_mutex_unlock(&fileSystemMutex);
	return user;
}

NodeList* MegaApi::getInShares(User *user)
{
	if(!user) return new NodeList(NULL, 0, 0);

	pthread_mutex_lock(&fileSystemMutex);
	vector<Node*> vNodes;

	Node *n;
	for (handle_set::iterator sit = user->sharing.begin(); sit != user->sharing.end(); sit++)
	{
		if ((n = client->nodebyhandle(*sit)))
			vNodes.push_back(n);
	}
	NodeList *nodeList = new NodeList(&(vNodes[0]), vNodes.size(), 1);
	pthread_mutex_unlock(&fileSystemMutex);
	return nodeList;
}

ShareList* MegaApi::getOutShares(Node *node)
{
	if(!node) return new ShareList(NULL, 0, 0);

	pthread_mutex_lock(&fileSystemMutex);
	node = client->nodebyhandle(node->nodehandle);
	if(!node) 
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return new ShareList(NULL, 0, 0);
	}
	
	vector<Share*> vShares;

	for (share_map::iterator it = node->outshares.begin(); it != node->outshares.end(); it++)
	{
		vShares.push_back(it->second);
	}

	ShareList *shareList = new ShareList(&(vShares[0]), vShares.size(), 1);
	pthread_mutex_unlock(&fileSystemMutex);
	return shareList;
}

const char *MegaApi::getAccess(Node* node)
{
	if(!node) return NULL;

	pthread_mutex_lock(&fileSystemMutex);
	node = client->nodebyhandle(node->nodehandle);
	if(!node)
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return NULL;
	}
	
	if (!client->loggedin())
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return "r";
	}
	if(node->type > FOLDERNODE)
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return "own";
	}

	Node *n = node;
	accesslevel a = FULL;
	while (n)
	{
		if (n->inshare) { a = n->inshare->access; break; }
		n = client->nodebyhandle(n->parenthandle);
	}

	pthread_mutex_unlock(&fileSystemMutex);

	switch(a)
	{
	case RDONLY: return "r";
	case RDWR: return "rw";
	default: return "full";
	}
}

bool MegaApi::processTree(Node* node, TreeProcessor* processor, bool recursive)
{
	if(!node) return 1; 
	if(!processor) return 0;

	pthread_mutex_lock(&fileSystemMutex);
	node = client->nodebyhandle(node->nodehandle);
	if(!node) 
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return 1;
	}

	if (node->type != FILENODE)
	{
		for (node_list::iterator it = node->children.begin(); it != node->children.end(); ) 
		{    
			if(recursive)
			{
				if(!processTree(*it++,processor))
				{
					pthread_mutex_unlock(&fileSystemMutex);
					return 0;
				}
			}
			else
			{
				if(!processor->processNode(*it++))
				{
					pthread_mutex_unlock(&fileSystemMutex);
					return 0;
				}
			}
		}
	}
	bool result = processor->processNode(node);

	pthread_mutex_unlock(&fileSystemMutex);
	return result;
}

NodeList* MegaApi::search(Node* node, const char* searchString, bool recursive)
{
	if(!node || !searchString) return new NodeList(NULL, 0, 0);

	pthread_mutex_lock(&fileSystemMutex);
	node = client->nodebyhandle(node->nodehandle);
	if(!node)
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return new NodeList(NULL, 0, 0);
	}

	SearchTreeProcessor searchProcessor(searchString);
	processTree(node, &searchProcessor, recursive);
	vector<Node *>& vNodes = searchProcessor.getResults();
	NodeList *nodeList = new NodeList(&(vNodes[0]), vNodes.size(), 1);
	pthread_mutex_unlock(&fileSystemMutex);

	return nodeList;
}

SearchTreeProcessor::SearchTreeProcessor(const char *search) { this->search = search; }

int SearchTreeProcessor::processNode(Node* node)
{
	if(!node) return 1;
	if(!search) return 0;
#ifndef _WIN32
	if(strcasestr(node->displayname(), search)!=NULL) results.push_back(node);
//TODO: Implement this for Windows
#endif
	return 1;
}

vector<Node *> &SearchTreeProcessor::getResults()
{
	return results;
}

FileAccess* MegaApi::newfile()
{
	return new CFileAccess();
}

// topen() failed
void MegaApi::topen_result(int td, error e)
{
	MegaTransfer* transfer = transferMap[client->ft[td].tag];
	if(!transfer) return;

	transfer->setTime(client->httpio->ds);
	MegaError megaError(e);
	if (e) cout << "TD " << td << ": Failed to open file (" << megaError.getErrorString() << ")" << endl;
	client->tclose(td);
	fireOnTransferFinish(this, transfer, megaError);
}

// topen() succeeded (download only)
void MegaApi::topen_result(int td, string* filename, const char* fa, int pfa)
{
	MegaTransfer* transfer = transferMap[client->ft[td].tag];
	if(!transfer) return;

	transfer->setTime(client->httpio->ds);

	//cout << "TD " << td << ": File opened successfully, filename: " << *filename << endl;
	//if (fa) cout << "File has attributes: " << fa << " / " << pfa << endl;
	MegaClient::escapefilename(filename);

	string tmpfilename;
	if(transfer->getPath()) tmpfilename = transfer->getPath();
	else
	{
		tmpfilename = transfer->getParentPath();
		tmpfilename += "/";
		tmpfilename += *filename;
		transfer->setPath(tmpfilename.c_str());
	}
	tmpfilename.append(".tmp");

	client->dlopen(td,tmpfilename.c_str());
	fireOnTransferUpdate(this, transfer);
}

void MegaApi::transfer_update(int td, m_off_t bytes, m_off_t size, dstime starttime)
{
	MegaTransfer* transfer = transferMap[client->ft[td].tag];
	if(!transfer) return;

	//cout << "TD " << td << ": Update: " << bytes/1024 << " KB of " << size/1024 << " KB, " << bytes*10/(1024*(dstime()-starttime)+1) << " KB/s" << endl;

	transfer->setTransferredBytes(bytes);
	transfer->setTotalBytes(size);
	transfer->setStartTime(starttime);

	//Do not send notifications too often
	if(client->httpio->ds - transfer->getTime())
	{
		transfer->setTime(client->httpio->ds);
		fireOnTransferUpdate(this, transfer);
	}
}

int MegaApi::transfer_error(int td, int httpcode, int count)
{
	MegaTransfer* transfer = transferMap[client->ft[td].tag];
	if(!transfer) return 1;

	cout << "TD " << td << ": Failed, HTTP error code " << httpcode << " (count " << count << ")" << endl;

	transfer->setNumRetry(count);
	transfer->setTime(client->httpio->ds);

	if(count<transfer->getMaxRetries())
	{
		fireOnTransferTemporaryError(this, transfer, MegaError(httpcode));
		return 0; //Retry
	}	
	else
	{
		fireOnTransferFinish(this, transfer, MegaError(httpcode));
		return 1; //Abort
	}
}

void MegaApi::transfer_failed(int td, string& filename, error e)
{
	MegaTransfer* transfer = transferMap[client->ft[td].tag];
	if(!transfer) return;

	MegaError megaError(e);
	transfer->setPath(filename.c_str());
	transfer->setTime(client->httpio->ds);
	if(e) cout << "TD " << td << ": Download failed (" << megaError.getErrorString() << ")" << endl;
	filename.append(".tmp");
	unlink(filename.c_str());
	fireOnTransferFinish(this, transfer, megaError);
}

void MegaApi::transfer_failed(int td, error e)
{
	MegaError megaError(e);
	MegaTransfer* transfer = transferMap[client->ft[td].tag];
	if(!transfer) return;

	transfer->setTime(client->httpio->ds);

	cout << "TD " << td << ": Upload failed (" << megaError.getErrorString() << ")" << endl;
	fireOnTransferFinish(this, transfer, megaError);
}

void MegaApi::transfer_limit(int td)
{
	MegaTransfer* transfer = transferMap[client->ft[td].tag];
	if(!transfer) return;

	transfer->setTime(client->httpio->ds);

	cout << "TD " << td << ": Transfer limit reached." << endl;
	fireOnTransferFinish(this, transfer, MegaError(API_EOVERQUOTA));
}

void MegaApi::transfer_complete(int td, chunkmac_map* macs, const char* fn)
{
	MegaTransfer* transfer = transferMap[client->ft[td].tag];
	if(!transfer) return;

	transfer->setTime(client->httpio->ds);

	string filename = transfer->getPath();
	string tmpfilename = transfer->getPath();
	tmpfilename.append(".tmp");

	client->tclose(td);

	if (!::rename(tmpfilename.c_str(),filename.c_str())) fireOnTransferFinish(this, transfer, MegaError(API_OK));
	else fireOnTransferFinish(this, transfer, MegaError(API_EACCESS));
}

void MegaApi::transfer_complete(int td, handle ulhandle, const byte* ultoken, const byte* filekey, SymmCipher* key)
{
	MegaTransfer* transfer = transferMap[client->ft[td].tag];
	if(!transfer) return;

	transfer->setTime(client->httpio->ds);

	Node* n;

	//cout << "TD " << td << ": Upload complete" << endl;

	handle uploadtarget = transfer->getParentHandle();
	if (!(n = client->nodebyhandle(uploadtarget)))
	{
		cout << "Upload target folder inaccessible" << endl;
		fireOnTransferFinish(this, transfer, MegaError(API_EACCESS));
		return;
	}

	string uploadfilename(transfer->getFileName());
	NewNode *newnode = new NewNode[1];

	// build new node
	newnode->source = NEW_UPLOAD;

	//TODO: Check this
	if(!ulhandle) ulhandle = client->uploadhandle(td);

	// upload handle required to retrieve pending file attributes	
	newnode->uploadhandle = ulhandle;

	// reference to uploaded file
	memcpy(newnode->uploadtoken,ultoken,sizeof newnode->uploadtoken);

	// file's crypto key
	newnode->nodekey.assign((char*)filekey,Node::FILENODEKEYLENGTH);
	newnode->mtime = newnode->ctime = time(NULL);
	newnode->type = FILENODE;
	newnode->parenthandle = UNDEF;

	AttrMap attrs;

	MegaClient::unescapefilename(&uploadfilename);

	attrs.map['n'] = uploadfilename;
	attrs.getjson(&uploadfilename);

	client->makeattr(key,&newnode->attrstring,uploadfilename.c_str());
	client->tclose(td);


	MegaRequest *request = new MegaRequest(transfer);
	requestMap[client->nextreqtag()]=request;

	//TODO: Send files to users
	client->putnodes(uploadtarget,newnode,1);
}

// user addition/update (users never get deleted)
void MegaApi::users_updated(User** u, int count)
{
	//if (count == 1) cout << "1 user received" << endl;
	//else cout << count << " users received" << endl;

	UserList* userList = new UserList(u, count);
	fireOnUsersUpdate(this, userList);
	delete userList;
}

void MegaApi::setattr_result(handle h, error e)
{
	MegaError megaError(e);
	if(e) cout << "Node attribute update failed (" << megaError.getErrorString() << ")" << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;
	
	if(request->getType() != MegaRequest::TYPE_RENAME) cout << "INCORRECT REQUEST OBJECT (1)";
	request->setNodeHandle(h);
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::rename_result(handle h, error e)
{
	MegaError megaError(e);
	if(e) cout << "Node move failed (" << megaError.getErrorString() << ")" << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if(request->getType() != MegaRequest::TYPE_MOVE) cout << "INCORRECT REQUEST OBJECT (2)";
	request->setNodeHandle(h);
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::unlink_result(handle h, error e)
{
	MegaError megaError(e);
	if(e) cout << "Node deletion failed (" << megaError.getErrorString() << ")" << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if(request->getType() != MegaRequest::TYPE_REMOVE) cout << "INCORRECT REQUEST OBJECT (3)";
	request->setNodeHandle(h);
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::fetchnodes_result(error e)
{
	MegaError megaError(e);
    if(e) cout << "File/folder retrieval failed (" << megaError.getErrorString() << ") Auth: " << getAuthString() << endl;

	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if((request->getType() != MegaRequest::TYPE_FETCH_NODES) && (request->getType() != MegaRequest::TYPE_FOLDER_ACCESS))
		cout << "INCORRECT REQUEST OBJECT (4)";
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::putnodes_result(error e, targettype t, NewNode* nn)
{
	MegaError megaError(e);
	if (t == USER_HANDLE)
	{
		delete[] nn;	// free array allocated by the app
		if (!e) cout << "Success." << endl;
		return; //TODO: Check this
	}

	if(e) cout << "Node addition failed (" << megaError.getErrorString() << ")" << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if((request->getType() != MegaRequest::TYPE_IMPORT_LINK) && (request->getType() != MegaRequest::TYPE_MKDIR) &&
			(request->getType() != MegaRequest::TYPE_COPY) && (request->getType() != MegaRequest::TYPE_UPLOAD) &&
			(request->getType() != MegaRequest::TYPE_IMPORT_NODE))
		cout << "INCORRECT REQUEST OBJECT (5)";


	handle h = UNDEF;
	Node *n = NULL;
	if(client->nodenotify.size()) n = client->nodenotify.back();
	if(n) n->applykey(client);

	if(request->getType() == MegaRequest::TYPE_UPLOAD)
	{
		handlepair_set::iterator it;
		it = client->uhnh.lower_bound(pair<handle,handle>(nn->uploadhandle,0));
		if (it != client->uhnh.end() && it->first == nn->uploadhandle) h = it->second;

		request->getTransfer()->setNodeHandle(h);
		fireOnTransferFinish(this, request->getTransfer(), megaError);
		requestMap.erase(client->restag);
		delete request;
		return;
	}

	if(n) h = n->nodehandle;
	request->setNodeHandle(h);
	fireOnRequestFinish(this, request, megaError);
	delete [] nn;
}

void MegaApi::share_result(error e)
{
	MegaError megaError(e);
	cout << "Share creation/modification request failed (" << megaError.getErrorString() << ")" << endl;

	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if(request->getType() == MegaRequest::TYPE_EXPORT)
	{ 
		return;
		//exportnode_result will be called to end the request.
	}
	if(request->getType() != MegaRequest::TYPE_SHARE) cout << "INCORRECT REQUEST OBJECT (6)";
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::share_result(int, error e)
{
	MegaError megaError(e);
	if (e) cout << "Share creation/modification failed (" << megaError.getErrorString() << ")" << endl;
	else cout << "Share creation/modification succeeded" << endl;

	cout << "First share tag: " << client->restag << endl;

	//The other callback will be called at the end of the request
	//MegaRequest *request = requestQueue.front();
	//if(request->getType() == MegaRequest::TYPE_EXPORT) return; //exportnode_result will be called to end the request.
	//if(request->getType() != MegaRequest::TYPE_SHARE) cout << "INCORRECT REQUEST OBJECT";
	//fireOnRequestFinish(this, request, megaError);
}

void MegaApi::fa_complete(Node* n, fatype type, const char* data, uint32_t len)
{
	cout << "Got attribute of type " << type << " (" << len << " bytes) for " << n->displayname() << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if(request->getType() != MegaRequest::TYPE_GET_ATTR_FILE) cout << "INCORRECT REQUEST OBJECT (fa_complete)";

	FileAccess *f = this->newfile();
	f->fopen(request->getFile(), 0, 1);
	f->fwrite((const byte*)data, len, 0);
	delete f;
	fireOnRequestFinish(this, request, MegaError(API_OK));
}

int MegaApi::fa_failed(handle, fatype type, int retries)
{
	cout << "File attribute retrieval of type " << type << " failed (retries: " << retries << ")" << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return 1;

	if(request->getType() != MegaRequest::TYPE_GET_ATTR_FILE) cout << "INCORRECT REQUEST OBJECT (fa_complete)";
	if(retries > 3)
	{
		fireOnRequestFinish(this, request, MegaError(API_EINTERNAL));
		return 1;
	}
	fireOnRequestTemporaryError(this, request, MegaError(API_EAGAIN));
	return 0;
}

void MegaApi::putfa_result(handle, fatype, error e)
{
	MegaError megaError(e);
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if(e) cout << "File attribute attachment failed (" << megaError.getErrorString() << ")" << endl;
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::clearing()
{
	cout << "Clearing all nodes/users..." << endl;
}

void MegaApi::notify_retry(dstime dsdelta)
{
	cout << "API request failed, retrying in " << dsdelta*100 << " ms..." << endl;
	/*
	 * MegaRequest *request = requestMap[client->restag];
	 * request->setNextRetryDelay(dsdelta*100);
	 * request->setNumRetry(request->getNumRetry()+1);
	 * fireOnRequestTemporaryError(this, request, MegaError(API_EAGAIN));
	 * */
}

// callback for non-EAGAIN request-level errors
// retrying is futile
// this can occur e.g. with syntactically malformed requests (due to a bug) or due to an invalid application key
void MegaApi::request_error(error e)
{	
	MegaError megaError(e);
	cout << "FATAL: Request failed (" << megaError.getErrorString() << "), exiting" << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	fireOnRequestFinish(this, request, megaError);
}

// login result
void MegaApi::login_result(error result)
{    
	MegaError megaError(result);
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

    if(result) cout << "Login failed" << endl;
    else cout << "Login OK" << endl;

	if((request->getType() != MegaRequest::TYPE_LOGIN) && (request->getType() != MegaRequest::TYPE_FAST_LOGIN)) 
		cout << "INCORRECT REQUEST OBJECT (7) " << request->getRequestString() << endl;

	/* Support to renew an expired SID. Deactivated. It needs SDK changes.
		if(loginRequest) delete loginRequest;
		updateSIDtime = curl->ds;
		if(result == API_OK)
		{
			loginRequest = new MegaRequest(*request);
			if(updatingSID)
			{
			updatingSID = 0;
			cout << "SID updated OK!" << endl;
			//This is an internal request.
			//Deleting request without calling listeners.
			requestMap.erase(client->restag);
			//delete request; Alredy deleted. It's loginRequest
			curl->notify(); //Wake up pending request
			return;
			}
		}
		else
		{
			loginRequest = NULL;
			if(updatingSID)
			{
			updatingSID = 0;
			cout << "SID update FAILED!" << endl;
			//This is an internal request.
			//Deleting request without calling listeners.
			requestMap.erase(client->restag);
			//delete request; Alredy deleted. It's loginRequest
			curl->notify(); //Wake up pending request
			return;
			}
		}
	 */

	fireOnRequestFinish(this, request, megaError);
}

// password change result
void MegaApi::changepw_result(error result)
{
	MegaError megaError(result);
	if (result == API_OK) cout << "Password updated." << endl;
	else cout << "Password update failed: " << megaError.getErrorString() << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if(request->getType() != MegaRequest::TYPE_CHANGE_PW) cout << "INCORRECT REQUEST OBJECT (8)";
	fireOnRequestFinish(this, request, megaError);
}

// node export failed
void MegaApi::exportnode_result(error result)
{
	MegaError megaError(result);
	cout << "Export failed: " << megaError.getErrorString() << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if(request->getType() != MegaRequest::TYPE_EXPORT) cout << "INCORRECT REQUEST OBJECT (9)";
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::exportnode_result(handle h, handle ph)
{
	Node* n;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if(request->getType() != MegaRequest::TYPE_EXPORT) cout << "INCORRECT REQUEST OBJECT (10)";		

	if ((n = client->nodebyhandle(h)))
	{
		cout << "Exported node finished" << endl;

		char node[9];
		char key[Node::FILENODEKEYLENGTH*4/3+3];

		Base64::btoa((byte*)&ph,MegaClient::NODEHANDLE,node);

		// the key
		if (n->type == FILENODE) Base64::btoa((const byte*)n->nodekey.data(),Node::FILENODEKEYLENGTH,key);
		else if (n->sharekey) Base64::btoa(n->sharekey->key,Node::FOLDERNODEKEYLENGTH,key);
		else
		{
			cout << "No key available for exported folder" << endl;
			fireOnRequestFinish(this, request, MegaError(MegaError::API_EKEY));
			return;
		}

		string link = "https://mega.co.nz/#";
		link += (n->type ? "F" : ""); 
		link += "!";
		link += node;
		link += "!";
		link += key;
		request->setLink(link.c_str());
		fireOnRequestFinish(this, request, MegaError(MegaError::API_OK));
	}
	else 
	{
		request->setNodeHandle(UNDEF);
		cout << "Exported node no longer available" << endl;
		fireOnRequestFinish(this, request, MegaError(MegaError::API_ENOENT));
	}
}

// the requested link could not be opened
void MegaApi::openfilelink_result(error result)
{
	MegaError megaError(result);
	cout << "Failed to open link: " << megaError.getErrorString() << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if((request->getType() != MegaRequest::TYPE_IMPORT_LINK) && (request->getType() != MegaRequest::TYPE_GET_PUBLIC_NODE))
		cout << "INCORRECT REQUEST OBJECT (11)";

	fireOnRequestFinish(this, request, megaError);
}

// the requested link was opened successfully
// (it is the application's responsibility to delete n!)
void MegaApi::openfilelink_result(Node* n)
{
	//cout << "Importing " << n->displayname() << "..." << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if((request->getType() != MegaRequest::TYPE_IMPORT_LINK) && (request->getType() != MegaRequest::TYPE_GET_PUBLIC_NODE))
		cout << "INCORRECT REQUEST OBJECT (12)";

	if (!client->loggedin()) 
	{
		cout << "Need to be logged in to import file links." << endl;
		fireOnRequestFinish(this, request, MegaError(MegaError::API_EACCESS));
	}
	else
	{
		if(request->getType() == MegaRequest::TYPE_IMPORT_LINK)
		{
			NewNode *newnode = new NewNode[1];
			string attrstring;

			// copy core properties
			*(NodeCore*)newnode = *(NodeCore*)n;

			// generate encrypted attribute string
			n->attrs.getjson(&attrstring);
			client->makeattr(&n->key,&newnode->attrstring,attrstring.c_str());

			delete n;

			newnode->source = NEW_PUBLIC;
			newnode->parenthandle = UNDEF;

			// add node		
			requestMap.erase(client->restag);
			requestMap[client->nextreqtag()]=request;
			client->putnodes(request->getParentHandle(),newnode,1);
		}
		else
		{
			request->setPublicNode(n);
			fireOnRequestFinish(this, request, MegaError(MegaError::API_OK));
		}
	}
}

// reload needed
void MegaApi::reload(const char* reason)
{
	cout << "Reload suggested (" << reason << ")" << endl;
	fireOnReloadNeeded(this);
}


void MegaApi::debug_log(const char* message)
{
	//cout << "DEBUG: " << message << endl;
}


// nodes have been modified
// (nodes with their removed flag set will be deleted immediately after returning from this call,
// at which point their pointers will become invalid at that point.)
void MegaApi::nodes_updated(Node** n, int count)
{
	/*int c[2][6] = { { 0 } };

	if (n)
	{
		while (count--)
			if ((*n)->type < 6)
			{
				c[!(*n)->removed][(*n)->type]++;
				n++;
			}
	}
	else
	{
		for (node_map::iterator it = client->nodes.begin(); it != client->nodes.end(); it++)
			if (it->second->type < 6)
				c[1][it->second->type]++;
	}

	nodestats(c[1],"added or updated");
	nodestats(c[0],"removed");
	 */

	NodeList *nodeList = NULL;
	if(n != NULL) nodeList = new NodeList(n, count);
	fireOnNodesUpdate(this, nodeList);
	delete nodeList;
}

// display account details/history
void MegaApi::account_details(AccountDetails* ad, int storage, int transfer, int pro, int purchases, int transactions, int sessions)
{
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	int numDetails = request->getNumDetails();
	numDetails--;
	request->setNumDetails(numDetails);
	if(!numDetails)
	{
		if(request->getType() != MegaRequest::TYPE_ACCOUNT_DETAILS) cout << "INCORRECT REQUEST OBJECT (13)";
		fireOnRequestFinish(this, request, MegaError(MegaError::API_OK));
	}
}

void MegaApi::account_details(AccountDetails* ad, error e)
{
	MegaError megaError(e);
	cout << "Account details retrieval failed (" << megaError.getErrorString() << ")" << endl;
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if(request->getType() != MegaRequest::TYPE_ACCOUNT_DETAILS) cout << "INCORRECT REQUEST OBJECT (14)";
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::invite_result(error e)
{
	MegaError megaError(e);
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if (e) cout << "Invitation failed (" << megaError.getErrorString() << ")" << endl;
	else cout << "Success." << endl;
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::putua_result(error e)
{
	MegaError megaError(e);
	//MegaRequest *request = requestMap[client->restag];
	if (e) cout << "User attribute update failed (" << megaError.getErrorString() << ")" << endl;
	else cout << "Success." << endl;
	//fireOnRequestFinish(this, request, megaError);
}

void MegaApi::getua_result(error e)
{
	MegaError megaError(e);
	//MegaRequest *request = requestMap[client->restag];
	cout << "User attribute retrieval failed (" << megaError.getErrorString() << ")" << endl;
	//fireOnRequestFinish(this, request, megaError);
}

void MegaApi::getua_result(byte* data, unsigned l)
{
	//MegaError megaError(API_OK);
	//MegaRequest *request = requestMap[client->restag];
	cout << "Received " << l << " byte(s) of user attribute: ";
	//fwrite(data,1,l,stdout);
	//cout << endl;
	//fireOnRequestFinish(this, request, megaError);
}

// user attribute update notification
void MegaApi::userattr_update(User* u, int priv, const char* n)
{
	cout << "Notification: User " << u->email << " -" << (priv ? " private" : "") << " attribute " << n << " added or updated" << endl;
}

void MegaApi::ephemeral_result(error e)
{
    cout << "Ephemeral error" << endl;

	MegaError megaError(e);
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if (e) cout << "Ephemeral session error (" << megaError.getErrorString() << ")" << endl;
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::ephemeral_result(handle uh, const byte* pw)
{
    cout << "Ephemeral ok" << endl;

	MegaRequest *request = requestMap[client->restag];
    if(!request)
    {
        cout << "No request" << endl;
        return;
    }

	if((request->getType() != MegaRequest::TYPE_CREATE_ACCOUNT) &&
		(request->getType() != MegaRequest::TYPE_FAST_CREATE_ACCOUNT))
		cout << "INCORRECT REQUEST OBJECT (15)";

	requestMap.erase(client->restag);
	requestMap[client->nextreqtag()]=request;

	byte pwkey[SymmCipher::KEYLENGTH];
	if(request->getType() == MegaRequest::TYPE_CREATE_ACCOUNT)
		client->pw_key(request->getPassword(),pwkey);
	else
		Base64::atob(request->getPassword(), (byte *)pwkey, sizeof pwkey);

    cout << "Send signup link" << endl;
	client->sendsignuplink(request->getEmail(),request->getName(),pwkey);
}

void MegaApi::sendsignuplink_result(error e)
{
	MegaError megaError(e);
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if((request->getType() != MegaRequest::TYPE_CREATE_ACCOUNT) &&
		(request->getType() != MegaRequest::TYPE_FAST_CREATE_ACCOUNT))
		cout << "INCORRECT REQUEST OBJECT (16)";

	if (e) cout << "Unable to send signup link (" << megaError.getErrorString() << ")" << endl;
	else cout << "Thank you. Please check your e-mail and enter the command signup followed by the confirmation link." << endl;
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::querysignuplink_result(error e)
{
	MegaError megaError(e);
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	cout << "Signuplink confirmation failed (" << megaError.getErrorString() << ")" << endl;
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::querysignuplink_result(handle uh, const char* email, const char* name, const byte* pwc, const byte* kc, const byte* c, size_t len)
{
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	cout << "Ready to confirm user account " << email << " (" << name << ") - enter confirm to execute." << endl;

	request->setEmail(email);
	request->setName(name);

	if(request->getType() == MegaRequest::TYPE_QUERY_SIGNUP_LINK)
	{
		fireOnRequestFinish(this, request, MegaError(API_OK));
		return;
	}

	string signupemail = email;
	string signupcode;
	signupcode.assign((char*)c,len);

	byte signuppwchallenge[SymmCipher::KEYLENGTH];
	byte signupencryptedmasterkey[SymmCipher::KEYLENGTH];

	memcpy(signuppwchallenge,pwc,sizeof signuppwchallenge);
	memcpy(signupencryptedmasterkey,pwc,sizeof signupencryptedmasterkey);

	byte pwkey[SymmCipher::KEYLENGTH];
	if(request->getType() == MegaRequest::TYPE_CONFIRM_ACCOUNT)
		client->pw_key(request->getPassword(),pwkey);
	else
		Base64::atob(request->getPassword(), (byte *)pwkey, sizeof pwkey);

	// verify correctness of supplied signup password
	SymmCipher pwcipher(pwkey);
	pwcipher.ecb_decrypt(signuppwchallenge);

	if (*(uint64_t*)(signuppwchallenge+4))
	{
		cout << endl << "Incorrect password, please try again.";
		fireOnRequestFinish(this, request, MegaError(API_ENOENT));
	}
	else
	{
		// decrypt and set master key, then proceed with the confirmation
		pwcipher.ecb_decrypt(signupencryptedmasterkey);
		client->key.setkey(signupencryptedmasterkey);

		requestMap.erase(client->restag);
		requestMap[client->nextreqtag()]=request;
		//fireOnRequestFinish(this, request, MegaError(API_EACCESS));

		client->confirmsignuplink((const byte*)signupcode.data(),signupcode.size(),MegaClient::stringhash64(&signupemail,&pwcipher));
	}
}

void MegaApi::confirmsignuplink_result(error e)
{
	MegaError megaError(e);
	MegaRequest *request = requestMap[client->restag];
	if(!request) return;

	if (e) cout << "Signuplink confirmation failed (" << megaError.getErrorString() << ")" << endl;
	else
	{
		cout << "Signup confirmed, logging in..." << endl;
		//client->login(signupemail.c_str(),pwkey);
	}
	fireOnRequestFinish(this, request, megaError);
}

void MegaApi::setkeypair_result(error e)
{
	MegaError megaError(e);

	if (e) cout << "RSA keypair setup failed (" << megaError.getErrorString() << ")" << endl;
	else cout << "RSA keypair added. Account setup complete." << endl;
}

void MegaApi::addListener(MegaListener* listener)
{
	pthread_mutex_lock(&listenerMutex);
	listeners.insert(listener);
	pthread_mutex_unlock(&listenerMutex);
}

void MegaApi::addRequestListener(MegaRequestListener* listener)
{
	pthread_mutex_lock(&requestListenerMutex);
	requestListeners.insert(listener);
	pthread_mutex_unlock(&requestListenerMutex);
}

void MegaApi::addTransferListener(MegaTransferListener* listener)
{
	pthread_mutex_lock(&transferListenerMutex);
	transferListeners.insert(listener);
	pthread_mutex_unlock(&transferListenerMutex);
}

void MegaApi::addGlobalListener(MegaGlobalListener* listener)
{
	pthread_mutex_lock(&globalListenerMutex);
	globalListeners.insert(listener);	
	pthread_mutex_unlock(&globalListenerMutex);
}

void MegaApi::removeListener(MegaListener* listener)
{
	pthread_mutex_lock(&listenerMutex);
	listeners.erase(listener);	
	pthread_mutex_unlock(&listenerMutex);
}

void MegaApi::removeRequestListener(MegaRequestListener* listener)
{
	pthread_mutex_lock(&requestListenerMutex);
	requestListeners.erase(listener);
	pthread_mutex_unlock(&requestListenerMutex);
}

void MegaApi::removeTransferListener(MegaTransferListener* listener)
{
	pthread_mutex_lock(&transferListenerMutex);
	transferListeners.erase(listener);	
	pthread_mutex_unlock(&transferListenerMutex);
}

void MegaApi::removeGlobalListener(MegaGlobalListener* listener)
{
	pthread_mutex_lock(&globalListenerMutex);
	globalListeners.erase(listener);	
	pthread_mutex_unlock(&globalListenerMutex);
}

void MegaApi::fireOnRequestStart(MegaApi* api, MegaRequest *request)
{
	pthread_mutex_lock(&requestListenerMutex);
	for(set<MegaRequestListener *>::iterator it = requestListeners.begin(); it != requestListeners.end() ; it++)
		(*it)->onRequestStart(api, request);
	pthread_mutex_unlock(&requestListenerMutex);

	pthread_mutex_lock(&listenerMutex);
	for(set<MegaListener *>::iterator it = listeners.begin(); it != listeners.end() ; it++)		
		(*it)->onRequestStart(api, request);
	pthread_mutex_unlock(&listenerMutex);

	MegaRequestListener* listener = request->getListener();
	if(listener) listener->onRequestStart(api, request);
}


void MegaApi::fireOnRequestFinish(MegaApi* api, MegaRequest *request, MegaError e)
{    
	/*  Renew an expired SID. Deactivated. It needs SDK changes
	//If expired Session ID
	if((e.getErrorCode()==MegaError::API_ESID) && (loginRequest) &&
		(request->getType() != MegaRequest::TYPE_LOGIN) && (request->getType() != MegaRequest::TYPE_FAST_LOGIN))
	{	    
		//If not already updating SID and no unrecoverable error
		if((!updatingSID) && (curl->ds - updateSIDtime) > 1000)
		{
		//Updating SID...
		cout << "Updating SID..." << endl;
		client->setsid(NULL);
		updatingSID = 1;
		requestQueue.push_front(loginRequest);
		}

		//If no unrecoverable error
		if((curl->ds - updateSIDtime) > 1000)
		{
		//Clean request
		requestMap.erase(client->restag);

		//Repeat request after updating SID
		requestQueue.push(request);	

		//Notify cURL	
		curl->notify();
		return;
		}
	}
	 */

	MegaError *megaError = new MegaError(e);

	pthread_mutex_lock(&requestListenerMutex);
	for(set<MegaRequestListener *>::iterator it = requestListeners.begin(); it != requestListeners.end() ; it++)
		(*it)->onRequestFinish(api, request, megaError);
	pthread_mutex_unlock(&requestListenerMutex);

	pthread_mutex_lock(&listenerMutex);
	for(set<MegaListener *>::iterator it = listeners.begin(); it != listeners.end() ; it++)		
		(*it)->onRequestFinish(api, request, megaError);
	pthread_mutex_unlock(&listenerMutex);

	MegaRequestListener* listener = request->getListener();
	if(listener) listener->onRequestFinish(api, request, megaError);

	requestMap.erase(client->restag);
	delete request;
	delete megaError;
}

void MegaApi::fireOnRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError e)
{
	MegaError *megaError = new MegaError(e);

	pthread_mutex_lock(&requestListenerMutex);
	for(set<MegaRequestListener *>::iterator it = requestListeners.begin(); it != requestListeners.end() ; it++)
		(*it)->onRequestTemporaryError(api, request, megaError);
	pthread_mutex_unlock(&requestListenerMutex);

	pthread_mutex_lock(&listenerMutex);
	for(set<MegaListener *>::iterator it = listeners.begin(); it != listeners.end() ; it++)		
		(*it)->onRequestTemporaryError(api, request, megaError);
	pthread_mutex_unlock(&listenerMutex);

	MegaRequestListener* listener = request->getListener();
	if(listener) listener->onRequestTemporaryError(api, request, megaError);
	delete megaError;
}

void MegaApi::fireOnTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError e)
{
	MegaError *megaError = new MegaError(e);

	pthread_mutex_lock(&transferListenerMutex);
	for(set<MegaTransferListener *>::iterator it = transferListeners.begin(); it != transferListeners.end() ; it++)
		(*it)->onTransferFinish(api, transfer, megaError);
	pthread_mutex_unlock(&transferListenerMutex);

	pthread_mutex_lock(&listenerMutex);
	for(set<MegaListener *>::iterator it = listeners.begin(); it != listeners.end() ; it++)		
		(*it)->onTransferFinish(api, transfer, megaError);
	pthread_mutex_unlock(&listenerMutex);

	MegaTransferListener* listener = transfer->getListener();
	if(listener) listener->onTransferFinish(api, transfer, megaError);

	transferMap[transfer->getTag()]=NULL;
	delete transfer;
	delete megaError;
}

void MegaApi::fireOnTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError e)	
{
	MegaError *megaError = new MegaError(e);

	pthread_mutex_lock(&transferListenerMutex);
	for(set<MegaTransferListener *>::iterator it = transferListeners.begin(); it != transferListeners.end() ; it++)
		(*it)->onTransferTemporaryError(api, transfer, megaError);
	pthread_mutex_unlock(&transferListenerMutex);

	pthread_mutex_lock(&listenerMutex);
	for(set<MegaListener *>::iterator it = listeners.begin(); it != listeners.end() ; it++)		
		(*it)->onTransferTemporaryError(api, transfer, megaError);
	pthread_mutex_unlock(&listenerMutex);

	MegaTransferListener* listener = transfer->getListener();
	if(listener) listener->onTransferTemporaryError(api, transfer, megaError);
	delete megaError;
}

void MegaApi::fireOnTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
	pthread_mutex_lock(&transferListenerMutex);
	for(set<MegaTransferListener *>::iterator it = transferListeners.begin(); it != transferListeners.end() ; it++)
		(*it)->onTransferUpdate(api, transfer);
	pthread_mutex_unlock(&transferListenerMutex);

	pthread_mutex_lock(&listenerMutex);
	for(set<MegaListener *>::iterator it = listeners.begin(); it != listeners.end() ; it++)		
		(*it)->onTransferUpdate(api, transfer);	
	pthread_mutex_unlock(&listenerMutex);

	MegaTransferListener* listener = transfer->getListener();
	if(listener) listener->onTransferUpdate(api, transfer);	
}

void MegaApi::fireOnUsersUpdate(MegaApi* api, UserList *users)
{
	pthread_mutex_lock(&globalListenerMutex);
	for(set<MegaGlobalListener *>::iterator it = globalListeners.begin(); it != globalListeners.end() ; it++)
		(*it)->onUsersUpdate(api, users);
	pthread_mutex_unlock(&globalListenerMutex);

	pthread_mutex_lock(&listenerMutex);
	for(set<MegaListener *>::iterator it = listeners.begin(); it != listeners.end() ; it++)		
		(*it)->onUsersUpdate(api, users);
	pthread_mutex_unlock(&listenerMutex);
}

void MegaApi::fireOnNodesUpdate(MegaApi* api, NodeList *nodes)
{
	pthread_mutex_lock(&globalListenerMutex);
	for(set<MegaGlobalListener *>::iterator it = globalListeners.begin(); it != globalListeners.end() ; it++)
		(*it)->onNodesUpdate(api, nodes);
	pthread_mutex_unlock(&globalListenerMutex);

	pthread_mutex_lock(&listenerMutex);
	for(set<MegaListener *>::iterator it = listeners.begin(); it != listeners.end() ; it++)		
		(*it)->onNodesUpdate(api, nodes);
	pthread_mutex_unlock(&listenerMutex);
}

void MegaApi::fireOnReloadNeeded(MegaApi* api)
{
	pthread_mutex_lock(&globalListenerMutex);
	for(set<MegaGlobalListener *>::iterator it = globalListeners.begin(); it != globalListeners.end() ; it++)
		(*it)->onReloadNeeded(api);
	pthread_mutex_unlock(&globalListenerMutex);

	pthread_mutex_lock(&listenerMutex);
	for(set<MegaListener *>::iterator it = listeners.begin(); it != listeners.end() ; it++)		
		(*it)->onReloadNeeded(api);	
	pthread_mutex_unlock(&listenerMutex);
}


MegaError MegaApi::checkAccess(Node* node, const char *level)
{
	if(!node || !level)	return MegaError(API_EINTERNAL);

	pthread_mutex_lock(&fileSystemMutex);
	node = client->nodebyhandle(node->nodehandle);
	if(!node)
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return MegaError(API_EINTERNAL);
	}

	accesslevel a = OWNER;
	if(level == NULL) a = RDONLY;
	else if (!strcmp(level,"r") || !strcmp(level,"ro")) a = RDONLY;
	else if (!strcmp(level,"rw")) a = RDWR;
	else if (!strcmp(level,"full")) a = FULL;
	else if (!strcmp(level,"owner")) a = OWNER;
	MegaError e(client->checkaccess(node, a) ? API_OK : API_EACCESS);
	pthread_mutex_unlock(&fileSystemMutex);

	return e;
}

MegaError MegaApi::checkMove(Node* node, Node* target)
{
	if(!node || !target) return MegaError(API_EINTERNAL);

	pthread_mutex_lock(&fileSystemMutex);
	node = client->nodebyhandle(node->nodehandle);
	target = client->nodebyhandle(target->nodehandle);
	if(!node || !target)
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return MegaError(API_EINTERNAL);
	}

	MegaError e(client->checkmove(node,target));
	pthread_mutex_unlock(&fileSystemMutex);

	return e;
}

bool MegaApi::nodeComparatorDefaultASC (Node *i, Node *j)
{ 
	if(i->type < j->type) return 0;
	if(i->type > j->type) return 1;
	if(strcasecmp(i->displayname(), j->displayname())<=0) return 1;
	return 0;
};

bool MegaApi::nodeComparatorDefaultDESC (Node *i, Node *j)
{ 
	if(i->type < j->type) return 1;
	if(i->type > j->type) return 0;
	if(strcasecmp(i->displayname(), j->displayname())<=0) return 0;
	return 1;
};

bool MegaApi::nodeComparatorSizeASC (Node *i, Node *j)
{ if(i->size < j->size) return 1; return 0;}
bool MegaApi::nodeComparatorSizeDESC (Node *i, Node *j)
{ if(i->size < j->size) return 0; return 1;}

bool MegaApi::nodeComparatorCreationASC  (Node *i, Node *j)
{ if(i->ctime < j->ctime) return 1; return 0;}
bool MegaApi::nodeComparatorCreationDESC  (Node *i, Node *j)
{ if(i->ctime < j->ctime) return 0; return 1;}

bool MegaApi::nodeComparatorModificationASC  (Node *i, Node *j)
{ if(i->mtime < j->mtime) return 1; return 0;}
bool MegaApi::nodeComparatorModificationDESC  (Node *i, Node *j)	
{ if(i->mtime < j->mtime) return 0; return 1;}

bool MegaApi::nodeComparatorAlphabeticalASC  (Node *i, Node *j)
{ if(strcasecmp(i->displayname(), j->displayname())<=0) return 1; return 0; }
bool MegaApi::nodeComparatorAlphabeticalDESC  (Node *i, Node *j)	
{ if(strcasecmp(i->displayname(), j->displayname())<=0) return 0; return 1; }


NodeList *MegaApi::getChildren(Node* parent, int order)
{
	if(!parent) return new NodeList(NULL, 0, 0);

	pthread_mutex_lock(&fileSystemMutex);
	parent = client->nodebyhandle(parent->nodehandle);
	if(!parent)
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return new NodeList(NULL, 0, 0);
	}

	vector<Node *> childrenNodes;

	if(!order || order>ORDER_ALPHABETICAL_DESC)
	{
		for (node_list::iterator it = parent->children.begin(); it != parent->children.end(); )
			childrenNodes.push_back(*it++);
	}
	else
	{
		bool (*comp)(Node*, Node*);
		switch(order)
		{
		case ORDER_DEFAULT_ASC: comp = MegaApi::nodeComparatorDefaultASC; break;
		case ORDER_DEFAULT_DESC: comp = MegaApi::nodeComparatorDefaultDESC; break;
		case ORDER_SIZE_ASC: comp = MegaApi::nodeComparatorSizeASC; break;
		case ORDER_SIZE_DESC: comp = MegaApi::nodeComparatorSizeDESC; break;
		case ORDER_CREATION_ASC: comp = MegaApi::nodeComparatorCreationASC; break;
		case ORDER_CREATION_DESC: comp = MegaApi::nodeComparatorCreationDESC; break;
		case ORDER_MODIFICATION_ASC: comp = MegaApi::nodeComparatorModificationASC; break;
		case ORDER_MODIFICATION_DESC: comp = MegaApi::nodeComparatorModificationDESC; break;
		case ORDER_ALPHABETICAL_ASC: comp = MegaApi::nodeComparatorAlphabeticalASC; break;
		case ORDER_ALPHABETICAL_DESC: comp = MegaApi::nodeComparatorAlphabeticalDESC; break;
		default: comp = MegaApi::nodeComparatorDefaultASC; break;
		}

		for (node_list::iterator it = parent->children.begin(); it != parent->children.end(); )
		{
			Node *n = *it++;
			vector<Node *>::iterator i = std::lower_bound(childrenNodes.begin(),
					childrenNodes.end(), n, comp);
			childrenNodes.insert(i, n);
		}
	}
	pthread_mutex_unlock(&fileSystemMutex);
	return new NodeList(&(childrenNodes[0]), childrenNodes.size(), 1);
}


Node* MegaApi::getChildNode(Node *parent, const char* name)
{
	if(!parent || !name) return NULL;
	pthread_mutex_lock(&fileSystemMutex);
	parent = client->nodebyhandle(parent->nodehandle);
	if(!parent)
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return NULL;
	}

	Node *result = NULL;
	for (node_list::iterator it = parent->children.begin(); it != parent->children.end(); it++)
	{
		if (!strcmp(name,(*it)->displayname()))
		{
			result = *it;
			break;
		}
	}
	pthread_mutex_unlock(&fileSystemMutex);
	return result;
}

Node* MegaApi::getParentNode(Node* node)
{
	if(!node) return NULL;

	pthread_mutex_lock(&fileSystemMutex);
	node = client->nodebyhandle(node->nodehandle);
	if(!node)
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return NULL;
	}

	Node *result = client->nodebyhandle(node->parenthandle);
	pthread_mutex_unlock(&fileSystemMutex);

	return result;
}

const char* MegaApi::getNodePath(Node *n)
{
	if(!n) return NULL;

	pthread_mutex_lock(&fileSystemMutex);
	n = client->nodebyhandle(n->nodehandle);
	if(!n)
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return NULL;
	}

	string path;
	if (n->nodehandle == client->rootnodes[0])
	{
		path = "/";
		pthread_mutex_unlock(&fileSystemMutex);
		return stringToArray(path);
	}

	while (n)
	{
		switch (n->type)
		{
		case FOLDERNODE:
			path.insert(0,n->displayname());

			if (n->inshare)
			{
				path.insert(0,":");
				if (n->inshare->user) path.insert(0,n->inshare->user->email);
				else path.insert(0,"UNKNOWN");
				pthread_mutex_unlock(&fileSystemMutex);
				return stringToArray(path);
			}
			break;

		case INCOMINGNODE:
			path.insert(0,"//in");
			pthread_mutex_unlock(&fileSystemMutex);
			return stringToArray(path);

		case ROOTNODE:
			pthread_mutex_unlock(&fileSystemMutex);
			return stringToArray(path);

		case RUBBISHNODE:
			path.insert(0,"//bin");
			pthread_mutex_unlock(&fileSystemMutex);
			return stringToArray(path);

		case MAILNODE:
			path.insert(0,"//mail");
			pthread_mutex_unlock(&fileSystemMutex);
			return stringToArray(path);

		case TYPE_UNKNOWN:
		case FILENODE:
			path.insert(0,n->displayname());
		}

		path.insert(0,"/");

		n = getParentNode(n);
	}
	pthread_mutex_unlock(&fileSystemMutex);
	return stringToArray(path);
}

Node* MegaApi::getNodeByPath(const char *path, Node* cwd)
{
	pthread_mutex_lock(&fileSystemMutex);
	if(cwd) cwd = client->nodebyhandle(cwd->nodehandle);

	vector<string> c;
	string s;
	int l = 0;
	const char* bptr = path;
	int remote = 0;
	Node* n;
	Node* nn;

	// split path by / or :
	do {
		if (!l)
		{
			if (*path >= 0)
			{
				if (*path == '\\')
				{
					if (path > bptr) s.append(bptr,path-bptr);
					bptr = ++path;

					if (*bptr == 0)
					{
						c.push_back(s);
						break;
					}

					path++;
					continue;
				}

				if (*path == '/' || *path == ':' || !*path)
				{
					if (*path == ':')
					{
						if (c.size())
						{
							pthread_mutex_unlock(&fileSystemMutex);
							return NULL;
						}
						remote = 1;
					}

					if (path > bptr) s.append(bptr,path-bptr);

					bptr = path+1;

					c.push_back(s);

					s.erase();
				}
			}
			else if ((*path & 0xf0) == 0xe0) l = 1;
			else if ((*path & 0xf8) == 0xf0) l = 2;
			else if ((*path & 0xfc) == 0xf8) l = 3;
			else if ((*path & 0xfe) == 0xfc) l = 4;
		}
		else l--;
	} while (*path++);

	if (l)
	{
		pthread_mutex_unlock(&fileSystemMutex);
		return NULL;
	}

	if (remote)
	{
		// target: user inbox - record username/email and return NULL
		if (c.size() == 2 && !c[1].size())
		{
			//if (user) *user = c[0];
			pthread_mutex_unlock(&fileSystemMutex);
			return NULL;
		}

		User* u;

		if ((u = client->finduser(c[0].c_str())))
		{
			// locate matching share from this user
			handle_set::iterator sit;

			for (sit = u->sharing.begin(); sit != u->sharing.end(); sit++)
			{
				if ((n = client->nodebyhandle(*sit)))
				{
					l = 2;
					break;
				}

				if (l) break;
			}
		}

		if (!l)
		{
			pthread_mutex_unlock(&fileSystemMutex);
			return NULL;
		}
	}
	else
	{
		// path starting with /
		if (c.size() > 1 && !c[0].size())
		{
			// path starting with //
			if (c.size() > 2 && !c[1].size())
			{
				if (c[2] == "in") n = client->nodebyhandle(client->rootnodes[1]);
				else if (c[2] == "bin") n = client->nodebyhandle(client->rootnodes[2]);
				else if (c[2] == "mail") n = client->nodebyhandle(client->rootnodes[3]);
				else
				{
					pthread_mutex_unlock(&fileSystemMutex);
					return NULL;
				}

				l = 3;
			}
			else
			{
				n = client->nodebyhandle(client->rootnodes[0]);
				l = 1;
			}
		}
		else n = cwd;
	}

	// parse relative path
	while (n && l < (int)c.size())
	{
		if (c[l] != ".")
		{
			if (c[l] == "..")
			{
				if (getParentNode(n)) n = getParentNode(n);
			}
			else
			{
				// locate child node (explicit ambiguity resolution: not implemented)
				if (c[l].size())
				{
					nn = getChildNode(n,c[l].c_str());

					if (!nn)
					{
						pthread_mutex_unlock(&fileSystemMutex);
						return NULL;
					}

					n = nn;
				}
			}
		}

		l++;
	}
	pthread_mutex_unlock(&fileSystemMutex);
	return n;	
}

Node* MegaApi::getNodeByHandle(handle handle)
{
	if(handle == UNDEF) return NULL;
	pthread_mutex_lock(&fileSystemMutex);
	Node *result = client->nodebyhandle(handle);
	pthread_mutex_unlock(&fileSystemMutex);
	return result;
}

void MegaApi::setDebug(bool debug) { curl->setDebug(debug); }
bool MegaApi::getDebug() { return curl->getDebug(); }

StringList *MegaApi::getRootNodeNames() { return rootNodeNames; }
StringList *MegaApi::getRootNodePaths() { return rootNodePaths; }

const char* MegaApi::rootnodenames[] = { "ROOT", "INBOX", "RUBBISH", "MAIL" };
const char* MegaApi::rootnodepaths[] = { "/", "//in", "//bin", "//mail" };
StringList * MegaApi::rootNodeNames = new StringList(rootnodenames, 4);
StringList * MegaApi::rootNodePaths = new StringList(rootnodepaths, 4);


void MegaApi::sendPendingTransfers()
{
	MegaTransfer *transfer;
	error e;
	int nextTag;
	while((transfer = transferQueue.pop()))
	{
		nextTag = client->nextreqtag();
		transferMap[nextTag]=transfer;
		transfer->setTag(nextTag);
		e = API_OK;

		switch(transfer->getType())
		{
			case MegaTransfer::TYPE_UPLOAD:
			{
				const char* localPath = transfer->getPath();
				if(!localPath) { e = API_EARGS; break; }

				int td = client->topen(localPath);
				if(td < 0) { e = (error)td; break; }
				transfer->setSlot(td);
				break;
			}
			case MegaTransfer::TYPE_DOWNLOAD:
			{
				handle nodehandle = transfer->getNodeHandle();
				const char* base64key = transfer->getBase64Key();

				if(!transfer->getPath() && !transfer->getParentPath()) { e = API_EARGS; break; }

				int td;
				if(base64key)
				{
					byte key[Node::FILENODEKEYLENGTH];
					Base64::atob(base64key,key,sizeof key);
					td = client->topen(nodehandle, (const byte*)key);
				}
				else
				{
					td = client->topen(nodehandle, NULL);
				}
				if(td < 0) { e = (error)td; break; }
				transfer->setSlot(td);
				break;
			}
		}

		if(e)
		{
			client->restag = nextTag;
			fireOnTransferFinish(this, transfer, MegaError(e));
		}
	}
}


void MegaApi::sendPendingRequests()
{
	MegaRequest *request;
	error e;
	int nextTag;

	while((request = requestQueue.pop()))
	{
		nextTag = client->nextreqtag();
		requestMap[nextTag]=request;
		e = API_OK;

		fireOnRequestStart(this, request);
		switch(request->getType())
		{
		case MegaRequest::TYPE_LOGIN:
		{
			const char *login = request->getEmail();
			const char *password = request->getPassword();
			if(!login || !password) { e = API_EARGS; break; }

			byte pwkey[SymmCipher::KEYLENGTH];
			if((e = client->pw_key(password,pwkey))) break;
			client->login(login, pwkey);

			if(updatingSID) return;
			break;
		}
		case MegaRequest::TYPE_MKDIR:
		{
			Node *parent = client->nodebyhandle(request->getParentHandle());
			const char *name = request->getName();
			if(!name || !parent) { e = API_EARGS; break; }

			NewNode *newnode = new NewNode[1];
			SymmCipher key;
			string attrstring;
			byte buf[Node::FOLDERNODEKEYLENGTH];

			// set up new node as folder node
			newnode->source = NEW_NODE;
			newnode->type = FOLDERNODE;
			newnode->nodehandle = 0;
			newnode->mtime = newnode->ctime = time(NULL);
			newnode->parenthandle = UNDEF;

			// generate fresh random key for this folder node
			PrnGen::genblock(buf,Node::FOLDERNODEKEYLENGTH);
			newnode->nodekey.assign((char*)buf,Node::FOLDERNODEKEYLENGTH);
			key.setkey(buf);

			// generate fresh attribute object with the folder name
			AttrMap attrs;
			attrs.map['n'] = name;

			// JSON-encode object and encrypt attribute string
			attrs.getjson(&attrstring);
			client->makeattr(&key,&newnode->attrstring,attrstring.c_str());

			// add the newly generated folder node
			client->putnodes(parent->nodehandle,newnode,1);
			break;
		}
		case MegaRequest::TYPE_MOVE:
		{
			Node *node = client->nodebyhandle(request->getNodeHandle());
			Node *newParent = client->nodebyhandle(request->getParentHandle());
			if(!node || !newParent) { e = API_EARGS; break; }

			if(node->parent == newParent) fireOnRequestFinish(this, request, MegaError(API_OK));
			if((e = client->checkmove(node,newParent))) break;

			e = client->rename(node, newParent);
			break;
		}
		case MegaRequest::TYPE_COPY:
		{
			Node *node = client->nodebyhandle(request->getNodeHandle());
			Node *target = client->nodebyhandle(request->getParentHandle());
			if(!node || !target) { e = API_EARGS; break; }

			unsigned nc;
			TreeProcCopy tc;
			// determine number of nodes to be copied
			client->proctree(node,&tc);
			tc.allocnodes();
			nc = tc.nc;
			// build new nodes array
			client->proctree(node,&tc);
			if (!nc) { e = API_EARGS; break; }

			tc.nn->parenthandle = UNDEF;

			client->putnodes(target->nodehandle,tc.nn,nc);
			break;
		}
		case MegaRequest::TYPE_RENAME:
		{
			Node* node = client->nodebyhandle(request->getNodeHandle());
			const char* newName = request->getName();
			if(!node || !newName) { e = API_EARGS; break; }

			if (!client->checkaccess(node,FULL)) { e = API_EACCESS; break; }
			node->attrs.map['n'] = string(newName);
			e = client->setattr(node);
			break;
		}
		case MegaRequest::TYPE_REMOVE:
		{
			Node* node = client->nodebyhandle(request->getNodeHandle());
			if(!node) { e = API_EARGS; break; }

			if (!client->checkaccess(node,FULL)) { e = API_EACCESS; break; }
			e = client->unlink(node);
			break;
		}
		case MegaRequest::TYPE_SHARE:
		{
			Node *node = client->nodebyhandle(request->getNodeHandle());
			const char* email = request->getEmail();
			const char* access = request->getAccess();
			if(!node || !email || !access) { e = API_EARGS; break; }

			accesslevel a = ACCESS_UNKNOWN;
			if(access == NULL) a = RDONLY;
			else if (!strcmp(access,"r") || !strcmp(access,"ro")) a = RDONLY;
			else if (!strcmp(access,"rw")) a = RDWR;
			else if (!strcmp(access,"full")) a = FULL;
			else { e = API_EARGS; break; }
			client->setshare(node, email, a);
			break;
		}
		case MegaRequest::TYPE_FOLDER_ACCESS:
		{
			const char* megaFolderLink = request->getLink();
			if(!megaFolderLink) { e = API_EARGS; break; }

			const char* ptr;
			if (!((ptr = strstr(megaFolderLink,"#F!")) && (strlen(ptr)>12) && ptr[11] == '!'))
			{ e = API_EARGS; break; }
			if((e = client->folderaccess(ptr+3,ptr+12))) break;
			client->fetchnodes();
			break;
		}
		case MegaRequest::TYPE_IMPORT_LINK:
		case MegaRequest::TYPE_GET_PUBLIC_NODE:
		{
			Node *node = client->nodebyhandle(request->getParentHandle());
			const char* megaFileLink = request->getLink();
			if(!megaFileLink) { e = API_EARGS; break; }
			if((request->getType()==MegaRequest::TYPE_IMPORT_LINK) && (!node)) { e = API_EARGS; break; }

			e = client->openfilelink(megaFileLink);
			break;
		}
		case MegaRequest::TYPE_IMPORT_NODE:
		{
			NewNode *newnode = (NewNode *)request->getPublicNode();
			Node *parent = client->nodebyhandle(request->getParentHandle());

			if(!newnode || !parent) { e = API_EARGS; break; }

			// add node
			client->putnodes(parent->nodehandle,newnode,1);

			break;
		}
		case MegaRequest::TYPE_EXPORT:
		{
			cout << "Export tag: " << nextTag << endl;
			Node* node = client->nodebyhandle(request->getNodeHandle());
			if(!node) { e = API_EARGS; break; }

			e = client->exportnode(node, 0);
			break;
		}
		case MegaRequest::TYPE_FETCH_NODES:
		{
			client->fetchnodes();
			break;
		}
		case MegaRequest::TYPE_ACCOUNT_DETAILS:
		{
			int numDetails = request->getNumDetails();
			int storage = numDetails & 0x01;
			int transfer = numDetails & 0x02;
			int pro = numDetails & 0x04;
			int transactions = numDetails & 0x08;
			int purchases = numDetails & 0x10;
			int sessions =  numDetails & 0x20;

			numDetails = 1;
			if(transactions) numDetails++;
			if(purchases) numDetails++;
			if(sessions) numDetails++;

			request->setNumDetails(numDetails);

			client->getaccountdetails(request->getAccountDetails(),storage,transfer,pro,transactions,purchases,sessions);
			break;
		}
		case MegaRequest::TYPE_CHANGE_PW:
		{
			const char* oldPassword = request->getPassword();
			const char* newPassword = request->getNewPassword();
			if(!oldPassword || !newPassword) { e = API_EARGS; break; }

			byte pwkey[SymmCipher::KEYLENGTH];
			byte newpwkey[SymmCipher::KEYLENGTH];
			if((e = client->pw_key(oldPassword, pwkey))) { e = API_EARGS; break; }
			if((e = client->pw_key(newPassword, newpwkey))) { e = API_EARGS; break; }
			e = client->changepw(pwkey, newpwkey);
			break;
		}
		case MegaRequest::TYPE_LOGOUT:
		{
			if(loginRequest) delete loginRequest;
			loginRequest = NULL;

			for (std::map<int,MegaRequest*>::iterator it=requestMap.begin(); it!=requestMap.end(); ++it)
			{
				if(it->first != nextTag)
				{
					client->restag = it->first;
					if(it->second) fireOnRequestFinish(this, it->second, MegaError(MegaError::API_EACCESS));
				}
			}

			for (std::map<int,MegaTransfer*>::iterator it=transferMap.begin(); it!=transferMap.end(); ++it)
			{
				client->restag = it->first;
				if(it->second) fireOnTransferFinish(this, it->second, MegaError(MegaError::API_EACCESS));
			}

			client->logout();

			client->restag = nextTag;
			fireOnRequestFinish(this, request, MegaError(e));
			break;
		}
		case MegaRequest::TYPE_FAST_LOGIN:
		{
			const char* email = request->getEmail();
			const char* stringHash = request->getPassword();
			const char* base64pwkey = request->getPrivateKey();
			if(!email || !base64pwkey || !stringHash) { e = API_EARGS; break; }

			byte pwkey[SymmCipher::KEYLENGTH];
			Base64::atob(base64pwkey, (byte *)pwkey, sizeof pwkey);

			byte strhash[SymmCipher::KEYLENGTH];
			Base64::atob(stringHash, (byte *)strhash, sizeof strhash);

			client->key.setkey((byte*)pwkey);
			client->reqs[client->r].add(new CommandLogin(client,email,*(uint64_t*)strhash));

			if(updatingSID) return;
			break;
		}
		case MegaRequest::TYPE_GET_ATTR_FILE:
		{
			const char* dstFilePath = request->getFile();
			int type = request->getAttrType();
			Node *node = client->nodebyhandle(request->getNodeHandle());

			if(!dstFilePath || !node) { e = API_EARGS; break; }

			e = client->getfa(node, type);
			break;
		}
		case MegaRequest::TYPE_SET_ATTR_FILE:
		{
			const char* srcFilePath = request->getFile();
			int type = request->getAttrType();
			Node *node = client->nodebyhandle(request->getNodeHandle());

			if(!srcFilePath || !node) { e = API_EARGS; break; }

			string thumbnail;
			FileAccess *f = this->newfile();
			f->fopen(srcFilePath, 1, 0);
			f->fread(&thumbnail, f->size, 0, 0);
			delete f;

			client->putfa(&(node->key),node->nodehandle,type,(const byte*)thumbnail.data(),thumbnail.size());
			break;
		}
		case MegaRequest::TYPE_RETRY_PENDING_CONNECTIONS:
		{
			client->abortbackoff();
			client->disconnect();
			break;
		}
		case MegaRequest::TYPE_ADD_CONTACT:
		{
			const char *email = request->getEmail();
			if(!email) { e = API_EARGS; break; }
			client->invite(email, VISIBLE);
			break;
		}
		case MegaRequest::TYPE_CREATE_ACCOUNT:
		case MegaRequest::TYPE_FAST_CREATE_ACCOUNT:
		{
			const char *email = request->getEmail();
			const char *password = request->getPassword();
			const char *name = request->getName();

			if(!email || !password || !name) { e = API_EARGS; break; }

            cout << "Create Ephemeral Start" <<  endl;
			client->createephemeral();
			break;
		}
		case MegaRequest::TYPE_QUERY_SIGNUP_LINK:
		case MegaRequest::TYPE_CONFIRM_ACCOUNT:
		case MegaRequest::TYPE_FAST_CONFIRM_ACCOUNT:
		{
			const char *link = request->getLink();
			const char *password = request->getPassword();
			if(((request->getType()!=MegaRequest::TYPE_QUERY_SIGNUP_LINK) && !password) || (!link))
				{ e = API_EARGS; break; }

			const char* ptr = link;
			const char* tptr;

			if ((tptr = strstr(ptr,"#confirm"))) ptr = tptr+8;

			unsigned len = (strlen(link)-(ptr-link))*3/4+4;
			byte *c = new byte[len];
			len = Base64::atob(ptr,c,len)-c;
			client->querysignuplink(c,len);
			delete[] c;
			break;
		}
		}

		if(e)
		{
			client->restag = nextTag;
			fireOnRequestFinish(this, request, MegaError(e));
		}
	}
}

char* MegaApi::stringToArray(string &buffer)
{	
	char *newbuffer = new char[buffer.size()+1];
	buffer.copy(newbuffer, buffer.size());
	newbuffer[buffer.size()]='\0';
	return newbuffer;
}

char* MegaApi::strdup(const char* buffer)
{	
	if(!buffer) return NULL;
	int tam = strlen(buffer)+1;
	char *newbuffer = new char[tam];
	memcpy(newbuffer, buffer, tam);
	return newbuffer;
}

TreeProcCopy::TreeProcCopy()
{
	nn = NULL;
	nc = 0;
}

void TreeProcCopy::allocnodes()
{
	nn = new NewNode[nc];
}

TreeProcCopy::~TreeProcCopy()
{
	delete[] nn;
}

// determine node tree size (nn = NULL) or write node tree to new nodes array
void TreeProcCopy::proc(MegaClient* client, Node* n)
{
	if (nn)
	{
		string attrstring;
		SymmCipher key;
		NewNode* t = nn+--nc;

		// copy node
		t->source = NEW_NODE;
		t->type = n->type;
		t->nodehandle = n->nodehandle;
		t->parenthandle = n->parenthandle;
		t->mtime = n->mtime;
		t->ctime = n->ctime;

		// copy key (if file) or generate new key (if folder)
		if (n->type == FILENODE) t->nodekey = n->nodekey;
		else
		{
			byte buf[Node::FOLDERNODEKEYLENGTH];
			PrnGen::genblock(buf,sizeof buf);
			t->nodekey.assign((char*)buf,Node::FOLDERNODEKEYLENGTH);
		}

		key.setkey((const byte*)t->nodekey.data(),n->type);

		n->attrs.getjson(&attrstring);
		client->makeattr(&key,&t->attrstring,attrstring.c_str());
	}
	else nc++;
}

