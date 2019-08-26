#include "Preferences.h"
#include "platform/Platform.h"

#include <QDesktopServices>
#include <assert.h>

using namespace mega;

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

const char Preferences::CLIENT_KEY[] = "FhMgXbqb";
const char Preferences::USER_AGENT[] = "MEGAsync/4.2.4.0";
const int Preferences::VERSION_CODE = 4204;
const int Preferences::BUILD_ID = 0;
// Do not change the location of VERSION_STRING, create_tarball.sh parses this file
const QString Preferences::VERSION_STRING = QString::fromAscii("4.2.4");
QString Preferences::SDK_ID = QString::fromAscii("7cc8508");
const QString Preferences::CHANGELOG = QString::fromUtf8(QT_TR_NOOP(
    "- Fix a crash during processing of some PDF files\n"
    "- Resume pending transfers after a crash on next startup\n"
    "- Include option to add synchronizations from the main dialog\n"
    "- Other minor bug fixes and improvements"));

const QString Preferences::TRANSLATION_FOLDER = QString::fromAscii("://translations/");
const QString Preferences::TRANSLATION_PREFIX = QString::fromAscii("MEGASyncStrings_");

int Preferences::STATE_REFRESH_INTERVAL_MS        = 10000;
int Preferences::FINISHED_TRANSFER_REFRESH_INTERVAL_MS        = 10000;

int Preferences::MAX_FIRST_SYNC_DELAY_S = 120; // Max delay time to wait for local paths before trying to restore syncs
int Preferences::MIN_FIRST_SYNC_DELAY_S = 40; // Min delay time to wait for local paths before trying to restore syncs

long long Preferences::OQ_DIALOG_INTERVAL_MS = 604800000; // 7 days
long long Preferences::OQ_NOTIFICATION_INTERVAL_MS = 129600000; // 36 hours
long long Preferences::ALMOST_OS_INTERVAL_MS = 259200000; // 72 hours
long long Preferences::OS_INTERVAL_MS = 129600000; // 36 hours
long long Preferences::USER_INACTIVITY_MS = 20000; // 20 secs

long long Preferences::MIN_UPDATE_STATS_INTERVAL  = 300000;
long long Preferences::MIN_UPDATE_CLEANING_INTERVAL_MS  = 7200000;
long long Preferences::MIN_UPDATE_NOTIFICATION_INTERVAL_MS    = 172800000;
long long Preferences::MIN_REBOOT_INTERVAL_MS                 = 300000;
long long Preferences::MIN_EXTERNAL_NODES_WARNING_MS          = 60000;
long long Preferences::MIN_TRANSFER_NOTIFICATION_INTERVAL_MS  = 10000;

unsigned int Preferences::UPDATE_INITIAL_DELAY_SECS           = 60;
unsigned int Preferences::UPDATE_RETRY_INTERVAL_SECS          = 7200;
unsigned int Preferences::UPDATE_TIMEOUT_SECS                 = 600;
unsigned int Preferences::MAX_LOGIN_TIME_MS                   = 40000;
unsigned int Preferences::PROXY_TEST_TIMEOUT_MS               = 10000;
unsigned int Preferences::MAX_IDLE_TIME_MS                    = 600000;
unsigned int Preferences::MAX_COMPLETED_ITEMS                 = 1000;

const qint16 Preferences::HTTP_PORT  = 6341;
const qint16 Preferences::HTTPS_PORT = 6342;

const QString Preferences::defaultHttpsKey = QString::fromUtf8(
            "-----BEGIN RSA PRIVATE KEY-----\n"
            "MIIEowIBAAKCAQEAsZPQVLJIMtnRzbnb686rI9+teFJ4l4wMN7JDD/sVFCwYGk3B\n"
            "oEJGETDOiO66P/UaKaucqSuafHJlKk2Ibv/WeET/2rBwyDAu5SKiL4KGV0FnSRwI\n"
            "SfSgYJ2Ivo0DdEOiEAWnfVkf3SjHLpZzdKDMp/NqzvaMFeO3WFc3UJzxNZNc9R6s\n"
            "ysqqwYx7xn0kMjANmArkpdStxM0GPDRWF3rt2Y8BQj77g5kaS6pICga78Bz7tM2k\n"
            "De1pYjQc/vCZwbej1DTb02dMv4mFEU95OEh5YYv8V3VfY6owDeN90TCKxiFqRZwa\n"
            "CaWkxW07FN8+jY+QVP02EKWgTonok3zttWnJewIDAQABAoIBAHjzMuzdWau5DNvR\n"
            "AegFJjATA/Z2RckDDLdK5b6Wx+6rcedImt9rBIpk8ef2okk5yo4jY1seZeTpjdpY\n"
            "YNspyoy0gogTsV3I48gR5OHzR0lbNNxpCYRF6I6l/iO7rGSeRF4hW8lW0Qsz4Sbs\n"
            "NstzUrH6a/93kka+iXzUOsrferN//owiNORT/t5UeATIgSKh8BFFyXhFNbaUd96o\n"
            "INdIotfDYKkHC4q9iP8Wk+58wr5AZW8WWCxLTOXVi9Se/agjkv1pQ7xUo5c4dAZJ\n"
            "AITWMu0goYzxjrtK/5NOwNnnPSsTtPe9QdLYqeDvsPDoxw2b1fNEKQlxinEHxLXf\n"
            "v5gUnIECgYEA5ZkPT2Dm/BkgtmLbYF8iJIrAqm1GpiLKIxS+9FNMJ5MoMpEF05MY\n"
            "h2D7TFYOFssi3xe6D6p9A9GDmBHfnkRWIh3a/ggOCKKbFmeSOB6mLa28LWqTEUDx\n"
            "UczSDDXTKRZXwRkVfWSkELVx1eOOcJ99iglGVTtugOmGL5s5qO0EeVsCgYEAxf9d\n"
            "wb0z5ZvpNJtoYXlFqYkKh62Fv6AGoqyu4lZk+EJZ9WDglL7de5g0cV49ezRCkeLl\n"
            "XA4secq2xbgpaXg2WZKtf9hyAeWw/dbkFlc5WJwsrWmLrH6t1yn3MBvxiLepot5b\n"
            "LWVeF3RBcY3M3VrexHlsAAK/g32MOqQGK7jCymECgYA2LO+X412djrBRWPI6wzfj\n"
            "N3pOvIo3GW5o6LRGIoGNtQN/Kwkc61CWYSPq10NrzXLlAUw99hSZmg5bC5ygU6+f\n"
            "TTwnRO9KylOd7cJqy5CapqwIa2VYZOKMUrAdgXXZ9BOw1KJlfCHab3X06kTp/jg5\n"
            "IgcehW+Fq5SmFxJ78bFkkQKBgFdWrxfgCv10yekfyGp+4UDIp7Fz1LTTtTma7MYA\n"
            "kCeGtUHtSwOyMebsG+4KQRqQ0fCiYsQ5qFrNFdSfyPyyP19BuXDUnG3D0VAXs6M7\n"
            "NJumwHp6PVeIxcxMwbIiEWhag+TJKvUaaufzq06Y206lX/DREtfqXmp1Ph877VDp\n"
            "/lrhAoGBAMY/vn5PAPnYpoMDNas4dHIU1qmcoE0DbzACHddZ0VSagtp1Trdb0pzO\n"
            "BwQXfuqsRunijscxuFcmCbh3dfAE/EaZTKzJRZiCab10aBSIKDz0u6lJ2XIxInT5\n"
            "3VxuNvsWw/Hg7B5xuCzBwffifcS8AMbPjixg/pI84fh5KZM2Eg6e\n"
            "-----END RSA PRIVATE KEY-----\n");

const QString Preferences::defaultHttpsCert = QString::fromUtf8(
            "-----BEGIN CERTIFICATE-----\n"
            "MIIGAjCCBOqgAwIBAgIRAMN6iHtOgy68QBu3kXiaFc8wDQYJKoZIhvcNAQELBQAw\n"
            "gZYxCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\n"
            "BgNVBAcTB1NhbGZvcmQxGjAYBgNVBAoTEUNPTU9ETyBDQSBMaW1pdGVkMTwwOgYD\n"
            "VQQDEzNDT01PRE8gUlNBIE9yZ2FuaXphdGlvbiBWYWxpZGF0aW9uIFNlY3VyZSBT\n"
            "ZXJ2ZXIgQ0EwHhcNMTgwMzE0MDAwMDAwWhcNMjAwNDA5MjM1OTU5WjCByTELMAkG\n"
            "A1UEBhMCTloxDTALBgNVBBETBDEwMTAxETAPBgNVBAgTCEF1Y2tsYW5kMREwDwYD\n"
            "VQQHEwhBdWNrbGFuZDEoMCYGA1UECRMfMTUsIFB3YyBUb3dlciwgMTg4IFF1YXkg\n"
            "U3RyZWV0LDEVMBMGA1UEChMMTWVnYSBMaW1pdGVkMRcwFQYDVQQLEw5JbnN0YW50\n"
            "U1NMIFBybzErMCkGA1UEAxMibG9jYWxob3N0Lm1lZ2FzeW5jbG9vcGJhY2subWVn\n"
            "YS5uejCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALGT0FSySDLZ0c25\n"
            "2+vOqyPfrXhSeJeMDDeyQw/7FRQsGBpNwaBCRhEwzojuuj/1GimrnKkrmnxyZSpN\n"
            "iG7/1nhE/9qwcMgwLuUioi+ChldBZ0kcCEn0oGCdiL6NA3RDohAFp31ZH90oxy6W\n"
            "c3SgzKfzas72jBXjt1hXN1Cc8TWTXPUerMrKqsGMe8Z9JDIwDZgK5KXUrcTNBjw0\n"
            "Vhd67dmPAUI++4OZGkuqSAoGu/Ac+7TNpA3taWI0HP7wmcG3o9Q029NnTL+JhRFP\n"
            "eThIeWGL/Fd1X2OqMA3jfdEwisYhakWcGgmlpMVtOxTfPo2PkFT9NhCloE6J6JN8\n"
            "7bVpyXsCAwEAAaOCAhQwggIQMB8GA1UdIwQYMBaAFJrzK9rPrU+2L7sqSEgqErcb\n"
            "QsEkMB0GA1UdDgQWBBRHlrC2tJsrpgC6c0vMTwVfEcEJRTAOBgNVHQ8BAf8EBAMC\n"
            "BaAwDAYDVR0TAQH/BAIwADAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIw\n"
            "UAYDVR0gBEkwRzA7BgwrBgEEAbIxAQIBAwQwKzApBggrBgEFBQcCARYdaHR0cHM6\n"
            "Ly9zZWN1cmUuY29tb2RvLmNvbS9DUFMwCAYGZ4EMAQICMFoGA1UdHwRTMFEwT6BN\n"
            "oEuGSWh0dHA6Ly9jcmwuY29tb2RvY2EuY29tL0NPTU9ET1JTQU9yZ2FuaXphdGlv\n"
            "blZhbGlkYXRpb25TZWN1cmVTZXJ2ZXJDQS5jcmwwgYsGCCsGAQUFBwEBBH8wfTBV\n"
            "BggrBgEFBQcwAoZJaHR0cDovL2NydC5jb21vZG9jYS5jb20vQ09NT0RPUlNBT3Jn\n"
            "YW5pemF0aW9uVmFsaWRhdGlvblNlY3VyZVNlcnZlckNBLmNydDAkBggrBgEFBQcw\n"
            "AYYYaHR0cDovL29jc3AuY29tb2RvY2EuY29tMFUGA1UdEQROMEyCImxvY2FsaG9z\n"
            "dC5tZWdhc3luY2xvb3BiYWNrLm1lZ2EubnqCJnd3dy5sb2NhbGhvc3QubWVnYXN5\n"
            "bmNsb29wYmFjay5tZWdhLm56MA0GCSqGSIb3DQEBCwUAA4IBAQCgT+MFT4W3lQgS\n"
            "pTFrpQhsGUaCtJt9JNva6PZGBinSlU7turwAS+mVteM2Q/Rh6A37hjouVaP8WRSJ\n"
            "rpP5HriACMtWZeUTmRky4gJX08aW2X8QcihrpluBA3AirT9cu0FZzllO3jAYXUI1\n"
            "pPL2WOGxYAuOOLCoEn0C4nn2Wzk39vpcaedVvnU8tqWpHTwQgQ3d2dTTxlddAGGU\n"
            "H6P7d2r7l5Oam7q17L5yjAvQ50gkIJ6QQpOmat7OqnS2FiLl0LNdlWv8OF+WDguW\n"
            "ANWn96E9StPMkIqw4Uc6O+QPbaRqDUddZNcBiQeAZPzAAXk2a0RmOxMGRU6/v6vg\n"
            "WJ1PfDrK\n"
            "-----END CERTIFICATE-----\n");

const QString Preferences::defaultHttpsCertIntermediate = QString::fromUtf8(            
            "-----BEGIN CERTIFICATE-----\n"
            "MIIFdDCCBFygAwIBAgIQJ2buVutJ846r13Ci/ITeIjANBgkqhkiG9w0BAQwFADBv\n"
            "MQswCQYDVQQGEwJTRTEUMBIGA1UEChMLQWRkVHJ1c3QgQUIxJjAkBgNVBAsTHUFk\n"
            "ZFRydXN0IEV4dGVybmFsIFRUUCBOZXR3b3JrMSIwIAYDVQQDExlBZGRUcnVzdCBF\n"
            "eHRlcm5hbCBDQSBSb290MB4XDTAwMDUzMDEwNDgzOFoXDTIwMDUzMDEwNDgzOFow\n"
            "gYUxCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\n"
            "BgNVBAcTB1NhbGZvcmQxGjAYBgNVBAoTEUNPTU9ETyBDQSBMaW1pdGVkMSswKQYD\n"
            "VQQDEyJDT01PRE8gUlNBIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MIICIjANBgkq\n"
            "hkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAkehUktIKVrGsDSTdxc9EZ3SZKzejfSNw\n"
            "AHG8U9/E+ioSj0t/EFa9n3Byt2F/yUsPF6c947AEYe7/EZfH9IY+Cvo+XPmT5jR6\n"
            "2RRr55yzhaCCenavcZDX7P0N+pxs+t+wgvQUfvm+xKYvT3+Zf7X8Z0NyvQwA1onr\n"
            "ayzT7Y+YHBSrfuXjbvzYqOSSJNpDa2K4Vf3qwbxstovzDo2a5JtsaZn4eEgwRdWt\n"
            "4Q08RWD8MpZRJ7xnw8outmvqRsfHIKCxH2XeSAi6pE6p8oNGN4Tr6MyBSENnTnIq\n"
            "m1y9TBsoilwie7SrmNnu4FGDwwlGTm0+mfqVF9p8M1dBPI1R7Qu2XK8sYxrfV8g/\n"
            "vOldxJuvRZnio1oktLqpVj3Pb6r/SVi+8Kj/9Lit6Tf7urj0Czr56ENCHonYhMsT\n"
            "8dm74YlguIwoVqwUHZwK53Hrzw7dPamWoUi9PPevtQ0iTMARgexWO/bTouJbt7IE\n"
            "IlKVgJNp6I5MZfGRAy1wdALqi2cVKWlSArvX31BqVUa/oKMoYX9w0MOiqiwhqkfO\n"
            "KJwGRXa/ghgntNWutMtQ5mv0TIZxMOmm3xaG4Nj/QN370EKIf6MzOi5cHkERgWPO\n"
            "GHFrK+ymircxXDpqR+DDeVnWIBqv8mqYqnK8V0rSS527EPywTEHl7R09XiidnMy/\n"
            "s1Hap0flhFMCAwEAAaOB9DCB8TAfBgNVHSMEGDAWgBStvZh6NLQm9/rEJlTvA73g\n"
            "JMtUGjAdBgNVHQ4EFgQUu69+Aj36pvE8hI6t7jiY7NkyMtQwDgYDVR0PAQH/BAQD\n"
            "AgGGMA8GA1UdEwEB/wQFMAMBAf8wEQYDVR0gBAowCDAGBgRVHSAAMEQGA1UdHwQ9\n"
            "MDswOaA3oDWGM2h0dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9BZGRUcnVzdEV4dGVy\n"
            "bmFsQ0FSb290LmNybDA1BggrBgEFBQcBAQQpMCcwJQYIKwYBBQUHMAGGGWh0dHA6\n"
            "Ly9vY3NwLnVzZXJ0cnVzdC5jb20wDQYJKoZIhvcNAQEMBQADggEBAGS/g/FfmoXQ\n"
            "zbihKVcN6Fr30ek+8nYEbvFScLsePP9NDXRqzIGCJdPDoCpdTPW6i6FtxFQJdcfj\n"
            "Jw5dhHk3QBN39bSsHNA7qxcS1u80GH4r6XnTq1dFDK8o+tDb5VCViLvfhVdpfZLY\n"
            "Uspzgb8c8+a4bmYRBbMelC1/kZWSWfFMzqORcUx8Rww7Cxn2obFshj5cqsQugsv5\n"
            "B5a6SE2Q8pTIqXOi6wZ7I53eovNNVZ96YUWYGGjHXkBrI/V5eu+MtWuLt29G9Hvx\n"
            "PUsE2JOAWVrgQSQdso8VYFhH2+9uRv0V9dlfmrPb2LjkQLPNlzmuhbsdjrzch5vR\n"
            "pu/xO28QOG8=\n"
            "-----END CERTIFICATE----- \n"
            ";"
            "-----BEGIN CERTIFICATE-----\n"
            "MIIGDjCCA/agAwIBAgIQNoJef7WkgZN+9tFza7k8pjANBgkqhkiG9w0BAQwFADCB\n"
            "hTELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4G\n"
            "A1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNV\n"
            "BAMTIkNPTU9ETyBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTQwMjEy\n"
            "MDAwMDAwWhcNMjkwMjExMjM1OTU5WjCBljELMAkGA1UEBhMCR0IxGzAZBgNVBAgT\n"
            "EkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMR\n"
            "Q09NT0RPIENBIExpbWl0ZWQxPDA6BgNVBAMTM0NPTU9ETyBSU0EgT3JnYW5pemF0\n"
            "aW9uIFZhbGlkYXRpb24gU2VjdXJlIFNlcnZlciBDQTCCASIwDQYJKoZIhvcNAQEB\n"
            "BQADggEPADCCAQoCggEBALkU2YXyQURX/zBEHtw8RKMXuG4B+KNfwqkhHc5Z9Ozz\n"
            "iKkJMjyxi2OkPic284/5OGYuB5dBj0um3cNfnnM858ogDU98MgXPwS5IZUqF0B9W\n"
            "MW2O5cYy1Bu8n32W/JjXT/j0WFb440W+kRiC5Iq+r81SN1GHTx6Xweg6rvn/RuRl\n"
            "Pz/DR4MvzLhCXi1+91porl1LwKY1IfWGo8hJi5hjYA3JIUjCkjBlRrKGNQRCJX6t\n"
            "p05LEkAAeohoXG+fo6R4ESGuPQsOvkUUI8/rddf2oPG8RWxevKEy7PNYeEIoCzoB\n"
            "dvDFoJ7BaXDej0umed/ydrbjDxN8GDuxUWxqIDnOnmkCAwEAAaOCAWUwggFhMB8G\n"
            "A1UdIwQYMBaAFLuvfgI9+qbxPISOre44mOzZMjLUMB0GA1UdDgQWBBSa8yvaz61P\n"
            "ti+7KkhIKhK3G0LBJDAOBgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIB\n"
            "ADAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwGwYDVR0gBBQwEjAGBgRV\n"
            "HSAAMAgGBmeBDAECAjBMBgNVHR8ERTBDMEGgP6A9hjtodHRwOi8vY3JsLmNvbW9k\n"
            "b2NhLmNvbS9DT01PRE9SU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNybDBxBggr\n"
            "BgEFBQcBAQRlMGMwOwYIKwYBBQUHMAKGL2h0dHA6Ly9jcnQuY29tb2RvY2EuY29t\n"
            "L0NPTU9ET1JTQUFkZFRydXN0Q0EuY3J0MCQGCCsGAQUFBzABhhhodHRwOi8vb2Nz\n"
            "cC5jb21vZG9jYS5jb20wDQYJKoZIhvcNAQEMBQADggIBAGmKNmiaHjtlC+B8z6ar\n"
            "cTuvYaQ/5GQBSRDTHY/i1e1n055bl71CHgf50Ltt9zKVWiIpYvgMnFlWJzagIhIR\n"
            "+kf0UclZeylKpUg1fMWXZuAnJTsVejJ1SpH7pmue4lP6DYwT+yO4CxIsru3bHUeQ\n"
            "1dCTaXaROBU01xjqfrxrWN4qOZADRARKVtho5fV8aX6efVRL0NiGq2dmE1deiSoX\n"
            "rS2uvUAOZu2K/1S0wQHLqeBHuhFhj62uI0gqxiV5iRxBBJXAEepXK9a0l/qx6RVi\n"
            "7Epxd/3zoZza9msAKcUy5/pO6rMqpxiXHFinQjZf7BTP+HsO993MiBWamlzI8SDH\n"
            "0YZyoRebrrr+bKgy0QB2SXP3PyeHPLbJLfqqkJDJCgmfyWkfBxmpv966+AuIgkQW\n"
            "EH8HwIAiX3+8MN66zQd5ZFbY//NPnDC7bh5RS+bNvRfExb/IP46xH4pGtwZDb2It\n"
            "z1GdRcqK6ROLwMeRvlu2+jdKif7wndoTJiIsBpA+ixOYoBnW3dpKSH89D4mdJHJL\n"
            "DntE/9Q2toN2I1iLFGy4XfdhbTl27d0SPWuHiJeRvsBGAh52HN22r1xP9QDWnE2p\n"
            "4J6ijvyxFnlcIdNFgZoMOWxtKNcl0rcRkND23m9e9Pqki2Z3ci+bkEAsUhJg+f+1\n"
            "cC6JmnkJiYEt7Fx4b4GH8fxV\n"
            "-----END CERTIFICATE----- \n"
            );

const long long Preferences::defaultHttpsCertExpiration = 1586476799;
const long long Preferences::LOCAL_HTTPS_CERT_MAX_EXPIRATION_SECS = 3888000; // 45 days
const long long Preferences::LOCAL_HTTPS_CERT_RENEW_INTERVAL_SECS = 7200; // 2 hours

const QString Preferences::FINDER_EXT_BUNDLE_ID = QString::fromUtf8("mega.mac.MEGAShellExtFinder");
QStringList Preferences::HTTPS_ALLOWED_ORIGINS;
bool Preferences::HTTPS_ORIGIN_CHECK_ENABLED = true;

QString Preferences::BASE_URL = QString::fromAscii("https://mega.nz");

#ifdef WIN32
    const QString Preferences::UPDATE_CHECK_URL                 = QString::fromUtf8("http://g.static.mega.co.nz/upd/wsync/v.txt");
#else
    const QString Preferences::UPDATE_CHECK_URL                 = QString::fromUtf8("http://g.static.mega.co.nz/upd/msync/v.txt");
#endif

const char Preferences::UPDATE_PUBLIC_KEY[] = "EACTzXPE8fdMhm6LizLe1FxV2DncybVh2cXpW3momTb8tpzRNT833r1RfySz5uHe8gdoXN1W0eM5Bk8X-LefygYYDS9RyXrRZ8qXrr9ITJ4r8ATnFIEThO5vqaCpGWTVi5pOPI5FUTJuhghVKTyAels2SpYT5CmfSQIkMKv7YVldaV7A-kY060GfrNg4--ETyIzhvaSZ_jyw-gmzYl_dwfT9kSzrrWy1vQG8JPNjKVPC4MCTZJx9SNvp1fVi77hhgT-Mc5PLcDIfjustlJkDBHtmGEjyaDnaWQf49rGq94q23mLc56MSjKpjOR1TtpsCY31d1Oy2fEXFgghM0R-1UkKswVuWhEEd8nO2PimJOl4u9ZJ2PWtJL1Ro0Hlw9OemJ12klIAxtGV-61Z60XoErbqThwWT5Uu3D2gjK9e6rL9dufSoqjC7UA2C0h7KNtfUcUHw0UWzahlR8XBNFXaLWx9Z8fRtA_a4seZcr0AhIA7JdQG5i8tOZo966KcFnkU77pfQTSprnJhCfEmYbWm9EZA122LJBWq2UrSQQN3pKc9goNaaNxy5PYU1yXyiAfMVsBDmDonhRWQh2XhdV-FWJ3rOGMe25zOwV4z1XkNBuW4T1JF2FgqGR6_q74B2ccFC8vrNGvlTEcs3MSxTI_EKLXQvBYy7hxG8EPUkrMVCaWzzTQAFEQ";
const QString Preferences::CRASH_REPORT_URL                 = QString::fromUtf8("http://g.api.mega.co.nz/hb?crashdump");
const QString Preferences::UPDATE_FOLDER_NAME               = QString::fromAscii("update");
const QString Preferences::UPDATE_BACKUP_FOLDER_NAME        = QString::fromAscii("backup");
const QString Preferences::PROXY_TEST_URL                   = QString::fromUtf8("https://g.api.mega.co.nz/cs");
const QString Preferences::PROXY_TEST_SUBSTRING             = QString::fromUtf8("-2");
const QString Preferences::syncsGroupKey            = QString::fromAscii("Syncs");
const QString Preferences::currentAccountKey        = QString::fromAscii("currentAccount");
const QString Preferences::emailKey                 = QString::fromAscii("email");
const QString Preferences::firstNameKey             = QString::fromAscii("firstName");
const QString Preferences::lastNameKey              = QString::fromAscii("lastName");
const QString Preferences::emailHashKey             = QString::fromAscii("emailHash");
const QString Preferences::privatePwKey             = QString::fromAscii("privatePw");
const QString Preferences::totalStorageKey          = QString::fromAscii("totalStorage");
const QString Preferences::usedStorageKey           = QString::fromAscii("usedStorage");
const QString Preferences::cloudDriveStorageKey     = QString::fromAscii("cloudDriveStorage");
const QString Preferences::inboxStorageKey          = QString::fromAscii("inboxStorage");
const QString Preferences::rubbishStorageKey        = QString::fromAscii("rubbishStorage");
const QString Preferences::inShareStorageKey        = QString::fromAscii("inShareStorage");
const QString Preferences::versionsStorageKey        = QString::fromAscii("versionsStorage");
const QString Preferences::cloudDriveFilesKey       = QString::fromAscii("cloudDriveFiles");
const QString Preferences::inboxFilesKey            = QString::fromAscii("inboxFiles");
const QString Preferences::rubbishFilesKey          = QString::fromAscii("rubbishFiles");
const QString Preferences::inShareFilesKey          = QString::fromAscii("inShareFiles");
const QString Preferences::cloudDriveFoldersKey     = QString::fromAscii("cloudDriveFolders");
const QString Preferences::inboxFoldersKey          = QString::fromAscii("inboxFolders");
const QString Preferences::rubbishFoldersKey        = QString::fromAscii("rubbishFolders");
const QString Preferences::inShareFoldersKey        = QString::fromAscii("inShareFolders");
const QString Preferences::totalBandwidthKey        = QString::fromAscii("totalBandwidth");
const QString Preferences::usedBandwidthIntervalKey        = QString::fromAscii("usedBandwidthInterval");
const QString Preferences::usedBandwidthKey         = QString::fromAscii("usedBandwidth");

const QString Preferences::overStorageDialogExecutionKey = QString::fromAscii("overStorageDialogExecution");
const QString Preferences::overStorageNotificationExecutionKey = QString::fromAscii("overStorageNotificationExecution");
const QString Preferences::almostOverStorageNotificationExecutionKey = QString::fromAscii("almostOverStorageNotificationExecution");
const QString Preferences::almostOverStorageDismissExecutionKey = QString::fromAscii("almostOverStorageDismissExecution");
const QString Preferences::overStorageDismissExecutionKey = QString::fromAscii("overStorageDismissExecution");
const QString Preferences::storageStateQKey = QString::fromAscii("storageState");

const QString Preferences::accountTypeKey           = QString::fromAscii("accountType");
const QString Preferences::proExpirityTimeKey       = QString::fromAscii("proExpirityTime");
const QString Preferences::showNotificationsKey     = QString::fromAscii("showNotifications");
const QString Preferences::startOnStartupKey        = QString::fromAscii("startOnStartup");
const QString Preferences::languageKey              = QString::fromAscii("language");
const QString Preferences::updateAutomaticallyKey   = QString::fromAscii("updateAutomatically");
const QString Preferences::uploadLimitKBKey         = QString::fromAscii("uploadLimitKB");
const QString Preferences::downloadLimitKBKey       = QString::fromAscii("downloadLimitKB");
const QString Preferences::parallelUploadConnectionsKey       = QString::fromAscii("parallelUploadConnections");
const QString Preferences::parallelDownloadConnectionsKey     = QString::fromAscii("parallelDownloadConnections");

const QString Preferences::upperSizeLimitKey        = QString::fromAscii("upperSizeLimit");
const QString Preferences::lowerSizeLimitKey        = QString::fromAscii("lowerSizeLimit");

const QString Preferences::lastCustomStreamingAppKey    = QString::fromAscii("lastCustomStreamingApp");
const QString Preferences::transferDownloadMethodKey    = QString::fromAscii("transferDownloadMethod");
const QString Preferences::transferUploadMethodKey      = QString::fromAscii("transferUploadMethod");

const QString Preferences::upperSizeLimitValueKey       = QString::fromAscii("upperSizeLimitValue");
const QString Preferences::lowerSizeLimitValueKey       = QString::fromAscii("lowerSizeLimitValue");
const QString Preferences::upperSizeLimitUnitKey        = QString::fromAscii("upperSizeLimitUnit");
const QString Preferences::lowerSizeLimitUnitKey        = QString::fromAscii("lowerSizeLimitUnit");


const QString Preferences::cleanerDaysLimitKey       = QString::fromAscii("cleanerDaysLimit");
const QString Preferences::cleanerDaysLimitValueKey  = QString::fromAscii("cleanerDaysLimitValue");

const QString Preferences::folderPermissionsKey         = QString::fromAscii("folderPermissions");
const QString Preferences::filePermissionsKey           = QString::fromAscii("filePermissions");

const QString Preferences::proxyTypeKey             = QString::fromAscii("proxyType");
const QString Preferences::proxyProtocolKey         = QString::fromAscii("proxyProtocol");
const QString Preferences::proxyServerKey           = QString::fromAscii("proxyServer");
const QString Preferences::proxyPortKey             = QString::fromAscii("proxyPort");
const QString Preferences::proxyRequiresAuthKey     = QString::fromAscii("proxyRequiresAuth");
const QString Preferences::proxyUsernameKey         = QString::fromAscii("proxyUsername");
const QString Preferences::proxyPasswordKey         = QString::fromAscii("proxyPassword");
const QString Preferences::syncNameKey              = QString::fromAscii("syncName");
const QString Preferences::syncIdKey                = QString::fromAscii("syncId");
const QString Preferences::localFolderKey           = QString::fromAscii("localFolder");
const QString Preferences::megaFolderKey            = QString::fromAscii("megaFolder");
const QString Preferences::megaFolderHandleKey      = QString::fromAscii("megaFolderHandle");
const QString Preferences::folderActiveKey          = QString::fromAscii("folderActive");
const QString Preferences::temporaryInactiveKey     = QString::fromAscii("temporaryInactive");
const QString Preferences::downloadFolderKey        = QString::fromAscii("downloadFolder");
const QString Preferences::uploadFolderKey          = QString::fromAscii("uploadFolder");
const QString Preferences::importFolderKey          = QString::fromAscii("importFolder");
const QString Preferences::hasDefaultUploadFolderKey    = QString::fromAscii("hasDefaultUploadFolder");
const QString Preferences::hasDefaultDownloadFolderKey  = QString::fromAscii("hasDefaultDownloadFolder");
const QString Preferences::hasDefaultImportFolderKey    = QString::fromAscii("hasDefaultImportFolder");
const QString Preferences::fileNameKey              = QString::fromAscii("fileName");
const QString Preferences::fileHandleKey            = QString::fromAscii("fileHandle");
const QString Preferences::localPathKey             = QString::fromAscii("localPath");
const QString Preferences::localFingerprintKey      = QString::fromAscii("localFingerprint");
const QString Preferences::fileTimeKey              = QString::fromAscii("fileTime");
const QString Preferences::isCrashedKey             = QString::fromAscii("isCrashed");
const QString Preferences::wasPausedKey             = QString::fromAscii("wasPaused");
const QString Preferences::wasUploadsPausedKey      = QString::fromAscii("wasUploadsPaused");
const QString Preferences::wasDownloadsPausedKey    = QString::fromAscii("wasDownloadsPaused");
const QString Preferences::lastExecutionTimeKey     = QString::fromAscii("lastExecutionTime");
const QString Preferences::excludedSyncNamesKey     = QString::fromAscii("excludedSyncNames");
const QString Preferences::excludedSyncPathsKey     = QString::fromAscii("excludedSyncPaths");
const QString Preferences::lastVersionKey           = QString::fromAscii("lastVersion");
const QString Preferences::lastStatsRequestKey      = QString::fromAscii("lastStatsRequest");
const QString Preferences::lastUpdateTimeKey        = QString::fromAscii("lastUpdateTime");
const QString Preferences::lastUpdateVersionKey     = QString::fromAscii("lastUpdateVersion");
const QString Preferences::previousCrashesKey       = QString::fromAscii("previousCrashes");
const QString Preferences::lastRebootKey            = QString::fromAscii("lastReboot");
const QString Preferences::lastExitKey              = QString::fromAscii("lastExit");
const QString Preferences::disableOverlayIconsKey   = QString::fromAscii("disableOverlayIcons");
const QString Preferences::disableFileVersioningKey = QString::fromAscii("disableFileVersioning");
const QString Preferences::disableLeftPaneIconsKey  = QString::fromAscii("disableLeftPaneIcons");
const QString Preferences::sessionKey               = QString::fromAscii("session");
const QString Preferences::firstStartDoneKey        = QString::fromAscii("firstStartDone");
const QString Preferences::firstSyncDoneKey         = QString::fromAscii("firstSyncDone");
const QString Preferences::firstFileSyncedKey       = QString::fromAscii("firstFileSynced");
const QString Preferences::firstWebDownloadKey      = QString::fromAscii("firstWebclientDownload");
const QString Preferences::fatWarningShownKey       = QString::fromAscii("fatWarningShown");
const QString Preferences::installationTimeKey      = QString::fromAscii("installationTime");
const QString Preferences::accountCreationTimeKey   = QString::fromAscii("accountCreationTime");
const QString Preferences::hasLoggedInKey           = QString::fromAscii("hasLoggedIn");
const QString Preferences::useHttpsOnlyKey          = QString::fromAscii("useHttpsOnly");
const QString Preferences::SSLcertificateExceptionKey  = QString::fromAscii("SSLcertificateException");
const QString Preferences::maxMemoryUsageKey        = QString::fromAscii("maxMemoryUsage");
const QString Preferences::maxMemoryReportTimeKey   = QString::fromAscii("maxMemoryReportTime");
const QString Preferences::oneTimeActionDoneKey     = QString::fromAscii("oneTimeActionDone");
const QString Preferences::httpsKeyKey              = QString::fromAscii("httpsKey2");
const QString Preferences::httpsCertKey             = QString::fromAscii("httpsCert2");
const QString Preferences::httpsCertIntermediateKey = QString::fromAscii("httpsCertIntermediate2");
const QString Preferences::httpsCertExpirationKey   = QString::fromAscii("httpsCertExpiration2");
const QString Preferences::transferIdentifierKey    = QString::fromAscii("transferIdentifier");
const QString Preferences::lastPublicHandleKey      = QString::fromAscii("lastPublicHandle");
const QString Preferences::lastPublicHandleTimestampKey = QString::fromAscii("lastPublicHandleTimestamp");

const bool Preferences::defaultShowNotifications    = true;
const bool Preferences::defaultStartOnStartup       = true;
const bool Preferences::defaultUpdateAutomatically  = true;
const bool Preferences::defaultUpperSizeLimit       = false;
const bool Preferences::defaultLowerSizeLimit       = false;

const bool Preferences::defaultCleanerDaysLimit     = true;

const bool Preferences::defaultUseHttpsOnly         = true;
const bool Preferences::defaultSSLcertificateException = false;
const int  Preferences::defaultUploadLimitKB        = -1;
const int  Preferences::defaultDownloadLimitKB      = 0;
const long long Preferences::defaultTimeStamp       = 0;
const unsigned long long  Preferences::defaultTransferIdentifier   = 0;
const int  Preferences::defaultParallelUploadConnections      = 3;
const int  Preferences::defaultParallelDownloadConnections    = 4;
const int Preferences::defaultTransferDownloadMethod      = MegaApi::TRANSFER_METHOD_AUTO;
const int Preferences::defaultTransferUploadMethod        = MegaApi::TRANSFER_METHOD_AUTO;
const long long  Preferences::defaultUpperSizeLimitValue              = 0;
const long long  Preferences::defaultLowerSizeLimitValue              = 0;
const int  Preferences::defaultCleanerDaysLimitValue            = 30;
const int Preferences::defaultLowerSizeLimitUnit =  Preferences::MEGA_BYTE_UNIT;
const int Preferences::defaultUpperSizeLimitUnit =  Preferences::MEGA_BYTE_UNIT;
const int Preferences::defaultFolderPermissions = 0;
const int Preferences::defaultFilePermissions   = 0;
#ifdef WIN32
const int  Preferences::defaultProxyType            = Preferences::PROXY_TYPE_AUTO;
#else
const int  Preferences::defaultProxyType            = Preferences::PROXY_TYPE_NONE;
#endif
const int  Preferences::defaultProxyProtocol        = Preferences::PROXY_PROTOCOL_HTTP;
const QString  Preferences::defaultProxyServer      = QString::fromAscii("127.0.0.1");
const int Preferences::defaultProxyPort             = 8080;
const bool Preferences::defaultProxyRequiresAuth    = false;
const QString Preferences::defaultProxyUsername     = QString::fromAscii("");
const QString Preferences::defaultProxyPassword     = QString::fromAscii("");

Preferences *Preferences::preferences = NULL;

Preferences *Preferences::instance()
{
    if (!preferences)
    {
        Preferences::HTTPS_ALLOWED_ORIGINS.append(Preferences::BASE_URL);
        Preferences::HTTPS_ALLOWED_ORIGINS.append(QString::fromUtf8("https://mega.co.nz"));
        Preferences::HTTPS_ALLOWED_ORIGINS.append(QString::fromUtf8("chrome-extension://*"));
        Preferences::HTTPS_ALLOWED_ORIGINS.append(QString::fromUtf8("moz-extension://*"));
        Preferences::HTTPS_ALLOWED_ORIGINS.append(QString::fromUtf8("edge-extension://*"));

        preferences = new Preferences();
    }
    return Preferences::preferences;
}

void Preferences::initialize(QString dataPath)
{
    this->dataPath = dataPath;
#if QT_VERSION >= 0x050000
    QString lockSettingsFile = QDir::toNativeSeparators(dataPath + QString::fromAscii("/MEGAsync.cfg.lock"));
    QFile::remove(lockSettingsFile);
#endif

    QDir dir(dataPath);
    dir.mkpath(QString::fromAscii("."));
    QString settingsFile = QDir::toNativeSeparators(dataPath + QString::fromAscii("/MEGAsync.cfg"));
    QString bakSettingsFile = QDir::toNativeSeparators(dataPath + QString::fromAscii("/MEGAsync.cfg.bak"));
    bool retryFlag = false;

    errorFlag = false;
    settings = new EncryptedSettings(settingsFile);

    QString currentAccount = settings->value(currentAccountKey).toString();
    if (currentAccount.size())
    {
        if (hasEmail(currentAccount))
        {
            login(currentAccount);
        }
        else
        {
            errorFlag = true;
            retryFlag = true;
        }
    }
    else
    {
        retryFlag = true;
    }

    if (QFile::exists(bakSettingsFile) && retryFlag)
    {
        if (QFile::exists(settingsFile))
        {
            QFile::remove(settingsFile);
        }

        if (QFile::rename(bakSettingsFile,settingsFile))
        {
            delete settings;
            settings = new EncryptedSettings(settingsFile);

            //Retry with backup file
            currentAccount = settings->value(currentAccountKey).toString();
            if (currentAccount.size())
            {
                if (hasEmail(currentAccount))
                {
                    login(currentAccount);
                    errorFlag = false;
                }
                else
                {
                    errorFlag = true;
                }
            }
        }
    }

    if (errorFlag)
    {
        clearAll();
    }
}

Preferences::Preferences() : QObject(), mutex(QMutex::Recursive)
{
    diffTimeWithSDK = 0;
    overStorageDialogExecution = -1;
    overStorageNotificationExecution = -1;
    almostOverStorageNotificationExecution = -1;
    almostOverStorageDismissExecution = -1;
    overStorageDismissExecution = -1;
    lastTransferNotification = 0;
    clearTemporalBandwidth();
}

QString Preferences::email()
{
    mutex.lock();
    assert(logged());
    QString value = settings->value(emailKey).toString();
    mutex.unlock();
    return value;
}

void Preferences::setEmail(QString email)
{
    mutex.lock();
    login(email);
    settings->setValue(emailKey, email);
    settings->sync();
    mutex.unlock();
    emit stateChanged();
}

QString Preferences::firstName()
{
    mutex.lock();
    assert(logged());
    QString value = settings->value(firstNameKey, QString()).toString();
    mutex.unlock();
    return value;
}

void Preferences::setFirstName(QString firstName)
{
    mutex.lock();
    settings->setValue(firstNameKey, firstName);
    settings->sync();
    mutex.unlock();
}

QString Preferences::lastName()
{
    mutex.lock();
    assert(logged());
    QString value = settings->value(lastNameKey, QString()).toString();
    mutex.unlock();
    return value;
}

void Preferences::setLastName(QString lastName)
{
    mutex.lock();
    settings->setValue(lastNameKey, lastName);
    settings->sync();
    mutex.unlock();
}

QString Preferences::emailHash()
{
    mutex.lock();
    assert(logged());
    QString value = settings->value(emailHashKey).toString();
    mutex.unlock();
    return value;
}

QString Preferences::privatePw()
{
    mutex.lock();
    assert(logged());
    QString value = settings->value(privatePwKey).toString();
    mutex.unlock();
    return value;
}

void Preferences::setSession(QString session)
{
    mutex.lock();
    assert(logged());
    settings->setValue(sessionKey, session);
    settings->remove(emailHashKey);
    settings->remove(privatePwKey);
    settings->sync();
    mutex.unlock();
}

QString Preferences::getSession()
{
    mutex.lock();
    assert(logged());
    QString value = settings->value(sessionKey).toString();
    mutex.unlock();
    return value;
}

unsigned long long Preferences::transferIdentifier()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(transferIdentifierKey, defaultTransferIdentifier).toLongLong();
    settings->setValue(transferIdentifierKey, ++value);
    mutex.unlock();
    return value;
}

long long Preferences::lastTransferNotificationTimestamp()
{
    return lastTransferNotification;
}

void Preferences::setLastTransferNotificationTimestamp()
{
    lastTransferNotification = QDateTime::currentMSecsSinceEpoch();
}

long long Preferences::totalStorage()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(totalStorageKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setTotalStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(totalStorageKey, value);
    mutex.unlock();
}

long long Preferences::usedStorage()
{
    mutex.lock();
    assert(logged());

    long long value = settings->value(usedStorageKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setUsedStorage(long long value)
{
    mutex.lock();
    assert(logged());
    if (value < 0)
    {
        value = 0;
    }
    settings->setValue(usedStorageKey, value);
    mutex.unlock();
}

long long Preferences::availableStorage()
{
    mutex.lock();
    assert(logged());
    long long total = settings->value(totalStorageKey).toLongLong();
    long long used = settings->value(usedStorageKey).toLongLong();
    mutex.unlock();
    long long available = total - used;
    return available >= 0 ? available : 0;
}

long long Preferences::cloudDriveStorage()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(cloudDriveStorageKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setCloudDriveStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(cloudDriveStorageKey, value);
    mutex.unlock();
}

long long Preferences::inboxStorage()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(inboxStorageKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setInboxStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inboxStorageKey, value);
    mutex.unlock();
}

long long Preferences::rubbishStorage()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(rubbishStorageKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setRubbishStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(rubbishStorageKey, value);
    mutex.unlock();
}

long long Preferences::inShareStorage()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(inShareStorageKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setInShareStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inShareStorageKey, value);
    mutex.unlock();
}

long long Preferences::versionsStorage()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(versionsStorageKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setVersionsStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(versionsStorageKey, value);
    mutex.unlock();
}

long long Preferences::cloudDriveFiles()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(cloudDriveFilesKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setCloudDriveFiles(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(cloudDriveFilesKey, value);
    mutex.unlock();
}

long long Preferences::inboxFiles()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(inboxFilesKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setInboxFiles(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inboxFilesKey, value);
    mutex.unlock();
}

long long Preferences::rubbishFiles()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(rubbishFilesKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setRubbishFiles(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(rubbishFilesKey, value);
    mutex.unlock();
}

long long Preferences::inShareFiles()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(inShareFilesKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setInShareFiles(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inShareFilesKey, value);
    mutex.unlock();
}

long long Preferences::cloudDriveFolders()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(cloudDriveFoldersKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setCloudDriveFolders(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(cloudDriveFoldersKey, value);
    mutex.unlock();
}

long long Preferences::inboxFolders()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(inboxFoldersKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setInboxFolders(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inboxFoldersKey, value);
    mutex.unlock();
}

long long Preferences::rubbishFolders()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(rubbishFoldersKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setRubbishFolders(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(rubbishFoldersKey, value);
    mutex.unlock();
}

long long Preferences::inShareFolders()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(inShareFoldersKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setInShareFolders(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inShareFoldersKey, value);
    mutex.unlock();
}

long long Preferences::totalBandwidth()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(totalBandwidthKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setTotalBandwidth(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(totalBandwidthKey, value);
    mutex.unlock();
}

int Preferences::bandwidthInterval()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(usedBandwidthIntervalKey).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setBandwidthInterval(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(usedBandwidthIntervalKey, value);
    mutex.unlock();
}

bool Preferences::isTemporalBandwidthValid()
{
    return isTempBandwidthValid;
}

long long Preferences::getMsDiffTimeWithSDK()
{
    return diffTimeWithSDK;
}

void Preferences::setDsDiffTimeWithSDK(long long diffTime)
{
    this->diffTimeWithSDK = diffTime;
}

long long Preferences::getOverStorageDialogExecution()
{
    if (overStorageDialogExecution != -1)
    {
        return overStorageDialogExecution;
    }

    mutex.lock();
    assert(logged());
    overStorageDialogExecution = settings->value(overStorageDialogExecutionKey, defaultTimeStamp).toLongLong();
    mutex.unlock();
    return overStorageDialogExecution;
}

void Preferences::setOverStorageDialogExecution(long long timestamp)
{
    overStorageDialogExecution = timestamp;
    mutex.lock();
    assert(logged());
    settings->setValue(overStorageDialogExecutionKey, timestamp);
    mutex.unlock();
}

long long Preferences::getOverStorageNotificationExecution()
{
    if (overStorageNotificationExecution != -1)
    {
        return overStorageNotificationExecution;
    }

    mutex.lock();
    assert(logged());
    overStorageNotificationExecution = settings->value(overStorageNotificationExecutionKey, defaultTimeStamp).toLongLong();
    mutex.unlock();
    return overStorageNotificationExecution;
}

void Preferences::setOverStorageNotificationExecution(long long timestamp)
{
    overStorageNotificationExecution = timestamp;
    mutex.lock();
    assert(logged());
    settings->setValue(overStorageNotificationExecutionKey, timestamp);
    mutex.unlock();
}

long long Preferences::getAlmostOverStorageNotificationExecution()
{
    if (almostOverStorageNotificationExecution != -1)
    {
        return almostOverStorageNotificationExecution;
    }

    mutex.lock();
    assert(logged());
    almostOverStorageNotificationExecution = settings->value(almostOverStorageNotificationExecutionKey, defaultTimeStamp).toLongLong();
    mutex.unlock();
    return almostOverStorageNotificationExecution;
}

void Preferences::setAlmostOverStorageNotificationExecution(long long timestamp)
{
    almostOverStorageNotificationExecution = timestamp;
    mutex.lock();
    assert(logged());
    settings->setValue(almostOverStorageNotificationExecutionKey, timestamp);
    mutex.unlock();
}

long long Preferences::getAlmostOverStorageDismissExecution()
{
    if (almostOverStorageDismissExecution != -1)
    {
        return almostOverStorageDismissExecution;
    }

    mutex.lock();
    assert(logged());
    almostOverStorageDismissExecution = settings->value(almostOverStorageDismissExecutionKey, defaultTimeStamp).toLongLong();
    mutex.unlock();
    return almostOverStorageDismissExecution;
}

void Preferences::setAlmostOverStorageDismissExecution(long long timestamp)
{
    almostOverStorageDismissExecution = timestamp;
    mutex.lock();
    assert(logged());
    settings->setValue(almostOverStorageDismissExecutionKey, timestamp);
    mutex.unlock();
}

long long Preferences::getOverStorageDismissExecution()
{
    if (overStorageDismissExecution != -1)
    {
        return overStorageDismissExecution;
    }

    mutex.lock();
    assert(logged());
    overStorageDismissExecution = settings->value(overStorageDismissExecutionKey, defaultTimeStamp).toLongLong();
    mutex.unlock();
    return overStorageDismissExecution;
}

void Preferences::setOverStorageDismissExecution(long long timestamp)
{
    overStorageDismissExecution = timestamp;
    mutex.lock();
    assert(logged());
    settings->setValue(overStorageDismissExecutionKey, timestamp);
    mutex.unlock();
}


int Preferences::getStorageState()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(storageStateQKey, MegaApi::STORAGE_STATE_GREEN).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setStorageState(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(storageStateQKey, value);
    mutex.unlock();
}

void Preferences::setTemporalBandwidthValid(bool value)
{
    this->isTempBandwidthValid = value;
}

long long Preferences::temporalBandwidth()
{
    return tempBandwidth > 0 ? tempBandwidth : 0;
}

void Preferences::setTemporalBandwidth(long long value)
{
    this->tempBandwidth = value;
}

int Preferences::temporalBandwidthInterval()
{
    return tempBandwidthInterval > 0 ? tempBandwidthInterval : 6;
}

void Preferences::setTemporalBandwidthInterval(int value)
{
    this->tempBandwidthInterval = value;
}

long long Preferences::usedBandwidth()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(usedBandwidthKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setUsedBandwidth(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(usedBandwidthKey, value);
    mutex.unlock();
}

int Preferences::accountType()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(accountTypeKey).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setAccountType(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(accountTypeKey, value);
    mutex.unlock();
}

long long Preferences::proExpirityTime()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(proExpirityTimeKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setProExpirityTime(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(proExpirityTimeKey, value);
    mutex.unlock();
}

bool Preferences::showNotifications()
{
    mutex.lock();
    bool value = settings->value(showNotificationsKey, defaultShowNotifications).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setShowNotifications(bool value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(showNotificationsKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::startOnStartup()
{
    mutex.lock();
    bool value = settings->value(startOnStartupKey, defaultStartOnStartup).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setStartOnStartup(bool value)
{
    mutex.lock();
    settings->setValue(startOnStartupKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::usingHttpsOnly()
{
    mutex.lock();
    bool value = settings->value(useHttpsOnlyKey, defaultUseHttpsOnly).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setUseHttpsOnly(bool value)
{
    mutex.lock();
    settings->setValue(useHttpsOnlyKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::SSLcertificateException()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }
    bool value = settings->value(SSLcertificateExceptionKey, defaultSSLcertificateException).toBool();
    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    mutex.unlock();
    return value;
}

void Preferences::setSSLcertificateException(bool value)
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }
    settings->setValue(SSLcertificateExceptionKey, value);
    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    settings->sync();
    mutex.unlock();
}

int Preferences::transferDownloadMethod()
{
    mutex.lock();
    int value = settings->value(transferDownloadMethodKey, defaultTransferDownloadMethod).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setTransferDownloadMethod(int value)
{
    mutex.lock();
    settings->setValue(transferDownloadMethodKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::transferUploadMethod()
{
    mutex.lock();
    int value = settings->value(transferUploadMethodKey, defaultTransferUploadMethod).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setTransferUploadMethod(int value)
{
    mutex.lock();
    settings->setValue(transferUploadMethodKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::language()
{
    mutex.lock();
    QString value = settings->value(languageKey, QLocale::system().name()).toString();
    mutex.unlock();
    return value;
}

void Preferences::setLanguage(QString &value)
{
    mutex.lock();
    settings->setValue(languageKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::updateAutomatically()
{
    mutex.lock();
    bool value = settings->value(updateAutomaticallyKey, defaultUpdateAutomatically).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setUpdateAutomatically(bool value)
{
    mutex.lock();
    settings->setValue(updateAutomaticallyKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::hasDefaultUploadFolder()
{
    mutex.lock();
    bool value = settings->value(hasDefaultUploadFolderKey, uploadFolder() != 0).toBool();
    mutex.unlock();
    return value;
}

bool Preferences::hasDefaultDownloadFolder()
{
    mutex.lock();
    bool value = settings->value(hasDefaultDownloadFolderKey, !downloadFolder().isEmpty()).toBool();
    mutex.unlock();
    return value;
}

bool Preferences::hasDefaultImportFolder()
{
    mutex.lock();
    bool value = settings->value(hasDefaultImportFolderKey, importFolder() != 0).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setHasDefaultUploadFolder(bool value)
{
    mutex.lock();
    settings->setValue(hasDefaultUploadFolderKey, value);
    settings->sync();
    mutex.unlock();
}

void Preferences::setHasDefaultDownloadFolder(bool value)
{
    mutex.lock();
    settings->setValue(hasDefaultDownloadFolderKey, value);
    settings->sync();
    mutex.unlock();
}

void Preferences::setHasDefaultImportFolder(bool value)
{
    mutex.lock();
    settings->setValue(hasDefaultImportFolderKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::canUpdate(QString filePath)
{
    mutex.lock();

    bool value = true;

#ifdef WIN32
    qt_ntfs_permission_lookup++; // turn checking on
#endif
    if (!QFileInfo(filePath).isWritable())
    {
        value = false;
    }
#ifdef WIN32
    qt_ntfs_permission_lookup--; // turn it off again
#endif

    mutex.unlock();

    return value;
}

int Preferences::uploadLimitKB()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(uploadLimitKBKey, defaultUploadLimitKB).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setUploadLimitKB(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(uploadLimitKBKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::downloadLimitKB()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(downloadLimitKBKey, defaultDownloadLimitKB).toInt();
    mutex.unlock();
    return value;
}

int Preferences::parallelUploadConnections()
{
    mutex.lock();
    int value = settings->value(parallelUploadConnectionsKey, defaultParallelUploadConnections).toInt();
    mutex.unlock();
    return value;
}

int Preferences::parallelDownloadConnections()
{
    mutex.lock();
    int value = settings->value(parallelDownloadConnectionsKey, defaultParallelDownloadConnections).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setParallelUploadConnections(int value)
{
    mutex.lock();
    assert(logged());
    if (value < 1 || value > 6)
    {
       value = 3;
    }
    settings->setValue(parallelUploadConnectionsKey, value);
    settings->sync();
    mutex.unlock();
}

void Preferences::setParallelDownloadConnections(int value)
{
    mutex.lock();
    assert(logged());
    if (value < 1 || value > 6)
    {
       value = 4;
    }
    settings->setValue(parallelDownloadConnectionsKey, value);
    settings->sync();
    mutex.unlock();
}

void Preferences::setDownloadLimitKB(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(downloadLimitKBKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::upperSizeLimit()
{
    mutex.lock();
    bool value = settings->value(upperSizeLimitKey, defaultUpperSizeLimit).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setUpperSizeLimit(bool value)
{
    mutex.lock();
    settings->setValue(upperSizeLimitKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::upperSizeLimitValue()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(upperSizeLimitValueKey, defaultUpperSizeLimitValue).toLongLong();
    mutex.unlock();
    return value;
}
void Preferences::setUpperSizeLimitValue(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(upperSizeLimitValueKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::cleanerDaysLimit()
{
    mutex.lock();
    bool value = settings->value(cleanerDaysLimitKey, defaultCleanerDaysLimit).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setCleanerDaysLimit(bool value)
{
    mutex.lock();
    settings->setValue(cleanerDaysLimitKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::cleanerDaysLimitValue()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(cleanerDaysLimitValueKey, defaultCleanerDaysLimitValue).toInt();
    mutex.unlock();
    return value;
}
void Preferences::setCleanerDaysLimitValue(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(cleanerDaysLimitValueKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::upperSizeLimitUnit()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(upperSizeLimitUnitKey, defaultUpperSizeLimitUnit).toInt();
    mutex.unlock();
    return value;
}
void Preferences::setUpperSizeLimitUnit(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(upperSizeLimitUnitKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::lowerSizeLimit()
{
    mutex.lock();
    bool value = settings->value(lowerSizeLimitKey, defaultLowerSizeLimit).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setLowerSizeLimit(bool value)
{
    mutex.lock();
    settings->setValue(lowerSizeLimitKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::lowerSizeLimitValue()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(lowerSizeLimitValueKey, defaultLowerSizeLimitValue).toLongLong();
    mutex.unlock();
    return value;
}
void Preferences::setLowerSizeLimitValue(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lowerSizeLimitValueKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::lowerSizeLimitUnit()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(lowerSizeLimitUnitKey, defaultLowerSizeLimitUnit).toInt();
    mutex.unlock();
    return value;
}
void Preferences::setLowerSizeLimitUnit(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lowerSizeLimitUnitKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::folderPermissionsValue()
{
    mutex.lock();
    int permissions = settings->value(folderPermissionsKey, defaultFolderPermissions).toInt();
    mutex.unlock();
    return permissions;
}

void Preferences::setFolderPermissionsValue(int permissions)
{
    mutex.lock();
    settings->setValue(folderPermissionsKey, permissions);
    settings->sync();
    mutex.unlock();
}

int Preferences::filePermissionsValue()
{
    mutex.lock();
    int permissions = settings->value(filePermissionsKey, defaultFilePermissions).toInt();
    mutex.unlock();
    return permissions;
}

void Preferences::setFilePermissionsValue(int permissions)
{
    mutex.lock();
    settings->setValue(filePermissionsKey, permissions);
    settings->sync();
    mutex.unlock();
}

int Preferences::proxyType()
{
    mutex.lock();
    int value = settings->value(proxyTypeKey, defaultProxyType).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setProxyType(int value)
{
    mutex.lock();
    settings->setValue(proxyTypeKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::proxyProtocol()
{
    mutex.lock();
    int value = settings->value(proxyProtocolKey, defaultProxyProtocol).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setProxyProtocol(int value)
{
    mutex.lock();
    settings->setValue(proxyProtocolKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::proxyServer()
{
    mutex.lock();
    QString value = settings->value(proxyServerKey, defaultProxyServer).toString();
    mutex.unlock();
    return value;
}

void Preferences::setProxyServer(const QString &value)
{
    mutex.lock();
    settings->setValue(proxyServerKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::proxyPort()
{
    mutex.lock();
    int value = settings->value(proxyPortKey, defaultProxyPort).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setProxyPort(int value)
{
    mutex.lock();
    settings->setValue(proxyPortKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::proxyRequiresAuth()
{
    mutex.lock();
    bool value = settings->value(proxyRequiresAuthKey, defaultProxyRequiresAuth).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setProxyRequiresAuth(bool value)
{
    mutex.lock();
    settings->setValue(proxyRequiresAuthKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::getProxyUsername()
{
    mutex.lock();
    QString value = settings->value(proxyUsernameKey, defaultProxyUsername).toString();
    mutex.unlock();
    return value;
}

void Preferences::setProxyUsername(const QString &value)
{
    mutex.lock();
    settings->setValue(proxyUsernameKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::getProxyPassword()
{
    mutex.lock();
    QString value = settings->value(proxyPasswordKey, defaultProxyPassword).toString();
    mutex.unlock();
    return value;
}

void Preferences::setProxyPassword(const QString &value)
{
    mutex.lock();
    settings->setValue(proxyPasswordKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::proxyHostAndPort()
{
    mutex.lock();
    QString proxy;
    QString hostname = proxyServer();

    QHostAddress ipAddress(hostname);
    if (ipAddress.protocol() == QAbstractSocket::IPv6Protocol)
    {
        proxy = QString::fromUtf8("[") + hostname + QString::fromAscii("]:") + QString::number(proxyPort());
    }
    else
    {
        proxy = hostname + QString::fromAscii(":") + QString::number(proxyPort());
    }

    mutex.unlock();
    return proxy;
}

long long Preferences::lastExecutionTime()
{
    mutex.lock();
    long long value = settings->value(lastExecutionTimeKey, 0).toLongLong();
    mutex.unlock();
    return value;
}
long long Preferences::installationTime()
{
    mutex.lock();
    long long value = settings->value(installationTimeKey, 0).toLongLong();
    mutex.unlock();
    return value;
}
void Preferences::setInstallationTime(long long time)
{
    mutex.lock();
    settings->setValue(installationTimeKey, time);
    settings->sync();
    mutex.unlock();
}
long long Preferences::accountCreationTime()
{
    mutex.lock();
    long long value = settings->value(accountCreationTimeKey, 0).toLongLong();
    mutex.unlock();
    return value;
}
void Preferences::setAccountCreationTime(long long time)
{
    mutex.lock();
    settings->setValue(accountCreationTimeKey, time);
    settings->sync();
    mutex.unlock();

}
long long Preferences::hasLoggedIn()
{
    mutex.lock();
    long long value = settings->value(hasLoggedInKey, 0).toLongLong();
    mutex.unlock();
    return value;
}
void Preferences::setHasLoggedIn(long long time)
{
    mutex.lock();
    settings->setValue(hasLoggedInKey, time);
    settings->sync();
    mutex.unlock();
}

bool Preferences::isFirstStartDone()
{
    mutex.lock();
    bool value = settings->value(firstStartDoneKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setFirstStartDone(bool value)
{
    mutex.lock();
    settings->setValue(firstStartDoneKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::isFirstSyncDone()
{
    mutex.lock();
    bool value = settings->value(firstSyncDoneKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setFirstSyncDone(bool value)
{
    mutex.lock();
    settings->setValue(firstSyncDoneKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::isFirstFileSynced()
{
    mutex.lock();
    bool value = settings->value(firstFileSyncedKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setFirstFileSynced(bool value)
{
    mutex.lock();
    settings->setValue(firstFileSyncedKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::isFirstWebDownloadDone()
{
    mutex.lock();
    bool value = settings->value(firstWebDownloadKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setFirstWebDownloadDone(bool value)
{
    mutex.lock();
    settings->setValue(firstWebDownloadKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::isFatWarningShown()
{
    mutex.lock();
    bool value = settings->value(fatWarningShownKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setFatWarningShown(bool value)
{
    mutex.lock();
    settings->setValue(fatWarningShownKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::lastCustomStreamingApp()
{
    mutex.lock();
    QString value = settings->value(lastCustomStreamingAppKey).toString();
    mutex.unlock();
    return value;
}

void Preferences::setLastCustomStreamingApp(const QString &value)
{
    mutex.lock();
    settings->setValue(lastCustomStreamingAppKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::getMaxMemoryUsage()
{
    mutex.lock();
    long long value = settings->value(maxMemoryUsageKey, 0).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setMaxMemoryUsage(long long value)
{
    mutex.lock();
    settings->setValue(maxMemoryUsageKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::getMaxMemoryReportTime()
{
    mutex.lock();
    long long value = settings->value(maxMemoryReportTimeKey, 0).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setMaxMemoryReportTime(long long timestamp)
{
    mutex.lock();
    settings->setValue(maxMemoryReportTimeKey, timestamp);
    settings->sync();
    mutex.unlock();
}

void Preferences::setLastExecutionTime(qint64 time)
{
    mutex.lock();
    settings->setValue(lastExecutionTimeKey, time);
    settings->sync();
    mutex.unlock();
}

long long Preferences::lastUpdateTime()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(lastUpdateTimeKey, 0).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setLastUpdateTime(long long time)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lastUpdateTimeKey, time);
    settings->sync();
    mutex.unlock();
}

int Preferences::lastUpdateVersion()
{
    mutex.lock();
    assert(logged());
    int value = settings->value(lastUpdateVersionKey, 0).toInt();
    mutex.unlock();
    return value;
}

void Preferences::setLastUpdateVersion(int version)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lastUpdateVersionKey, version);
    settings->sync();
    mutex.unlock();
}

QString Preferences::downloadFolder()
{
    mutex.lock();
    QString value = QDir::toNativeSeparators(settings->value(downloadFolderKey).toString());
    mutex.unlock();
    return value;
}

void Preferences::setDownloadFolder(QString value)
{
    mutex.lock();
    settings->setValue(downloadFolderKey, QDir::toNativeSeparators(value));
    settings->sync();
    mutex.unlock();
}

long long Preferences::uploadFolder()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(uploadFolderKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setUploadFolder(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(uploadFolderKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::importFolder()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(importFolderKey).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setImportFolder(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(importFolderKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::getNumSyncedFolders()
{
    mutex.lock();
    int value = localFolders.length();
    mutex.unlock();
    return value;
}

QString Preferences::getSyncName(int num)
{
    mutex.lock();
    assert(logged() && (syncNames.size()>num));
    if (num >= syncNames.size())
    {
        mutex.unlock();
        return QString();
    }
    QString value = syncNames.at(num);
    mutex.unlock();
    return value;
}

QString Preferences::getSyncID(int num)
{
    mutex.lock();
    assert(logged() && (syncIDs.size() > num));
    if (num >= syncIDs.size())
    {
        mutex.unlock();
        return QString();
    }
    QString value = syncIDs.at(num);
    mutex.unlock();
    return value;
}

QString Preferences::getLocalFolder(int num)
{
    mutex.lock();
    assert(logged() && (localFolders.size()>num));
    if (num >= localFolders.size())
    {
        mutex.unlock();
        return QString();
    }

    QFileInfo fileInfo(localFolders.at(num));
    QString value = QDir::toNativeSeparators(fileInfo.canonicalFilePath());
    if (value.isEmpty())
    {
        value = QDir::toNativeSeparators(localFolders.at(num));
    }

    mutex.unlock();
    return value;
}

QString Preferences::getMegaFolder(int num)
{
    mutex.lock();
    assert(logged() && (megaFolders.size()>num));
    if (num >= megaFolders.size())
    {
        mutex.unlock();
        return QString();
    }
    QString value = megaFolders.at(num);
    mutex.unlock();
    return value;
}

long long Preferences::getLocalFingerprint(int num)
{
    mutex.lock();
    assert(logged() && (localFingerprints.size()>num));
    if (num >= localFingerprints.size())
    {
        mutex.unlock();
        return 0;
    }
    long long value = localFingerprints.at(num);
    mutex.unlock();
    return value;
}

void Preferences::setLocalFingerprint(int num, long long fingerprint)
{
    mutex.lock();
    if (num >= localFingerprints.size())
    {
        mutex.unlock();
        return;
    }
    localFingerprints[num] = fingerprint;
    writeFolders();
    mutex.unlock();
}

MegaHandle Preferences::getMegaFolderHandle(int num)
{
    mutex.lock();
    assert(logged() && (megaFolderHandles.size()>num));
    if (num >= megaFolderHandles.size())
    {
        mutex.unlock();
        return mega::INVALID_HANDLE;
    }
    long long value = megaFolderHandles.at(num);
    mutex.unlock();
    return value;
}

bool Preferences::isFolderActive(int num)
{
    mutex.lock();
    if (num >= activeFolders.size())
    {
        mutex.unlock();
        return false;
    }
    bool value = activeFolders.at(num);
    mutex.unlock();
    return value;
}

bool Preferences::isTemporaryInactiveFolder(int num)
{
    mutex.lock();
    if (num >= temporaryInactiveFolders.size())
    {
        mutex.unlock();
        return false;
    }
    bool value = temporaryInactiveFolders.at(num);
    mutex.unlock();
    return value;
}

void Preferences::setSyncState(int num, bool enabled, bool temporaryDisabled)
{
    mutex.lock();
    if (num >= activeFolders.size() || num >= temporaryInactiveFolders.size())
    {
        mutex.unlock();
        return;
    }
    activeFolders[num] = enabled;
    temporaryInactiveFolders[num] = temporaryDisabled;
    writeFolders();
    mutex.unlock();

    if (enabled)
    {
        Platform::syncFolderAdded(localFolders[num], syncNames[num], syncIDs[num]);
    }
}

bool Preferences::isOneTimeActionDone(int action)
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    bool value = settings->value(oneTimeActionDoneKey + QString::number(action), false).toBool();

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    mutex.unlock();
    return value;
}

void Preferences::setOneTimeActionDone(int action, bool done)
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(oneTimeActionDoneKey + QString::number(action), done);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    settings->sync();
    mutex.unlock();
}

QStringList Preferences::getSyncNames()
{
    mutex.lock();
    QStringList value = syncNames;
    mutex.unlock();
    return value;
}

QStringList Preferences::getSyncIDs()
{
    mutex.lock();
    QStringList value = syncIDs;
    mutex.unlock();
    return value;
}

QStringList Preferences::getMegaFolders()
{
    mutex.lock();
    QStringList value = megaFolders;
    mutex.unlock();
    return value;
}

QStringList Preferences::getLocalFolders()
{
    mutex.lock();
    QStringList value = localFolders;
    mutex.unlock();
    return value;
}

QList<long long> Preferences::getMegaFolderHandles()
{
    mutex.lock();
    QList<long long> value = megaFolderHandles;
    mutex.unlock();
    return value;
}

void Preferences::addSyncedFolder(QString localFolder, QString megaFolder, mega::MegaHandle megaFolderHandle, QString syncName,  bool active)
{
    mutex.lock();
    assert(logged());

    QFileInfo localFolderInfo(localFolder);
    if (syncName.isEmpty())
    {
        syncName = localFolderInfo.fileName();
    }

    if (syncName.isEmpty())
    {
        syncName = QDir::toNativeSeparators(localFolder);
    }

    syncName.remove(QChar::fromAscii(':')).remove(QDir::separator());

    localFolder = QDir::toNativeSeparators(localFolderInfo.canonicalFilePath());
    syncNames.append(syncName);
    QString syncID = QUuid::createUuid().toString().toUpper();
    syncIDs.append(syncID);
    localFolders.append(localFolder);
    megaFolders.append(megaFolder);
    megaFolderHandles.append(megaFolderHandle);
    activeFolders.append(active);
    temporaryInactiveFolders.append(false);
    localFingerprints.append(0);
    writeFolders();
    mutex.unlock();
    Platform::syncFolderAdded(localFolder, syncName, syncID);
}

void Preferences::setMegaFolderHandle(int num, MegaHandle handle)
{
    mutex.lock();
    if (num >= megaFolderHandles.size())
    {
        mutex.unlock();
        return;
    }
    megaFolderHandles[num] = handle;
    writeFolders();
    mutex.unlock();
}

void Preferences::removeSyncedFolder(int num)
{
    mutex.lock();
    assert(logged());
    syncNames.removeAt(num);
    syncIDs.removeAt(num);
    localFolders.removeAt(num);
    megaFolders.removeAt(num);
    megaFolderHandles.removeAt(num);
    activeFolders.removeAt(num);
    temporaryInactiveFolders.removeAt(num);
    localFingerprints.removeAt(num);
    writeFolders();
    mutex.unlock();
}

void Preferences::removeAllFolders()
{
    mutex.lock();
    assert(logged());

    for (int i = 0; i < localFolders.size(); i++)
    {
        Platform::syncFolderRemoved(localFolders[i], syncNames[i], syncIDs[i]);
    }

    syncNames.clear();
    syncIDs.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    activeFolders.clear();
    temporaryInactiveFolders.clear();
    localFingerprints.clear();
    writeFolders();
    mutex.unlock();
}

QStringList Preferences::getExcludedSyncNames()
{
    mutex.lock();
    assert(logged());
    QStringList value = excludedSyncNames;
    mutex.unlock();
    return value;
}

void Preferences::setExcludedSyncNames(QStringList names)
{
    mutex.lock();
    assert(logged());
    excludedSyncNames = names;
    if (!excludedSyncNames.size())
    {
        settings->remove(excludedSyncNamesKey);
    }
    else
    {
        settings->setValue(excludedSyncNamesKey, excludedSyncNames.join(QString::fromAscii("\n")));
    }

    settings->sync();
    mutex.unlock();
}

QStringList Preferences::getExcludedSyncPaths()
{
    mutex.lock();
    assert(logged());
    QStringList value = excludedSyncPaths;
    mutex.unlock();
    return value;
}

void Preferences::setExcludedSyncPaths(QStringList paths)
{
    mutex.lock();
    assert(logged());
    excludedSyncPaths = paths;
    if (!excludedSyncPaths.size())
    {
        settings->remove(excludedSyncPathsKey);
    }
    else
    {
        settings->setValue(excludedSyncPathsKey, excludedSyncPaths.join(QString::fromAscii("\n")));
    }

    settings->sync();
    mutex.unlock();
}

QStringList Preferences::getPreviousCrashes()
{
    mutex.lock();
    QStringList previousCrashes;
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }
    previousCrashes = settings->value(previousCrashesKey).toString().split(QString::fromAscii("\n", QString::SkipEmptyParts));
    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return previousCrashes;
}

void Preferences::setPreviousCrashes(QStringList crashes)
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    if (!crashes.size())
    {
        settings->remove(previousCrashesKey);
    }
    else
    {
        settings->setValue(previousCrashesKey, crashes.join(QString::fromAscii("\n")));
    }

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    settings->sync();
    mutex.unlock();
}

long long Preferences::getLastReboot()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    long long value = settings->value(lastRebootKey).toLongLong();

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return value;
}

void Preferences::setLastReboot(long long value)
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(lastRebootKey, value);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    settings->sync();
    mutex.unlock();
}

long long Preferences::getLastExit()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    long long value = settings->value(lastExitKey).toLongLong();

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return value;
}

void Preferences::setLastExit(long long value)
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(lastExitKey, value);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    settings->sync();
    mutex.unlock();
}

QString Preferences::getHttpsKey()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    QString value = settings->value(httpsKeyKey, defaultHttpsKey).toString();

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return value;
}

void Preferences::setHttpsKey(QString key)
{
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(httpsKeyKey, key);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    settings->sync();
}

QString Preferences::getHttpsCert()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    QString value = settings->value(httpsCertKey, defaultHttpsCert).toString();

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return value;
}

void Preferences::setHttpsCert(QString cert)
{
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(httpsCertKey, cert);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    settings->sync();
}

QString Preferences::getHttpsCertIntermediate()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    QString value = settings->value(httpsCertIntermediateKey, defaultHttpsCertIntermediate).toString();

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return value;
}

void Preferences::setHttpsCertIntermediate(QString intermediate)
{
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(httpsCertIntermediateKey, intermediate);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    settings->sync();
}

long long Preferences::getHttpsCertExpiration()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    long long value = settings->value(httpsCertExpirationKey, defaultHttpsCertExpiration).toLongLong();

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return value;
}

void Preferences::setHttpsCertExpiration(long long expiration)
{
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(httpsCertExpirationKey, expiration);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    settings->sync();
}

long long Preferences::lastPublicHandleTimestamp()
{
    mutex.lock();
    assert(logged());
    long long value = settings->value(lastPublicHandleTimestampKey, 0).toLongLong();
    mutex.unlock();
    return value;
}

MegaHandle Preferences::lastPublicHandle()
{
    mutex.lock();
    assert(logged());
    MegaHandle value = settings->value(lastPublicHandleKey, (unsigned long long) mega::INVALID_HANDLE).toULongLong();
    mutex.unlock();
    return value;
}

void Preferences::setLastPublicHandle(MegaHandle handle)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lastPublicHandleKey, (unsigned long long) handle);
    settings->setValue(lastPublicHandleTimestampKey, QDateTime::currentMSecsSinceEpoch());
    settings->sync();
    mutex.unlock();
}

int Preferences::getNumUsers()
{
    mutex.lock();
    assert(!logged());
    int value = settings->numChildGroups();
    mutex.unlock();
    return value;
}

void Preferences::enterUser(int i)
{
    mutex.lock();
    assert(!logged());
    assert(i < settings->numChildGroups());
    if (i < settings->numChildGroups())
    {
        settings->beginGroup(i);
    }

    readFolders();
    loadExcludedSyncNames();
    mutex.unlock();
}

void Preferences::leaveUser()
{
    mutex.lock();
    assert(logged());
    settings->endGroup();

    clearTemporalBandwidth();
    syncNames.clear();
    syncIDs.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    activeFolders.clear();
    temporaryInactiveFolders.clear();
    localFingerprints.clear();
    mutex.unlock();
}

void Preferences::unlink()
{
    mutex.lock();
    assert(logged());
    settings->remove(emailHashKey);
    settings->remove(privatePwKey);
    settings->remove(sessionKey);
    settings->endGroup();

    settings->remove(currentAccountKey);
    clearTemporalBandwidth();
    syncNames.clear();
    syncIDs.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    activeFolders.clear();
    temporaryInactiveFolders.clear();
    localFingerprints.clear();
    settings->sync();
    mutex.unlock();
    emit stateChanged();
}

bool Preferences::isCrashed()
{
    mutex.lock();
    bool value = settings->value(isCrashedKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setCrashed(bool value)
{
    mutex.lock();
    settings->setValue(isCrashedKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::getGlobalPaused()
{
    mutex.lock();
    bool value = settings->value(wasPausedKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setGlobalPaused(bool value)
{
    mutex.lock();
    settings->setValue(wasPausedKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::getUploadsPaused()
{
    mutex.lock();
    bool value = settings->value(wasUploadsPausedKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setUploadsPaused(bool value)
{
    mutex.lock();
    settings->setValue(wasUploadsPausedKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::getDownloadsPaused()
{
    mutex.lock();
    bool value = settings->value(wasDownloadsPausedKey, false).toBool();
    mutex.unlock();
    return value;
}

void Preferences::setDownloadsPaused(bool value)
{
    mutex.lock();
    settings->setValue(wasDownloadsPausedKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::lastStatsRequest()
{
    mutex.lock();
    long long value = settings->value(lastStatsRequestKey, 0).toLongLong();
    mutex.unlock();
    return value;
}

void Preferences::setLastStatsRequest(long long value)
{
    mutex.lock();
    settings->setValue(lastStatsRequestKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::fileVersioningDisabled()
{
    mutex.lock();
    assert(logged());
    bool result = settings->value(disableFileVersioningKey, false).toBool();
    mutex.unlock();
    return result;
}

void Preferences::disableFileVersioning(bool value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(disableFileVersioningKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::overlayIconsDisabled()
{
    mutex.lock();
    bool result = settings->value(disableOverlayIconsKey, false).toBool();
    mutex.unlock();
    return result;
}

void Preferences::disableOverlayIcons(bool value)
{
    mutex.lock();
    settings->setValue(disableOverlayIconsKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::leftPaneIconsDisabled()
{
    mutex.lock();
    bool result = settings->value(disableLeftPaneIconsKey, false).toBool();
    mutex.unlock();
    return result;
}

void Preferences::disableLeftPaneIcons(bool value)
{
    mutex.lock();
    settings->setValue(disableLeftPaneIconsKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::error()
{
    return errorFlag;
}

QString Preferences::getDataPath()
{
    mutex.lock();
    QString ret = dataPath;
    mutex.unlock();
    return ret;
}

void Preferences::clearTemporalBandwidth()
{
    isTempBandwidthValid = false;
    tempBandwidth = 0;
    tempBandwidthInterval = 6;
}

void Preferences::clearAll()
{
    mutex.lock();
    if (logged())
    {
        unlink();
    }

    settings->clear();
    settings->sync();
    mutex.unlock();
}

void Preferences::sync()
{
    settings->sync();
}

void Preferences::login(QString account)
{
    mutex.lock();
    logout();
    settings->setValue(currentAccountKey, account);
    settings->beginGroup(account);
    readFolders();
    loadExcludedSyncNames();
    int lastVersion = settings->value(lastVersionKey).toInt();
    if (lastVersion != Preferences::VERSION_CODE)
    {
        if ((lastVersion != 0) && (lastVersion < Preferences::VERSION_CODE))
        {
            emit updated(lastVersion);
        }
        settings->setValue(lastVersionKey, Preferences::VERSION_CODE);
    }
    settings->sync();
    mutex.unlock();
}

bool Preferences::logged()
{
    mutex.lock();
    bool value = !settings->isGroupEmpty();
    mutex.unlock();
    return value;
}

bool Preferences::hasEmail(QString email)
{
    mutex.lock();
    assert(!logged());
    bool value = settings->containsGroup(email);
    if (value)
    {
        settings->beginGroup(email);
        QString storedEmail = settings->value(emailKey).toString();
        value = !storedEmail.compare(email);
        if (!value)
        {
            settings->remove(QString::fromAscii(""));
        }
        settings->endGroup();
    }
    mutex.unlock();
    return value;
}

void Preferences::logout()
{
    mutex.lock();
    if (logged())
    {
        settings->endGroup();
    }
    clearTemporalBandwidth();
    syncNames.clear();
    syncIDs.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    activeFolders.clear();
    temporaryInactiveFolders.clear();
    localFingerprints.clear();
    mutex.unlock();
}

static bool caseInsensitiveLessThan(const QString &s1, const QString &s2)
{
    return s1.toLower() < s2.toLower();
}

void Preferences::loadExcludedSyncNames()
{
    mutex.lock();
    excludedSyncNames = settings->value(excludedSyncNamesKey).toString().split(QString::fromAscii("\n", QString::SkipEmptyParts));
    if (excludedSyncNames.size()==1 && excludedSyncNames.at(0).isEmpty())
    {
        excludedSyncNames.clear();
    }

    excludedSyncPaths = settings->value(excludedSyncPathsKey).toString().split(QString::fromAscii("\n", QString::SkipEmptyParts));
    if (excludedSyncPaths.size()==1 && excludedSyncPaths.at(0).isEmpty())
    {
        excludedSyncPaths.clear();
    }

    if (settings->value(lastVersionKey).toInt() < 108)
    {
        excludedSyncNames.clear();
        excludedSyncNames.append(QString::fromUtf8("Thumbs.db"));
        excludedSyncNames.append(QString::fromUtf8("desktop.ini"));
        excludedSyncNames.append(QString::fromUtf8("~*"));
        excludedSyncNames.append(QString::fromUtf8(".*"));
    }

    if (settings->value(lastVersionKey).toInt() < 3400)
    {
        excludedSyncNames.append(QString::fromUtf8("*~.*"));
        excludedSyncNames.append(QString::fromUtf8("*.sb-????????-??????"));
        excludedSyncNames.append(QString::fromUtf8("*.tmp"));
    }

    if (settings->value(lastVersionKey).toInt() < 2907)
    {
        //This string is no longer excluded by default since 2907
        excludedSyncNames.removeAll(QString::fromUtf8("Icon?"));
    }

    QSet<QString> excludedSyncNamesSet = QSet<QString>::fromList(excludedSyncNames);
    excludedSyncNames = excludedSyncNamesSet.toList();
    qSort(excludedSyncNames.begin(), excludedSyncNames.end(), caseInsensitiveLessThan);

    QSet<QString> excludedSyncPathsSet = QSet<QString>::fromList(excludedSyncPaths);
    excludedSyncPaths = excludedSyncPathsSet.toList();
    qSort(excludedSyncPaths.begin(), excludedSyncPaths.end(), caseInsensitiveLessThan);

    setExcludedSyncNames(excludedSyncNames);
    setExcludedSyncPaths(excludedSyncPaths);
    mutex.unlock();
}

void Preferences::readFolders()
{
    mutex.lock();
    assert(logged());
    syncNames.clear();
    syncIDs.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    activeFolders.clear();
    temporaryInactiveFolders.clear();
    localFingerprints.clear();

    settings->beginGroup(syncsGroupKey);
    int numSyncs = settings->numChildGroups();
    for (int i = 0; i < numSyncs; i++)
    {
        settings->beginGroup(QString::number(i));

        syncNames.append(settings->value(syncNameKey).toString());
        syncIDs.append(settings->value(syncIdKey, QUuid::createUuid().toString().toUpper()).toString());
        localFolders.append(settings->value(localFolderKey).toString());
        megaFolders.append(settings->value(megaFolderKey).toString());
        megaFolderHandles.append(settings->value(megaFolderHandleKey).toLongLong());
        activeFolders.append(settings->value(folderActiveKey, true).toBool());
        temporaryInactiveFolders.append(settings->value(temporaryInactiveKey, false).toBool());
        localFingerprints.append(settings->value(localFingerprintKey, 0).toLongLong());

        settings->endGroup();
    }
    settings->endGroup();
    mutex.unlock();
}

void Preferences::writeFolders()
{
    mutex.lock();
    assert(logged());

    settings->beginGroup(syncsGroupKey);

    settings->remove(QString::fromAscii(""));
    for (int i = 0; i < localFolders.size(); i++)
    {
        settings->beginGroup(QString::number(i));

        settings->setValue(syncNameKey, syncNames[i]);
        settings->setValue(syncIdKey, syncIDs[i]);
        settings->setValue(localFolderKey, localFolders[i]);
        settings->setValue(megaFolderKey, megaFolders[i]);
        settings->setValue(megaFolderHandleKey, megaFolderHandles[i]);
        settings->setValue(folderActiveKey, activeFolders[i]);
        settings->setValue(temporaryInactiveKey,temporaryInactiveFolders[i]);
        settings->setValue(localFingerprintKey, localFingerprints[i]);

        settings->endGroup();
    }

    settings->endGroup();
    settings->sync();
    mutex.unlock();
}

void Preferences::setBaseUrl(const QString &value)
{
    BASE_URL = value;
}

template<typename T>
void Preferences::overridePreference(const QSettings &settings, QString &&name, T &value)
{
    T previous = value;
    QVariant variant = settings.value(name, previous);
    value = variant.value<T>();
    if (previous != value)
    {
        qDebug() << "Preference " << name << " overridden: " << value;
    }
}

void Preferences::overridePreferences(const QSettings &settings)
{
    overridePreference(settings, QString::fromUtf8("OQ_DIALOG_INTERVAL_MS"), Preferences::OQ_DIALOG_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("OQ_NOTIFICATION_INTERVAL_MS"), Preferences::OQ_NOTIFICATION_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("ALMOST_OS_INTERVAL_MS"), Preferences::ALMOST_OS_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("OS_INTERVAL_MS"), Preferences::OS_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("USER_INACTIVITY_MS"), Preferences::USER_INACTIVITY_MS);

    overridePreference(settings, QString::fromUtf8("MIN_UPDATE_STATS_INTERVAL"), Preferences::MIN_UPDATE_STATS_INTERVAL);
    overridePreference(settings, QString::fromUtf8("MIN_UPDATE_CLEANING_INTERVAL_MS"), Preferences::MIN_UPDATE_CLEANING_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("MIN_UPDATE_NOTIFICATION_INTERVAL_MS"), Preferences::MIN_UPDATE_NOTIFICATION_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("MIN_REBOOT_INTERVAL_MS"), Preferences::MIN_REBOOT_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("MIN_EXTERNAL_NODES_WARNING_MS"), Preferences::MIN_EXTERNAL_NODES_WARNING_MS);
    overridePreference(settings, QString::fromUtf8("MIN_TRANSFER_NOTIFICATION_INTERVAL_MS"), Preferences::MIN_TRANSFER_NOTIFICATION_INTERVAL_MS);

    overridePreference(settings, QString::fromUtf8("MAX_FIRST_SYNC_DELAY_S"), Preferences::MAX_FIRST_SYNC_DELAY_S);
    overridePreference(settings, QString::fromUtf8("MIN_FIRST_SYNC_DELAY_S"), Preferences::MIN_FIRST_SYNC_DELAY_S);

    overridePreference(settings, QString::fromUtf8("UPDATE_INITIAL_DELAY_SECS"), Preferences::UPDATE_INITIAL_DELAY_SECS);
    overridePreference(settings, QString::fromUtf8("UPDATE_RETRY_INTERVAL_SECS"), Preferences::UPDATE_RETRY_INTERVAL_SECS);
    overridePreference(settings, QString::fromUtf8("UPDATE_TIMEOUT_SECS"), Preferences::UPDATE_TIMEOUT_SECS);
    overridePreference(settings, QString::fromUtf8("MAX_LOGIN_TIME_MS"), Preferences::MAX_LOGIN_TIME_MS);
    overridePreference(settings, QString::fromUtf8("PROXY_TEST_TIMEOUT_MS"), Preferences::PROXY_TEST_TIMEOUT_MS);
    overridePreference(settings, QString::fromUtf8("MAX_IDLE_TIME_MS"), Preferences::MAX_IDLE_TIME_MS);
    overridePreference(settings, QString::fromUtf8("MAX_COMPLETED_ITEMS"), Preferences::MAX_COMPLETED_ITEMS);
}
