#include "Preferences.h"
#include "platform/Platform.h"

#include <QDesktopServices>
#include <assert.h>

using namespace mega;

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

const char Preferences::CLIENT_KEY[] = "FhMgXbqb";
const char Preferences::USER_AGENT[] = "MEGAsync/4.3.3.0";
const int Preferences::VERSION_CODE = 4303;
const int Preferences::BUILD_ID = 5;
// Do not change the location of VERSION_STRING, create_tarball.sh parses this file
const QString Preferences::VERSION_STRING = QString::fromAscii("4.3.3");
QString Preferences::SDK_ID = QString::fromAscii("b2948c7");
const QString Preferences::CHANGELOG = QString::fromUtf8(QT_TR_NOOP(
    "- New customized message boxes.\n"
    "- Clean retroactive logs when logout.\n"
    "- Fixed issues that disable Finder extension under some circumstances.\n"
    "- Email verification for locked accounts.\n"
    "- SMS verification for locked accounts.\n"
    "- Support for affiliate program.\n"
    "- Included option to send logs from crash report dialog.\n"
    "- Other UI fixes and adjustments.\n"
    "- Other performance improvements and adjustments."));

const QString Preferences::TRANSLATION_FOLDER = QString::fromAscii("://translations/");
const QString Preferences::TRANSLATION_PREFIX = QString::fromAscii("MEGASyncStrings_");

int Preferences::STATE_REFRESH_INTERVAL_MS        = 10000;
int Preferences::FINISHED_TRANSFER_REFRESH_INTERVAL_MS        = 10000;

int Preferences::MAX_FIRST_SYNC_DELAY_S = 120; // Max delay time to wait for local paths before trying to restore syncs
int Preferences::MIN_FIRST_SYNC_DELAY_S = 40; // Min delay time to wait for local paths before trying to restore syncs

long long Preferences::OQ_DIALOG_INTERVAL_MS = 604800000; // 7 days
long long Preferences::OQ_NOTIFICATION_INTERVAL_MS = 129600000; // 36 hours
long long Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS = 259200000; // 72 hours
long long Preferences::OQ_UI_MESSAGE_INTERVAL_MS = 129600000; // 36 hours
long long Preferences::PAYWALL_NOTIFICATION_INTERVAL_MS = 86400000; //24 hours
long long Preferences::USER_INACTIVITY_MS = 20000; // 20 secs

std::chrono::minutes Preferences::OVERQUOTA_DIALOG_DISABLE_DURATION{std::chrono::hours(7*24)};
std::chrono::minutes Preferences::OVER_QUOTA_OS_NOTIFICATION_DISABLE_DURATION{std::chrono::hours(36)};
std::chrono::minutes Preferences::ALMOST_OVER_QUOTA_UI_MESSAGE_DISABLE_DURATION{std::chrono::hours(72)};
std::chrono::minutes Preferences::OVER_QUOTA_UI_MESSAGE_DISABLE_DURATION{std::chrono::hours(36)};
std::chrono::minutes Preferences::ALMOST_OVER_QUOTA_OS_NOTIFICATION_DISABLE_DURATION{std::chrono::hours(36)};

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
    #ifdef _WIN64
        const QString Preferences::UPDATE_CHECK_URL             = QString::fromUtf8("http://g.static.mega.co.nz/upd/wsync64/v.txt");
    #else
        const QString Preferences::UPDATE_CHECK_URL             = QString::fromUtf8("http://g.static.mega.co.nz/upd/wsync/v.txt");
    #endif
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
const QString Preferences::currentAccountStatusKey  = QString::fromAscii("currentAccountStatus");
const QString Preferences::needsFetchNodesKey       = QString::fromAscii("needsFetchNodes");
const QString Preferences::emailKey                 = QString::fromAscii("email");
const QString Preferences::firstNameKey             = QString::fromAscii("firstName");
const QString Preferences::lastNameKey              = QString::fromAscii("lastName");
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
const QString Preferences::payWallNotificationExecutionKey = QString::fromAscii("payWallNotificationExecution");
const QString Preferences::almostOverStorageDismissExecutionKey = QString::fromAscii("almostOverStorageDismissExecution");
const QString Preferences::overStorageDismissExecutionKey = QString::fromAscii("overStorageDismissExecution");
const QString Preferences::storageStateQKey = QString::fromAscii("storageStopLight");
const QString Preferences::businessStateQKey = QString::fromAscii("businessState");
const QString Preferences::blockedStateQKey = QString::fromAscii("blockedState");

const QString Preferences::transferOverQuotaDialogDisabledUntilKey = QString::fromAscii("transferOverQuotaDialogDisabledUntil");
const QString Preferences::transferOverQuotaOsNotificationDisabledUntilKey = QString::fromAscii("transferOverQuotaOsNotificationDisabledUntil");
const QString Preferences::transferAlmostOverQuotaOsNotificationDisabledUntilKey = QString::fromAscii("transferAlmostOverQuotaOsNotificationDisabledUntil");
const QString Preferences::transferAlmostOverQuotaUiAlertDisabledUntilKey = QString::fromAscii("transferAlmostOverQuotaUiAlertDisabledUntil");
const QString Preferences::transferOverQuotaUiAlertDisableUntilKey = QString::fromAscii("transferOverQuotaUiAlertDisableUntil");

const QString Preferences::transferOverQuotaSyncDialogDisabledUntilKey = QString::fromAscii("transferOverQuotaSyncDialogDisabledUntil");
const QString Preferences::transferOverQuotaDownloadsDialogDisabledUntilKey = QString::fromAscii("transferOverQuotaDownloadsDialogDisabledUntil");
const QString Preferences::transferOverQuotaImportLinksDialogDisabledUntilKey = QString::fromAscii("transferOverQuotaImportLinksDialogDisabledUntil");
const QString Preferences::transferOverQuotaStreamDialogDisabledUntilKey = QString::fromAscii("transferOverQuotaStreamDialogDisabledUntil");
const QString Preferences::storageOverQuotaUploadsDialogDisabledUntilKey = QString::fromAscii("storageOverQuotaUploadsDialogDisabledUntil");
const QString Preferences::storageOverQuotaSyncsDialogDisabledUntilKey = QString::fromAscii("storageOverQuotaSyncsDialogDisabledUntil");
const QString Preferences::storageAndTransferOverQuotaSyncDialogDisabledUntilKey = QString::fromAscii("storageAndTransferOverQuotaSyncDialogDisabledUntil");

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
const QString Preferences::lastPublicHandleTypeKey = QString::fromAscii("lastPublicHandleType");

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

const int  Preferences::defaultAccountStatus      = STATE_NOT_INITIATED;
const bool  Preferences::defaultNeedsFetchNodes   = false;

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
    lastTransferNotification = 0;
    clearTemporalBandwidth();
}

QString Preferences::email()
{
    mutex.lock();
    assert(logged());
    QString value = getValue<QString>(emailKey);
    mutex.unlock();
    return value;
}

void Preferences::setEmail(QString email)
{
    mutex.lock();
    login(email);
    settings->setValue(emailKey, email);
    setCachedValue(emailKey, email);
    settings->sync();
    mutex.unlock();
    emit stateChanged();
}

QString Preferences::firstName()
{
    mutex.lock();
    assert(logged());
    QString value = getValue<QString>(firstNameKey, QString());
    mutex.unlock();
    return value;
}

void Preferences::setFirstName(QString firstName)
{
    mutex.lock();
    settings->setValue(firstNameKey, firstName);
    setCachedValue(firstNameKey, firstName);
    settings->sync();
    mutex.unlock();
}

QString Preferences::lastName()
{
    mutex.lock();
    assert(logged());
    QString value = getValue<QString>(lastNameKey, QString());
    mutex.unlock();
    return value;
}

void Preferences::setLastName(QString lastName)
{
    mutex.lock();
    settings->setValue(lastNameKey, lastName);
    setCachedValue(lastNameKey, lastName);
    settings->sync();
    mutex.unlock();
}

void Preferences::setSession(QString session)
{
    mutex.lock();
    storeSessionInGeneral(session);
    settings->sync();
    mutex.unlock();
}

void Preferences::setSessionInUserGroup(QString session)
{
    mutex.lock();
    assert(logged());
    settings->setValue(sessionKey, session);
    setCachedValue(sessionKey, session);
    settings->sync();
    mutex.unlock();
}

void Preferences::storeSessionInGeneral(QString session)
{
    mutex.lock();

    QString currentAccount;
    if (logged())
    {
        settings->setValue(sessionKey, session); //store in user group too (for backwards compatibility)
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(sessionKey, session);
    setCachedValue(sessionKey, session);
    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    settings->sync();
    mutex.unlock();
}

QString Preferences::getSessionInGeneral()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    QString value = getValue<QString>(sessionKey);
    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    mutex.unlock();
    return value;
}

QString Preferences::getSession()
{
    mutex.lock();
    QString value;
    if (logged())
    {
        value = settings->value(sessionKey).toString(); // for MEGAsync prior unfinished fetchnodes resumable sessions (<=4.3.1)
    }

    if (value.isEmpty() && needsFetchNodesInGeneral())
    {
        value = getSessionInGeneral(); // for MEGAsync with unfinished fetchnodes resumable sessions (>4.3.1)
    }

    mutex.unlock();
    return value;
}

unsigned long long Preferences::transferIdentifier()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(transferIdentifierKey, defaultTransferIdentifier);
    value++;
    settings->setValue(transferIdentifierKey, value);
    setCachedValue(transferIdentifierKey, value);
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
    long long value = getValue<long long>(totalStorageKey);
    mutex.unlock();
    return value;
}

void Preferences::setTotalStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(totalStorageKey, value);
    setCachedValue(totalStorageKey, value);

    mutex.unlock();
}

long long Preferences::usedStorage()
{
    mutex.lock();
    assert(logged());

    long long value = getValue<long long>(usedStorageKey);
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
    setCachedValue(usedStorageKey, value);
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
    long long value = getValue<long long>(cloudDriveStorageKey);
    mutex.unlock();
    return value;
}

void Preferences::setCloudDriveStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(cloudDriveStorageKey, value);
    setCachedValue(cloudDriveStorageKey, value);
    mutex.unlock();
}

long long Preferences::inboxStorage()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(inboxStorageKey);
    mutex.unlock();
    return value;
}

void Preferences::setInboxStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inboxStorageKey, value);
    setCachedValue(inboxStorageKey, value);
    mutex.unlock();
}

long long Preferences::rubbishStorage()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(rubbishStorageKey);
    mutex.unlock();
    return value;
}

void Preferences::setRubbishStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(rubbishStorageKey, value);
    setCachedValue(rubbishStorageKey, value);
    mutex.unlock();
}

long long Preferences::inShareStorage()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(inShareStorageKey);
    mutex.unlock();
    return value;
}

void Preferences::setInShareStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inShareStorageKey, value);
    setCachedValue(inShareStorageKey, value);
    mutex.unlock();
}

long long Preferences::versionsStorage()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(versionsStorageKey);
    mutex.unlock();
    return value;
}

void Preferences::setVersionsStorage(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(versionsStorageKey, value);
    setCachedValue(versionsStorageKey, value);
    mutex.unlock();
}

long long Preferences::cloudDriveFiles()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(cloudDriveFilesKey);
    mutex.unlock();
    return value;
}

void Preferences::setCloudDriveFiles(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(cloudDriveFilesKey, value);
    setCachedValue(cloudDriveFilesKey, value);
    mutex.unlock();
}

long long Preferences::inboxFiles()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(inboxFilesKey);
    mutex.unlock();
    return value;
}

void Preferences::setInboxFiles(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inboxFilesKey, value);
    setCachedValue(inboxFilesKey, value);
    mutex.unlock();
}

long long Preferences::rubbishFiles()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(rubbishFilesKey);
    mutex.unlock();
    return value;
}

void Preferences::setRubbishFiles(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(rubbishFilesKey, value);
    setCachedValue(rubbishFilesKey, value);
    mutex.unlock();
}

long long Preferences::inShareFiles()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(inShareFilesKey);
    mutex.unlock();
    return value;
}

void Preferences::setInShareFiles(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inShareFilesKey, value);
    setCachedValue(inShareFilesKey, value);
    mutex.unlock();
}

long long Preferences::cloudDriveFolders()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(cloudDriveFoldersKey);
    mutex.unlock();
    return value;
}

void Preferences::setCloudDriveFolders(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(cloudDriveFoldersKey, value);
    setCachedValue(cloudDriveFoldersKey, value);
    mutex.unlock();
}

long long Preferences::inboxFolders()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(inboxFoldersKey);
    mutex.unlock();
    return value;
}

void Preferences::setInboxFolders(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inboxFoldersKey, value);
    setCachedValue(inboxFoldersKey, value);
    mutex.unlock();
}

long long Preferences::rubbishFolders()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(rubbishFoldersKey);
    mutex.unlock();
    return value;
}

void Preferences::setRubbishFolders(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(rubbishFoldersKey, value);
    setCachedValue(rubbishFoldersKey, value);
    mutex.unlock();
}

long long Preferences::inShareFolders()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(inShareFoldersKey);
    mutex.unlock();
    return value;
}

void Preferences::setInShareFolders(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(inShareFoldersKey, value);
    setCachedValue(inShareFoldersKey, value);
    mutex.unlock();
}

long long Preferences::totalBandwidth()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(totalBandwidthKey);
    mutex.unlock();
    return value;
}

void Preferences::setTotalBandwidth(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(totalBandwidthKey, value);
    setCachedValue(totalBandwidthKey, value);
    mutex.unlock();
}

int Preferences::bandwidthInterval()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(usedBandwidthIntervalKey);
    mutex.unlock();
    return value;
}

void Preferences::setBandwidthInterval(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(usedBandwidthIntervalKey, value);
    setCachedValue(usedBandwidthIntervalKey, value);
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
    mutex.lock();
    assert(logged());
    long long overStorageDialogExecution = getValue<long long>(overStorageDialogExecutionKey, defaultTimeStamp);
    mutex.unlock();
    return overStorageDialogExecution;
}

void Preferences::setOverStorageDialogExecution(long long timestamp)
{
    mutex.lock();
    assert(logged());
    settings->setValue(overStorageDialogExecutionKey, timestamp);
    setCachedValue(overStorageDialogExecutionKey, timestamp);
    mutex.unlock();
}

long long Preferences::getOverStorageNotificationExecution()
{
    mutex.lock();
    assert(logged());
    long long overStorageNotificationExecution = getValue<long long>(overStorageNotificationExecutionKey, defaultTimeStamp);
    mutex.unlock();
    return overStorageNotificationExecution;
}

void Preferences::setOverStorageNotificationExecution(long long timestamp)
{
    mutex.lock();
    assert(logged());
    settings->setValue(overStorageNotificationExecutionKey, timestamp);
    setCachedValue(overStorageNotificationExecutionKey, timestamp);
    mutex.unlock();
}

long long Preferences::getAlmostOverStorageNotificationExecution()
{
    mutex.lock();
    assert(logged());
    long long almostOverStorageNotificationExecution = getValue<long long>(almostOverStorageNotificationExecutionKey, defaultTimeStamp);
    mutex.unlock();
    return almostOverStorageNotificationExecution;
}

void Preferences::setAlmostOverStorageNotificationExecution(long long timestamp)
{
    mutex.lock();
    assert(logged());
    settings->setValue(almostOverStorageNotificationExecutionKey, timestamp);
    setCachedValue(almostOverStorageNotificationExecutionKey, timestamp);
    mutex.unlock();
}

long long Preferences::getPayWallNotificationExecution()
{
    mutex.lock();
    assert(logged());
    long long payWallNotificationExecution = getValue<long long>(payWallNotificationExecutionKey, defaultTimeStamp);
    mutex.unlock();
    return payWallNotificationExecution;
}

void Preferences::setPayWallNotificationExecution(long long timestamp)
{
    mutex.lock();
    assert(logged());
    settings->setValue(payWallNotificationExecutionKey, timestamp);
    setCachedValue(payWallNotificationExecutionKey, timestamp);
    mutex.unlock();
}

long long Preferences::getAlmostOverStorageDismissExecution()
{
    mutex.lock();
    assert(logged());
    long long almostOverStorageDismissExecution = getValue<long long>(almostOverStorageDismissExecutionKey, defaultTimeStamp);
    mutex.unlock();
    return almostOverStorageDismissExecution;
}

void Preferences::setAlmostOverStorageDismissExecution(long long timestamp)
{
    mutex.lock();
    assert(logged());
    settings->setValue(almostOverStorageDismissExecutionKey, timestamp);
    setCachedValue(almostOverStorageDismissExecutionKey, timestamp);
    mutex.unlock();
}

long long Preferences::getOverStorageDismissExecution()
{
    mutex.lock();
    assert(logged());
    long long overStorageDismissExecution = getValue<long long>(overStorageDismissExecutionKey, defaultTimeStamp);
    mutex.unlock();
    return overStorageDismissExecution;
}

void Preferences::setOverStorageDismissExecution(long long timestamp)
{
    mutex.lock();
    assert(logged());
    settings->setValue(overStorageDismissExecutionKey, timestamp);
    setCachedValue(overStorageDismissExecutionKey, timestamp);
    mutex.unlock();
}

std::chrono::system_clock::time_point getTimePoint(const QString& key, EncryptedSettings * settings)
{
    constexpr auto defaultTimePoint{0};
    auto value{settings->value(key, static_cast<long long>(defaultTimePoint)).toLongLong()};
    std::chrono::milliseconds durationMillis(value);
    return std::chrono::system_clock::time_point{durationMillis};
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaDialogDisabledUntil()
{
    if(transferOverQuotaDialogDisabledUntil != std::chrono::system_clock::time_point())
    {
        return transferOverQuotaDialogDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    transferOverQuotaDialogDisabledUntil = getTimePoint(transferOverQuotaDialogDisabledUntilKey, settings);
    return transferOverQuotaDialogDisabledUntil;
}

void Preferences::setTransferOverQuotaDialogDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    transferOverQuotaDialogDisabledUntil = timepoint;
    auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());
    settings->setValue(transferOverQuotaDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(transferOverQuotaDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaOsNotificationDisabledUntil()
{
    if(transferOverQuotaOsNotificationDisabledUntil != std::chrono::system_clock::time_point())
    {
        return transferOverQuotaOsNotificationDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    transferOverQuotaOsNotificationDisabledUntil = getTimePoint(transferOverQuotaOsNotificationDisabledUntilKey, settings);
    return transferOverQuotaOsNotificationDisabledUntil;
}

void Preferences::setTransferOverQuotaOsNotificationDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    transferOverQuotaOsNotificationDisabledUntil = timepoint;
    auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());
    settings->setValue(transferOverQuotaOsNotificationDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(transferOverQuotaOsNotificationDisabledUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getTransferAlmostOverQuotaOsNotificationDisabledUntil()
{
    if(transferAlmostOverQuotaOsNotificationDisabledUntil != std::chrono::system_clock::time_point())
    {
        return transferAlmostOverQuotaOsNotificationDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    transferAlmostOverQuotaOsNotificationDisabledUntil = getTimePoint(transferAlmostOverQuotaOsNotificationDisabledUntilKey, settings);
    return transferAlmostOverQuotaOsNotificationDisabledUntil;
}

void Preferences::setTransferAlmostOverQuotaOsNotificationDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    transferAlmostOverQuotaOsNotificationDisabledUntil = timepoint;
    auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());
    settings->setValue(transferAlmostOverQuotaOsNotificationDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(transferAlmostOverQuotaOsNotificationDisabledUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getTransferAlmostOverQuotaUiAlertDisableUntil()
{
    if(transferAlmostOverQuotaUiAlertDisabledUntil != std::chrono::system_clock::time_point())
    {
        return transferAlmostOverQuotaUiAlertDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    transferAlmostOverQuotaUiAlertDisabledUntil = getTimePoint(transferAlmostOverQuotaUiAlertDisabledUntilKey, settings);
    return transferAlmostOverQuotaUiAlertDisabledUntil;
}

void Preferences::setTransferAlmostOverQuotaUiAlertDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    transferAlmostOverQuotaUiAlertDisabledUntil = timepoint;
    auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());
    settings->setValue(transferAlmostOverQuotaUiAlertDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(transferAlmostOverQuotaUiAlertDisabledUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaUiAlertDisabledUntil()
{
    if(transferOverQuotaUiAlertDisableUntil != std::chrono::system_clock::time_point())
    {
        return transferOverQuotaUiAlertDisableUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    transferOverQuotaUiAlertDisableUntil = getTimePoint(transferOverQuotaUiAlertDisableUntilKey, settings);
    return transferOverQuotaUiAlertDisableUntil;
}

void Preferences::setTransferOverQuotaUiAlertDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    transferOverQuotaUiAlertDisableUntil = timepoint;
    auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());
    settings->setValue(transferOverQuotaUiAlertDisableUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(transferOverQuotaUiAlertDisableUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaSyncDialogDisableUntil()
{
    if(transferOverQuotaSyncDialogDisabledUntil != std::chrono::system_clock::time_point())
    {
        return transferOverQuotaSyncDialogDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    transferOverQuotaSyncDialogDisabledUntil = getTimePoint(transferOverQuotaSyncDialogDisabledUntilKey, settings);
    return transferOverQuotaSyncDialogDisabledUntil;
}

void Preferences::setTransferOverQuotaSyncDialogDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    transferOverQuotaSyncDialogDisabledUntil = timepoint;
    auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());

    settings->setValue(transferOverQuotaSyncDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(transferOverQuotaSyncDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaDownloadsDialogDisabledUntil()
{
    if(transferOverQuotaDownloadsDialogDisabledUntil != std::chrono::system_clock::time_point())
    {
        return transferOverQuotaDownloadsDialogDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    transferOverQuotaDownloadsDialogDisabledUntil = getTimePoint(transferOverQuotaDownloadsDialogDisabledUntilKey, settings);
    return transferOverQuotaDownloadsDialogDisabledUntil;
}

void Preferences::setTransferOverQuotaDownloadsDialogDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    transferOverQuotaDownloadsDialogDisabledUntil = timepoint;
    const auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());    
    settings->setValue(transferOverQuotaDownloadsDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(transferOverQuotaDownloadsDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaImportLinksDialogDisabledUntil()
{
    if(transferOverQuotaImportLinksDialogDisabledUntil != std::chrono::system_clock::time_point())
    {
        return transferOverQuotaImportLinksDialogDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    transferOverQuotaImportLinksDialogDisabledUntil = getTimePoint(transferOverQuotaImportLinksDialogDisabledUntilKey, settings);
    return transferOverQuotaImportLinksDialogDisabledUntil;
}

void Preferences::setTransferOverQuotaImportLinksDialogDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    transferOverQuotaImportLinksDialogDisabledUntil = timepoint;
    const auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());
    settings->setValue(transferOverQuotaImportLinksDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(transferOverQuotaImportLinksDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaStreamDialogDisabledUntil()
{
    if(transferOverQuotaStreamDialogDisabledUntil != std::chrono::system_clock::time_point())
    {
        return transferOverQuotaStreamDialogDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    transferOverQuotaStreamDialogDisabledUntil = getTimePoint(transferOverQuotaStreamDialogDisabledUntilKey, settings);
    return transferOverQuotaStreamDialogDisabledUntil;
}

void Preferences::setTransferOverQuotaStreamDialogDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    transferOverQuotaStreamDialogDisabledUntil = timepoint;
    const auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());
    settings->setValue(transferOverQuotaStreamDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(transferOverQuotaStreamDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getStorageOverQuotaUploadsDialogDisabledUntil()
{
    if(storageOverQuotaUploadsDialogDisabledUntil != std::chrono::system_clock::time_point())
    {
        return storageOverQuotaUploadsDialogDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    storageOverQuotaUploadsDialogDisabledUntil = getTimePoint(storageOverQuotaUploadsDialogDisabledUntilKey, settings);
    return storageOverQuotaUploadsDialogDisabledUntil;
}

void Preferences::setStorageOverQuotaUploadsDialogDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    storageOverQuotaUploadsDialogDisabledUntil = timepoint;
    const auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());
    settings->setValue(storageOverQuotaUploadsDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(storageOverQuotaUploadsDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getStorageOverQuotaSyncsDialogDisabledUntil()
{
    if(storageOverQuotaSyncsDialogDisabledUntil != std::chrono::system_clock::time_point())
    {
        return storageOverQuotaSyncsDialogDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    storageOverQuotaSyncsDialogDisabledUntil = getTimePoint(storageOverQuotaSyncsDialogDisabledUntilKey, settings);
    return storageOverQuotaSyncsDialogDisabledUntil;
}

void Preferences::setStorageOverQuotaSyncsDialogDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    storageOverQuotaSyncsDialogDisabledUntil = timepoint;
    const auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());
    settings->setValue(storageOverQuotaSyncsDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(storageOverQuotaSyncsDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
}

std::chrono::system_clock::time_point Preferences::getStorageAndTransferOverQuotaSyncDialogDisabledUntil()
{
    if(storageAndTransferOverQuotaSyncDialogDisabledUntil != std::chrono::system_clock::time_point())
    {
        return storageAndTransferOverQuotaSyncDialogDisabledUntil;
    }

    QMutexLocker locker(&mutex);
    assert(logged());
    storageAndTransferOverQuotaSyncDialogDisabledUntil = getTimePoint(storageAndTransferOverQuotaSyncDialogDisabledUntilKey, settings);
    return storageAndTransferOverQuotaSyncDialogDisabledUntil;
}

void Preferences::setStorageAndTransferOverQuotaSyncDialogDisabledUntil(std::chrono::system_clock::time_point timepoint)
{
    storageAndTransferOverQuotaSyncDialogDisabledUntil = timepoint;
    const auto timePointMillis{std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count()};
    QMutexLocker locker(&mutex);
    assert(logged());
    settings->setValue(storageAndTransferOverQuotaSyncDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
    setCachedValue(storageAndTransferOverQuotaSyncDialogDisabledUntilKey, static_cast<long long>(timePointMillis));
}

int Preferences::getStorageState()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(storageStateQKey, MegaApi::STORAGE_STATE_UNKNOWN);
    mutex.unlock();
    return value;
}

void Preferences::setStorageState(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(storageStateQKey, value);
    setCachedValue(storageStateQKey, value);
    mutex.unlock();
}

int Preferences::getBusinessState()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(businessStateQKey, -2);
    mutex.unlock();
    return value;
}

void Preferences::setBusinessState(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(businessStateQKey, value);
    setCachedValue(businessStateQKey, value);
    mutex.unlock();
}

int Preferences::getBlockedState()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(blockedStateQKey, -2);
    mutex.unlock();
    return value;
}

void Preferences::setBlockedState(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(blockedStateQKey, value);
    setCachedValue(blockedStateQKey, value);
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
    long long value = getValue<long long>(usedBandwidthKey);
    mutex.unlock();
    return value;
}

void Preferences::setUsedBandwidth(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(usedBandwidthKey, value);
    setCachedValue(usedBandwidthKey, value);
    mutex.unlock();
}

int Preferences::accountType()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(accountTypeKey);
    mutex.unlock();
    return value;
}

void Preferences::setAccountType(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(accountTypeKey, value);
    setCachedValue(accountTypeKey, value);
    mutex.unlock();
}

long long Preferences::proExpirityTime()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(proExpirityTimeKey);
    mutex.unlock();
    return value;
}

void Preferences::setProExpirityTime(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(proExpirityTimeKey, value);
    setCachedValue(proExpirityTimeKey, value);
    mutex.unlock();
}

bool Preferences::showNotifications()
{
    mutex.lock();
    bool value = getValue<bool>(showNotificationsKey, defaultShowNotifications);
    mutex.unlock();
    return value;
}

void Preferences::setShowNotifications(bool value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(showNotificationsKey, value);
    setCachedValue(showNotificationsKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::startOnStartup()
{
    mutex.lock();
    bool value = getValue<bool>(startOnStartupKey, defaultStartOnStartup);
    mutex.unlock();
    return value;
}

void Preferences::setStartOnStartup(bool value)
{
    mutex.lock();
    settings->setValue(startOnStartupKey, value);
    setCachedValue(startOnStartupKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::usingHttpsOnly()
{
    mutex.lock();
    bool value = getValue<bool>(useHttpsOnlyKey, defaultUseHttpsOnly);
    mutex.unlock();
    return value;
}

void Preferences::setUseHttpsOnly(bool value)
{
    mutex.lock();
    settings->setValue(useHttpsOnlyKey, value);
    setCachedValue(useHttpsOnlyKey, value);
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
    bool value = getValue<bool>(SSLcertificateExceptionKey, defaultSSLcertificateException);
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
    setCachedValue(SSLcertificateExceptionKey, value);
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
    int value = getValue<int>(transferDownloadMethodKey, defaultTransferDownloadMethod);
    mutex.unlock();
    return value;
}

void Preferences::setTransferDownloadMethod(int value)
{
    mutex.lock();
    settings->setValue(transferDownloadMethodKey, value);
    setCachedValue(transferDownloadMethodKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::transferUploadMethod()
{
    mutex.lock();
    int value = getValue<int>(transferUploadMethodKey, defaultTransferUploadMethod);
    mutex.unlock();
    return value;
}

void Preferences::setTransferUploadMethod(int value)
{
    mutex.lock();
    settings->setValue(transferUploadMethodKey, value);
    setCachedValue(transferUploadMethodKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::language()
{
    mutex.lock();
    QString value = getValue<QString>(languageKey, QLocale::system().name());
    mutex.unlock();
    return value;
}

void Preferences::setLanguage(QString &value)
{
    mutex.lock();
    settings->setValue(languageKey, value);
    setCachedValue(languageKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::updateAutomatically()
{
    mutex.lock();
    bool value = getValue<bool>(updateAutomaticallyKey, defaultUpdateAutomatically);
    mutex.unlock();
    return value;
}

void Preferences::setUpdateAutomatically(bool value)
{
    mutex.lock();
    settings->setValue(updateAutomaticallyKey, value);
    setCachedValue(updateAutomaticallyKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::hasDefaultUploadFolder()
{
    mutex.lock();
    bool value = getValue<bool>(hasDefaultUploadFolderKey, uploadFolder() != 0);
    mutex.unlock();
    return value;
}

bool Preferences::hasDefaultDownloadFolder()
{
    mutex.lock();
    bool value = getValue<bool>(hasDefaultDownloadFolderKey, !downloadFolder().isEmpty());
    mutex.unlock();
    return value;
}

bool Preferences::hasDefaultImportFolder()
{
    mutex.lock();
    bool value = getValue<bool>(hasDefaultImportFolderKey, importFolder() != 0);
    mutex.unlock();
    return value;
}

void Preferences::setHasDefaultUploadFolder(bool value)
{
    mutex.lock();
    settings->setValue(hasDefaultUploadFolderKey, value);
    setCachedValue(hasDefaultUploadFolderKey, value);
    settings->sync();
    mutex.unlock();
}

void Preferences::setHasDefaultDownloadFolder(bool value)
{
    mutex.lock();
    settings->setValue(hasDefaultDownloadFolderKey, value);
    setCachedValue(hasDefaultDownloadFolderKey, value);
    settings->sync();
    mutex.unlock();
}

void Preferences::setHasDefaultImportFolder(bool value)
{
    mutex.lock();
    settings->setValue(hasDefaultImportFolderKey, value);
    setCachedValue(hasDefaultImportFolderKey, value);
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

int Preferences::accountStateInGeneral()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    int value = getValue<int>(currentAccountStatusKey, defaultAccountStatus);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    mutex.unlock();
    return value;
}

void Preferences::setAccountStateInGeneral(int value)
{
    mutex.lock();

    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(currentAccountStatusKey, value);
    setCachedValue(currentAccountStatusKey, value);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    settings->sync();
    mutex.unlock();
}


bool Preferences::needsFetchNodesInGeneral()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    bool value = getValue<bool>(needsFetchNodesKey, defaultNeedsFetchNodes);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    mutex.unlock();
    return value;
}

void Preferences::setNeedsFetchNodesInGeneral(bool value)
{
    mutex.lock();

    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->setValue(needsFetchNodesKey, value);
    setCachedValue(needsFetchNodesKey, value);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    settings->sync();
    mutex.unlock();
}


int Preferences::uploadLimitKB()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(uploadLimitKBKey, defaultUploadLimitKB);
    mutex.unlock();
    return value;
}

void Preferences::setUploadLimitKB(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(uploadLimitKBKey, value);
    setCachedValue(uploadLimitKBKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::downloadLimitKB()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(downloadLimitKBKey, defaultDownloadLimitKB);
    mutex.unlock();
    return value;
}

int Preferences::parallelUploadConnections()
{
    mutex.lock();
    int value = getValue<int>(parallelUploadConnectionsKey, defaultParallelUploadConnections);
    mutex.unlock();
    return value;
}

int Preferences::parallelDownloadConnections()
{
    mutex.lock();
    int value = getValue<int>(parallelDownloadConnectionsKey, defaultParallelDownloadConnections);
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
    setCachedValue(parallelUploadConnectionsKey, value);
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
    setCachedValue(parallelDownloadConnectionsKey, value);
    settings->sync();
    mutex.unlock();
}

void Preferences::setDownloadLimitKB(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(downloadLimitKBKey, value);
    setCachedValue(downloadLimitKBKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::upperSizeLimit()
{
    mutex.lock();
    bool value = getValue<bool>(upperSizeLimitKey, defaultUpperSizeLimit);
    mutex.unlock();
    return value;
}

void Preferences::setUpperSizeLimit(bool value)
{
    mutex.lock();
    settings->setValue(upperSizeLimitKey, value);
    setCachedValue(upperSizeLimitKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::upperSizeLimitValue()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(upperSizeLimitValueKey, defaultUpperSizeLimitValue);
    mutex.unlock();
    return value;
}
void Preferences::setUpperSizeLimitValue(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(upperSizeLimitValueKey, value);
    setCachedValue(upperSizeLimitValueKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::cleanerDaysLimit()
{
    mutex.lock();
    bool value = getValue<bool>(cleanerDaysLimitKey, defaultCleanerDaysLimit);
    mutex.unlock();
    return value;
}

void Preferences::setCleanerDaysLimit(bool value)
{
    mutex.lock();
    settings->setValue(cleanerDaysLimitKey, value);
    setCachedValue(cleanerDaysLimitKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::cleanerDaysLimitValue()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(cleanerDaysLimitValueKey, defaultCleanerDaysLimitValue);
    mutex.unlock();
    return value;
}
void Preferences::setCleanerDaysLimitValue(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(cleanerDaysLimitValueKey, value);
    setCachedValue(cleanerDaysLimitValueKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::upperSizeLimitUnit()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(upperSizeLimitUnitKey, defaultUpperSizeLimitUnit);
    mutex.unlock();
    return value;
}
void Preferences::setUpperSizeLimitUnit(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(upperSizeLimitUnitKey, value);
    setCachedValue(upperSizeLimitUnitKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::lowerSizeLimit()
{
    mutex.lock();
    bool value = getValue<bool>(lowerSizeLimitKey, defaultLowerSizeLimit);
    mutex.unlock();
    return value;
}

void Preferences::setLowerSizeLimit(bool value)
{
    mutex.lock();
    settings->setValue(lowerSizeLimitKey, value);
    setCachedValue(lowerSizeLimitKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::lowerSizeLimitValue()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(lowerSizeLimitValueKey, defaultLowerSizeLimitValue);
    mutex.unlock();
    return value;
}
void Preferences::setLowerSizeLimitValue(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lowerSizeLimitValueKey, value);
    setCachedValue(lowerSizeLimitValueKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::lowerSizeLimitUnit()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(lowerSizeLimitUnitKey, defaultLowerSizeLimitUnit);
    mutex.unlock();
    return value;
}
void Preferences::setLowerSizeLimitUnit(int value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lowerSizeLimitUnitKey, value);
    setCachedValue(lowerSizeLimitUnitKey, value);
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
    setCachedValue(folderPermissionsKey, permissions);
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
    setCachedValue(filePermissionsKey, permissions);
    settings->sync();
    mutex.unlock();
}

int Preferences::proxyType()
{
    mutex.lock();
    int value = getValue<int>(proxyTypeKey, defaultProxyType);
    mutex.unlock();
    return value;
}

void Preferences::setProxyType(int value)
{
    mutex.lock();
    settings->setValue(proxyTypeKey, value);
    setCachedValue(proxyTypeKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::proxyProtocol()
{
    mutex.lock();
    int value = getValue<int>(proxyProtocolKey, defaultProxyProtocol);
    mutex.unlock();
    return value;
}

void Preferences::setProxyProtocol(int value)
{
    mutex.lock();
    settings->setValue(proxyProtocolKey, value);
    setCachedValue(proxyProtocolKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::proxyServer()
{
    mutex.lock();
    QString value = getValue<QString>(proxyServerKey, defaultProxyServer);
    mutex.unlock();
    return value;
}

void Preferences::setProxyServer(const QString &value)
{
    mutex.lock();
    settings->setValue(proxyServerKey, value);
    setCachedValue(proxyServerKey, value);
    settings->sync();
    mutex.unlock();
}

int Preferences::proxyPort()
{
    mutex.lock();
    int value = getValue<int>(proxyPortKey, defaultProxyPort);
    mutex.unlock();
    return value;
}

void Preferences::setProxyPort(int value)
{
    mutex.lock();
    settings->setValue(proxyPortKey, value);
    setCachedValue(proxyPortKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::proxyRequiresAuth()
{
    mutex.lock();
    bool value = getValue<bool>(proxyRequiresAuthKey, defaultProxyRequiresAuth);
    mutex.unlock();
    return value;
}

void Preferences::setProxyRequiresAuth(bool value)
{
    mutex.lock();
    settings->setValue(proxyRequiresAuthKey, value);
    setCachedValue(proxyRequiresAuthKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::getProxyUsername()
{
    mutex.lock();
    QString value = getValue<QString>(proxyUsernameKey, defaultProxyUsername);
    mutex.unlock();
    return value;
}

void Preferences::setProxyUsername(const QString &value)
{
    mutex.lock();
    settings->setValue(proxyUsernameKey, value);
    setCachedValue(proxyUsernameKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::getProxyPassword()
{
    mutex.lock();
    QString value = getValue<QString>(proxyPasswordKey, defaultProxyPassword);
    mutex.unlock();
    return value;
}

void Preferences::setProxyPassword(const QString &value)
{
    mutex.lock();
    settings->setValue(proxyPasswordKey, value);
    setCachedValue(proxyPasswordKey, value);
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
    long long value = getValue<long long>(lastExecutionTimeKey, 0);
    mutex.unlock();
    return value;
}
long long Preferences::installationTime()
{
    mutex.lock();
    long long value = getValue<long long>(installationTimeKey, 0);
    mutex.unlock();
    return value;
}
void Preferences::setInstallationTime(long long time)
{
    mutex.lock();
    settings->setValue(installationTimeKey, time);
    setCachedValue(installationTimeKey, time);
    settings->sync();
    mutex.unlock();
}
long long Preferences::accountCreationTime()
{
    mutex.lock();
    long long value = getValue<long long>(accountCreationTimeKey, 0);
    mutex.unlock();
    return value;
}
void Preferences::setAccountCreationTime(long long time)
{
    mutex.lock();
    settings->setValue(accountCreationTimeKey, time);
    setCachedValue(accountCreationTimeKey, time);
    settings->sync();
    mutex.unlock();

}
long long Preferences::hasLoggedIn()
{
    mutex.lock();
    long long value = getValue<long long>(hasLoggedInKey, 0);
    mutex.unlock();
    return value;
}
void Preferences::setHasLoggedIn(long long time)
{
    mutex.lock();
    settings->setValue(hasLoggedInKey, time);
    setCachedValue(hasLoggedInKey, time);
    settings->sync();
    mutex.unlock();
}

bool Preferences::isFirstStartDone()
{
    mutex.lock();
    bool value = getValue<bool>(firstStartDoneKey, false);
    mutex.unlock();
    return value;
}

void Preferences::setFirstStartDone(bool value)
{
    mutex.lock();
    settings->setValue(firstStartDoneKey, value);
    setCachedValue(firstStartDoneKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::isFirstSyncDone()
{
    mutex.lock();
    bool value = getValue<bool>(firstSyncDoneKey, false);
    mutex.unlock();
    return value;
}

void Preferences::setFirstSyncDone(bool value)
{
    mutex.lock();
    settings->setValue(firstSyncDoneKey, value);
    setCachedValue(firstSyncDoneKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::isFirstFileSynced()
{
    mutex.lock();
    bool value = getValue<bool>(firstFileSyncedKey, false);
    mutex.unlock();
    return value;
}

void Preferences::setFirstFileSynced(bool value)
{
    mutex.lock();
    settings->setValue(firstFileSyncedKey, value);
    setCachedValue(firstFileSyncedKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::isFirstWebDownloadDone()
{
    mutex.lock();
    bool value = getValue<bool>(firstWebDownloadKey, false);
    mutex.unlock();
    return value;
}

void Preferences::setFirstWebDownloadDone(bool value)
{
    mutex.lock();
    settings->setValue(firstWebDownloadKey, value);
    setCachedValue(firstWebDownloadKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::isFatWarningShown()
{
    mutex.lock();
    bool value = getValue<bool>(fatWarningShownKey, false);
    mutex.unlock();
    return value;
}

void Preferences::setFatWarningShown(bool value)
{
    mutex.lock();
    settings->setValue(fatWarningShownKey, value);
    setCachedValue(fatWarningShownKey, value);
    settings->sync();
    mutex.unlock();
}

QString Preferences::lastCustomStreamingApp()
{
    mutex.lock();
    QString value = getValue<QString>(lastCustomStreamingAppKey);
    mutex.unlock();
    return value;
}

void Preferences::setLastCustomStreamingApp(const QString &value)
{
    mutex.lock();
    settings->setValue(lastCustomStreamingAppKey, value);
    setCachedValue(lastCustomStreamingAppKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::getMaxMemoryUsage()
{
    mutex.lock();
    long long value = getValue<long long>(maxMemoryUsageKey, 0);
    mutex.unlock();
    return value;
}

void Preferences::setMaxMemoryUsage(long long value)
{
    mutex.lock();
    settings->setValue(maxMemoryUsageKey, value);
    setCachedValue(maxMemoryUsageKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::getMaxMemoryReportTime()
{
    mutex.lock();
    long long value = getValue<long long>(maxMemoryReportTimeKey, 0);
    mutex.unlock();
    return value;
}

void Preferences::setMaxMemoryReportTime(long long timestamp)
{
    mutex.lock();
    settings->setValue(maxMemoryReportTimeKey, timestamp);
    setCachedValue(maxMemoryReportTimeKey, timestamp);
    settings->sync();
    mutex.unlock();
}

void Preferences::setLastExecutionTime(qint64 time)
{
    mutex.lock();
    settings->setValue(lastExecutionTimeKey, time);
    setCachedValue(lastExecutionTimeKey, time);
    settings->sync();
    mutex.unlock();
}

long long Preferences::lastUpdateTime()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(lastUpdateTimeKey, 0);
    mutex.unlock();
    return value;
}

void Preferences::setLastUpdateTime(long long time)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lastUpdateTimeKey, time);
    setCachedValue(lastUpdateTimeKey, time);
    settings->sync();
    mutex.unlock();
}

int Preferences::lastUpdateVersion()
{
    mutex.lock();
    assert(logged());
    int value = getValue<int>(lastUpdateVersionKey, 0);
    mutex.unlock();
    return value;
}

void Preferences::setLastUpdateVersion(int version)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lastUpdateVersionKey, version);
    setCachedValue(lastUpdateVersionKey, version);
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
    setCachedValue(downloadFolderKey, QDir::toNativeSeparators(value));
    settings->sync();
    mutex.unlock();
}

long long Preferences::uploadFolder()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(uploadFolderKey);
    mutex.unlock();
    return value;
}

void Preferences::setUploadFolder(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(uploadFolderKey, value);
    setCachedValue(uploadFolderKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::importFolder()
{
    mutex.lock();
    assert(logged());
    long long value = getValue<long long>(importFolderKey);
    mutex.unlock();
    return value;
}

void Preferences::setImportFolder(long long value)
{
    mutex.lock();
    assert(logged());
    settings->setValue(importFolderKey, value);
    setCachedValue(importFolderKey, value);
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

    bool value = getValue<bool>(oneTimeActionDoneKey + QString::number(action), false);

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
    setCachedValue(oneTimeActionDoneKey + QString::number(action), done);

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
        removeFromCache(excludedSyncNamesKey);
    }
    else
    {
        settings->setValue(excludedSyncNamesKey, excludedSyncNames.join(QString::fromAscii("\n")));
        setCachedValue(excludedSyncNamesKey, excludedSyncNames.join(QString::fromAscii("\n")));
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
        removeFromCache(excludedSyncPathsKey);
    }
    else
    {
        settings->setValue(excludedSyncPathsKey, excludedSyncPaths.join(QString::fromAscii("\n")));
        setCachedValue(excludedSyncPathsKey, excludedSyncPaths.join(QString::fromAscii("\n")));
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
        removeFromCache(previousCrashesKey);
    }
    else
    {
        settings->setValue(previousCrashesKey, crashes.join(QString::fromAscii("\n")));
        setCachedValue(previousCrashesKey, crashes.join(QString::fromAscii("\n")));
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

    long long value = getValue<long long>(lastRebootKey);

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
    setCachedValue(lastRebootKey, value);

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

    long long value = getValue<long long>(lastExitKey);

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
    setCachedValue(lastExitKey, value);

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

    QString value = getValue<QString>(httpsKeyKey, defaultHttpsKey);

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
    setCachedValue(httpsKeyKey, key);

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

    QString value = getValue<QString>(httpsCertKey, defaultHttpsCert);

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
    setCachedValue(httpsCertKey, cert);

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

    QString value = getValue<QString>(httpsCertIntermediateKey, defaultHttpsCertIntermediate);

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
    setCachedValue(httpsCertIntermediateKey, intermediate);

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

    long long value = getValue<long long>(httpsCertExpirationKey, defaultHttpsCertExpiration);

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
    setCachedValue(httpsCertExpirationKey, expiration);

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }

    settings->sync();
}

void Preferences::getLastHandleInfo(MegaHandle &lastHandle, int &type, long long &timestamp)
{
    mutex.lock();
    assert(logged());
    timestamp = getValue<long long>(lastPublicHandleTimestampKey, 0);
    lastHandle = getValue<unsigned long long>(lastPublicHandleKey, mega::INVALID_HANDLE);
    type = getValue<int>(lastPublicHandleTypeKey, MegaApi::AFFILIATE_TYPE_INVALID);
    mutex.unlock();
}

void Preferences::setLastPublicHandle(MegaHandle handle, int type)
{
    mutex.lock();
    assert(logged());
    settings->setValue(lastPublicHandleKey, (unsigned long long) handle);
    setCachedValue(lastPublicHandleKey, (unsigned long long) handle);
    settings->setValue(lastPublicHandleTimestampKey, QDateTime::currentMSecsSinceEpoch());
    setCachedValue(lastPublicHandleTimestampKey, QDateTime::currentMSecsSinceEpoch());
    settings->setValue(lastPublicHandleTypeKey, type);
    setCachedValue(lastPublicHandleTypeKey, type);
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
    settings->remove(sessionKey); // Remove session from specific account settings
    settings->endGroup();
    mutex.unlock();

    resetGlobalSettings();
}

void Preferences::resetGlobalSettings()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        settings->endGroup();
        currentAccount = settings->value(currentAccountKey).toString();
    }

    settings->remove(currentAccountKey);
    settings->remove(needsFetchNodesKey);
    settings->remove(currentAccountStatusKey);
    settings->remove(sessionKey); // Remove session from global settings
    clearTemporalBandwidth();
    syncNames.clear();
    syncIDs.clear();
    localFolders.clear();
    megaFolders.clear();
    megaFolderHandles.clear();
    activeFolders.clear();
    temporaryInactiveFolders.clear();
    localFingerprints.clear();

    if (!currentAccount.isEmpty())
    {
        settings->beginGroup(currentAccount);
    }
    settings->sync();
    cleanCache();
    mutex.unlock();

    emit stateChanged();
}

bool Preferences::isCrashed()
{
    mutex.lock();
    bool value = getValue<bool>(isCrashedKey, false);
    mutex.unlock();
    return value;
}

void Preferences::setCrashed(bool value)
{
    mutex.lock();
    settings->setValue(isCrashedKey, value);
    setCachedValue(isCrashedKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::getGlobalPaused()
{
    mutex.lock();
    bool value = getValue<bool>(wasPausedKey, false);
    mutex.unlock();
    return value;
}

void Preferences::setGlobalPaused(bool value)
{
    mutex.lock();
    settings->setValue(wasPausedKey, value);
    setCachedValue(wasPausedKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::getUploadsPaused()
{
    mutex.lock();
    bool value = getValue<bool>(wasUploadsPausedKey, false);
    mutex.unlock();
    return value;
}

void Preferences::setUploadsPaused(bool value)
{
    mutex.lock();
    settings->setValue(wasUploadsPausedKey, value);
    setCachedValue(wasUploadsPausedKey, value);
    settings->sync();
    mutex.unlock();
}

bool Preferences::getDownloadsPaused()
{
    mutex.lock();
    bool value = getValue<bool>(wasDownloadsPausedKey, false);
    mutex.unlock();
    return value;
}

void Preferences::setDownloadsPaused(bool value)
{
    mutex.lock();
    settings->setValue(wasDownloadsPausedKey, value);
    setCachedValue(wasDownloadsPausedKey, value);
    settings->sync();
    mutex.unlock();
}

long long Preferences::lastStatsRequest()
{
    mutex.lock();
    long long value = getValue<long long>(lastStatsRequestKey, 0);
    mutex.unlock();
    return value;
}

void Preferences::setLastStatsRequest(long long value)
{
    mutex.lock();
    settings->setValue(lastStatsRequestKey, value);
    setCachedValue(lastStatsRequestKey, value);
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
    setCachedValue(disableFileVersioningKey, value);
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
    setCachedValue(disableOverlayIconsKey, value);
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
    setCachedValue(disableLeftPaneIconsKey, value);
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
    mutex.lock();
    settings->sync();
    mutex.unlock();
}

void Preferences::deferSyncs(bool b)
{
    mutex.lock();
    settings->deferSyncs(b);
    mutex.unlock();
}

bool Preferences::needsDeferredSync()
{
    mutex.lock();
    bool b = settings->needsDeferredSync();
    mutex.unlock();
    return b;
}

void Preferences::setEmailAndGeneralSettings(const QString &email)
{
    int proxyType = this->proxyType();
    QString proxyServer = this->proxyServer();
    int proxyPort = this->proxyPort();
    int proxyProtocol = this->proxyProtocol();
    bool proxyAuth = this->proxyRequiresAuth();
    QString proxyUsername = this->getProxyUsername();
    QString proxyPassword = this->getProxyPassword();

    QString session = this->getSessionInGeneral();

    this->setEmail(email);

    this->setSessionInUserGroup(session); //this is required to provide backwards compatibility
    this->setProxyType(proxyType);
    this->setProxyServer(proxyServer);
    this->setProxyPort(proxyPort);
    this->setProxyProtocol(proxyProtocol);
    this->setProxyRequiresAuth(proxyAuth);
    this->setProxyUsername(proxyUsername);
    this->setProxyPassword(proxyPassword);
}

void Preferences::login(QString account)
{
    mutex.lock();
    logout();
    settings->setValue(currentAccountKey, account);
    setCachedValue(currentAccountKey, account);
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
        setCachedValue(lastVersionKey, Preferences::VERSION_CODE);
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
    cleanCache();
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
        setCachedValue(syncNameKey, syncNames[i]);
        settings->setValue(syncIdKey, syncIDs[i]);
        setCachedValue(syncIdKey, syncIDs[i]);
        settings->setValue(localFolderKey, localFolders[i]);
        setCachedValue(localFolderKey, localFolders[i]);
        settings->setValue(megaFolderKey, megaFolders[i]);
        setCachedValue(megaFolderKey, megaFolders[i]);
        settings->setValue(megaFolderHandleKey, megaFolderHandles[i]);
        setCachedValue(megaFolderHandleKey, megaFolderHandles[i]);
        settings->setValue(folderActiveKey, activeFolders[i]);
        setCachedValue(folderActiveKey, activeFolders[i]);
        settings->setValue(temporaryInactiveKey,temporaryInactiveFolders[i]);
        setCachedValue(temporaryInactiveKey, temporaryInactiveFolders[i]);
        settings->setValue(localFingerprintKey, localFingerprints[i]);
        setCachedValue(localFingerprintKey, localFingerprints[i]);

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
    overridePreference(settings, QString::fromUtf8("ALMOST_OS_INTERVAL_MS"), Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("OS_INTERVAL_MS"), Preferences::OQ_UI_MESSAGE_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("PAYWALL_NOTIFICATION_INTERVAL_MS"), Preferences::PAYWALL_NOTIFICATION_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("USER_INACTIVITY_MS"), Preferences::USER_INACTIVITY_MS);
    overridePreference(settings, QString::fromUtf8("STATE_REFRESH_INTERVAL_MS"), Preferences::STATE_REFRESH_INTERVAL_MS);

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
