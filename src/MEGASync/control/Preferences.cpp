#include "Preferences.h"
#include "platform/Platform.h"
#include "UserAttributesRequests/FullName.h"

#include <QDesktopServices>
#include <assert.h>

using namespace mega;

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

const char Preferences::CLIENT_KEY[] = "FhMgXbqb";
const char Preferences::USER_AGENT[] = "MEGAsync/5.0.11.0";
const int Preferences::VERSION_CODE = 5011;
const int Preferences::BUILD_ID = 0;
// Do not change the location of VERSION_STRING, create_tarball.sh parses this file
const QString Preferences::VERSION_STRING = QString::fromAscii("5.0.11");
QString Preferences::SDK_ID = QString::fromAscii("5625ce4");
const QString Preferences::CHANGELOG = QString::fromUtf8(QT_TR_NOOP(
"- Sync Rework ALPHA test version.\n"
"- All the very latest code changes.\n"));

const QString Preferences::TRANSLATION_FOLDER = QString::fromAscii("://translations/");
const QString Preferences::TRANSLATION_PREFIX = QString::fromAscii("MEGASyncStrings_");

int Preferences::STATE_REFRESH_INTERVAL_MS        = 10000;
int Preferences::NETWORK_REFRESH_INTERVAL_MS      = 30000;
int Preferences::FINISHED_TRANSFER_REFRESH_INTERVAL_MS        = 10000;

long long Preferences::OQ_DIALOG_INTERVAL_MS = 604800000; // 7 daysm
long long Preferences::OQ_NOTIFICATION_INTERVAL_MS = 129600000; // 36 hours
long long Preferences::ALMOST_OQ_UI_MESSAGE_INTERVAL_MS = 259200000; // 72 hours
long long Preferences::OQ_UI_MESSAGE_INTERVAL_MS = 129600000; // 36 hours
long long Preferences::PAYWALL_NOTIFICATION_INTERVAL_MS = 86400000; //24 hours
long long Preferences::USER_INACTIVITY_MS = 20000; // 20 secs

std::chrono::milliseconds Preferences::OVER_QUOTA_DIALOG_DISABLE_DURATION{std::chrono::hours(7*24)};
std::chrono::milliseconds Preferences::OVER_QUOTA_OS_NOTIFICATION_DISABLE_DURATION{std::chrono::hours(36)};
std::chrono::milliseconds Preferences::OVER_QUOTA_UI_ALERT_DISABLE_DURATION{std::chrono::hours(36)};
std::chrono::milliseconds Preferences::ALMOST_OVER_QUOTA_UI_ALERT_DISABLE_DURATION{std::chrono::hours(72)};
std::chrono::milliseconds Preferences::ALMOST_OVER_QUOTA_OS_NOTIFICATION_DISABLE_DURATION{std::chrono::hours(36)};
std::chrono::milliseconds Preferences::OVER_QUOTA_ACTION_DIALOGS_DISABLE_TIME{std::chrono::hours{12}};

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

unsigned int Preferences::MUTEX_STEALER_MS                    = 0;
unsigned int Preferences::MUTEX_STEALER_PERIOD_MS             = 0;
unsigned int Preferences::MUTEX_STEALER_PERIOD_ONLY_ONCE      = 0;

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
QString Preferences::BASE_URL = QString::fromAscii("https://mega.nz");
const QStringList Preferences::HTTPS_ALLOWED_ORIGINS = QStringList() << Preferences::BASE_URL
                                                                     << QLatin1String("https://mega.co.nz")
                                                                     << QLatin1String("chrome-extension://*")
                                                                     << QLatin1String("moz-extension://*")
                                                                     << QLatin1String("edge-extension://*");

bool Preferences::HTTPS_ORIGIN_CHECK_ENABLED = true;

#ifdef WIN32
    #ifdef _WIN64
        const QString Preferences::UPDATE_CHECK_URL             = QString::fromUtf8("http://g.static.mega.co.nz/upd/wsync64/v.txt");
    #else
        const QString Preferences::UPDATE_CHECK_URL             = QString::fromUtf8("http://g.static.mega.co.nz/upd/wsync/v.txt");
    #endif
#else
    #if defined(__arm64__)
        const QString Preferences::UPDATE_CHECK_URL                 = QString::fromUtf8("http://g.static.mega.co.nz/upd/msyncarm64/v.txt");
    #else
        const QString Preferences::UPDATE_CHECK_URL                 = QString::fromUtf8("http://g.static.mega.co.nz/upd/msyncv2/v.txt"); //Using msyncv2 to serve new updates and avoid keeping loader leftovers
    #endif
#endif

const char Preferences::UPDATE_PUBLIC_KEY[] = "EACTzXPE8fdMhm6LizLe1FxV2DncybVh2cXpW3momTb8tpzRNT833r1RfySz5uHe8gdoXN1W0eM5Bk8X-LefygYYDS9RyXrRZ8qXrr9ITJ4r8ATnFIEThO5vqaCpGWTVi5pOPI5FUTJuhghVKTyAels2SpYT5CmfSQIkMKv7YVldaV7A-kY060GfrNg4--ETyIzhvaSZ_jyw-gmzYl_dwfT9kSzrrWy1vQG8JPNjKVPC4MCTZJx9SNvp1fVi77hhgT-Mc5PLcDIfjustlJkDBHtmGEjyaDnaWQf49rGq94q23mLc56MSjKpjOR1TtpsCY31d1Oy2fEXFgghM0R-1UkKswVuWhEEd8nO2PimJOl4u9ZJ2PWtJL1Ro0Hlw9OemJ12klIAxtGV-61Z60XoErbqThwWT5Uu3D2gjK9e6rL9dufSoqjC7UA2C0h7KNtfUcUHw0UWzahlR8XBNFXaLWx9Z8fRtA_a4seZcr0AhIA7JdQG5i8tOZo966KcFnkU77pfQTSprnJhCfEmYbWm9EZA122LJBWq2UrSQQN3pKc9goNaaNxy5PYU1yXyiAfMVsBDmDonhRWQh2XhdV-FWJ3rOGMe25zOwV4z1XkNBuW4T1JF2FgqGR6_q74B2ccFC8vrNGvlTEcs3MSxTI_EKLXQvBYy7hxG8EPUkrMVCaWzzTQAFEQ";
const QString Preferences::CRASH_REPORT_URL                 = QString::fromUtf8("http://g.api.mega.co.nz/hb?crashdump");
const QString Preferences::UPDATE_FOLDER_NAME               = QString::fromAscii("update");
const QString Preferences::UPDATE_BACKUP_FOLDER_NAME        = QString::fromAscii("backup");
const QString Preferences::PROXY_TEST_URL                   = QString::fromUtf8("https://g.api.mega.co.nz/cs");
const QString Preferences::PROXY_TEST_SUBSTRING             = QString::fromUtf8("-2");
const QString Preferences::syncsGroupKey            = QString::fromAscii("Syncs");
const QString Preferences::syncsGroupByTagKey       = QString::fromAscii("SyncsByTag");
const QString Preferences::currentAccountKey        = QString::fromAscii("currentAccount");
const QString Preferences::currentAccountStatusKey  = QString::fromAscii("currentAccountStatus");
const QString Preferences::needsFetchNodesKey       = QString::fromAscii("needsFetchNodes");
const QString Preferences::emailKey                 = QString::fromAscii("email");
const QString Preferences::firstNameKey             = QString::fromAscii("firstName");
const QString Preferences::lastNameKey              = QString::fromAscii("lastName");
const QString Preferences::totalStorageKey          = QString::fromAscii("totalStorage");
const QString Preferences::usedStorageKey           = QString::fromAscii("usedStorage");
const QString Preferences::cloudDriveStorageKey     = QString::fromAscii("cloudDriveStorage");
const QString Preferences::vaultStorageKey          = QString::fromAscii("vaultStorage");
const QString Preferences::rubbishStorageKey        = QString::fromAscii("rubbishStorage");
const QString Preferences::inShareStorageKey        = QString::fromAscii("inShareStorage");
const QString Preferences::versionsStorageKey        = QString::fromAscii("versionsStorage");
const QString Preferences::cloudDriveFilesKey       = QString::fromAscii("cloudDriveFiles");
const QString Preferences::vaultFilesKey            = QString::fromAscii("vaultFiles");
const QString Preferences::rubbishFilesKey          = QString::fromAscii("rubbishFiles");
const QString Preferences::inShareFilesKey          = QString::fromAscii("inShareFiles");
const QString Preferences::cloudDriveFoldersKey     = QString::fromAscii("cloudDriveFolders");
const QString Preferences::vaultFoldersKey          = QString::fromAscii("vaultFolders");
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

const QString Preferences::transferOverQuotaDialogLastExecutionKey = QString::fromAscii("transferOverQuotaDialogLastExecution");
const QString Preferences::transferOverQuotaOsNotificationLastExecutionKey = QString::fromAscii("transferOverQuotaOsNotificationLastExecution");
const QString Preferences::transferAlmostOverQuotaOsNotificationLastExecutionKey = QString::fromAscii("transferAlmostOverQuotaOsNotificationLastExecution");
const QString Preferences::transferAlmostOverQuotaUiAlertLastExecutionKey = QString::fromAscii("transferAlmostOverQuotaUiAlertLastExecution");
const QString Preferences::transferOverQuotaUiAlertLastExecutionKey = QString::fromAscii("transferOverQuotaUiAlertDisableUntil");

const QString Preferences::transferOverQuotaSyncDialogLastExecutionKey = QString::fromAscii("transferOverQuotaSyncDialogLastExecution");
const QString Preferences::transferOverQuotaDownloadsDialogLastExecutionKey = QString::fromAscii("transferOverQuotaDownloadsDialogLastExecution");
const QString Preferences::transferOverQuotaImportLinksDialogLastExecutionKey = QString::fromAscii("transferOverQuotaImportLinksDialogLastExecution");
const QString Preferences::transferOverQuotaStreamDialogLastExecutionKey = QString::fromAscii("transferOverQuotaStreamDialogLastExecution");
const QString Preferences::storageOverQuotaUploadsDialogLastExecutionKey = QString::fromAscii("storageOverQuotaUploadsDialogLastExecution");
const QString Preferences::storageOverQuotaSyncsDialogLastExecutionKey = QString::fromAscii("storageOverQuotaSyncsDialogLastExecution");

const bool Preferences::defaultShowNotifications = true;

const bool Preferences::defaultDeprecatedNotifications      = true;
const QString Preferences::showDeprecatedNotificationsKey   = QString::fromAscii("showNotifications");

const QString Preferences::accountTypeKey           = QString::fromAscii("accountType");
const QString Preferences::proExpirityTimeKey       = QString::fromAscii("proExpirityTime");
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
const QString Preferences::configuredSyncsKey       = QString::fromAscii("configuredSyncs");
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
const QString Preferences::localFingerprintKey      = QString::fromAscii("localFingerprint");
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
const QString Preferences::firstBackupDoneKey       = QString::fromAscii("firstBackupDone");
const QString Preferences::firstFileSyncedKey       = QString::fromAscii("firstFileSynced");
const QString Preferences::firstFileBackedUpKey     = QString::fromAscii("firstFileBackedUp");
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
const QString Preferences::disabledSyncsKey = QString::fromAscii("disabledSyncs");
const QString Preferences::neverCreateLinkKey       = QString::fromUtf8("neverCreateLink");
const QString Preferences::notifyDisabledSyncsKey = QString::fromAscii("notifyDisabledSyncs");
const QString Preferences::importMegaLinksEnabledKey = QString::fromAscii("importMegaLinksEnabled");
const QString Preferences::downloadMegaLinksEnabledKey = QString::fromAscii("downloadMegaLinksEnabled");

//Sleep settings
const QString Preferences::awakeIfActiveKey = QString::fromAscii("sleepIfInactiveEnabledKey");
const bool Preferences::defaultAwakeIfActive = false;

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
const long long  Preferences::defaultUpperSizeLimitValue              = 1; //Input UI range 1-9999. Use 1 as default value
const long long  Preferences::defaultLowerSizeLimitValue              = 1; //Input UI range 1-9999. Use 1 as default value
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

const bool  Preferences::defaultNeverCreateLink   = false;

const bool  Preferences::defaultImportMegaLinksEnabled = true;
const bool  Preferences::defaultDownloadMegaLinksEnabled = true;

std::shared_ptr<Preferences> Preferences::instance()
{
    static std::shared_ptr<Preferences> preferences (new Preferences());
    return preferences;
}

void Preferences::initialize(QString dataPath)
{
    mDataPath = dataPath;
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
    mSettings.reset(new EncryptedSettings(settingsFile));

    QString currentAccount = mSettings->value(currentAccountKey).toString();
    if (currentAccount.size())
    {
        if (hasEmail(currentAccount))
        {
            login(currentAccount);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Settings does not contain current account group. Will try to use backup settings")
                         .toUtf8().constData());
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

        if (QFile::rename(bakSettingsFile, settingsFile))
        {
            mSettings.reset(new EncryptedSettings(settingsFile));

            //Retry with backup file
            currentAccount = mSettings->value(currentAccountKey).toString();
            if (currentAccount.size())
            {
                if (hasEmail(currentAccount))
                {
                    login(currentAccount);
                    errorFlag = false;
                }
                else
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Settings does not contain current account group in backup setting either.")
                                 .toUtf8().constData());
                    errorFlag = true;
                }
            }
        }
    }

    if (errorFlag)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Cleaning settings after error encountered.").toUtf8().constData());
        clearAll();
    }
    else
    {
        recoverDeprecatedNotificationsSettings();
    }
}

Preferences::Preferences() :
    QObject(),
    mutex(QMutex::Recursive),
    mSettings(nullptr),
    diffTimeWithSDK(0),
    lastTransferNotification(0)
{
    clearTemporalBandwidth();
}

QString Preferences::email()
{
    assert(logged());
    return getValueConcurrent<QString>(emailKey);
}

void Preferences::setEmail(QString email)
{
    mutex.lock();
    login(email);
    mSettings->setValue(emailKey, email);
    setCachedValue(emailKey, email);
    mSettings->sync();
    mutex.unlock();
    emit stateChanged();
}

QString Preferences::firstName()
{
    assert(logged());
    return getValueConcurrent<QString>(firstNameKey, QString());
}

void Preferences::setFirstName(QString firstName)
{
    setValueAndSyncConcurrent(firstNameKey, firstName);
}

QString Preferences::lastName()
{
    assert(logged());
    return getValueConcurrent<QString>(lastNameKey, QString());
}

void Preferences::setLastName(QString lastName)
{
    setValueAndSyncConcurrent(lastNameKey, lastName);
}

void Preferences::setSession(QString session)
{
    mutex.lock();
    storeSessionInGeneral(session);
    mSettings->sync();
    mutex.unlock();
}

void Preferences::setSessionInUserGroup(QString session)
{
    assert(logged());
    setValueAndSyncConcurrent(sessionKey, session);
}

void Preferences::storeSessionInGeneral(QString session)
{
    mutex.lock();

    QString currentAccount;
    if (logged())
    {
        mSettings->setValue(sessionKey, session); //store in user group too (for backwards compatibility)
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->setValue(sessionKey, session);
    setCachedValue(sessionKey, session);
    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }
    mSettings->sync();
    mutex.unlock();
}

QString Preferences::getSessionInGeneral()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    QString value = getValue<QString>(sessionKey);
    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
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
        value = mSettings->value(sessionKey).toString(); // for MEGAsync prior unfinished fetchnodes resumable sessions (<=4.3.1)
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
    auto value = getValue<unsigned long long>(transferIdentifierKey, defaultTransferIdentifier);
    value++;
    mSettings->setValue(transferIdentifierKey, value);
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
    assert(logged());
    return getValueConcurrent<long long>(totalStorageKey);
}

void Preferences::setTotalStorage(long long value)
{
    assert(logged());
    setValueConcurrent(totalStorageKey, value);
}

long long Preferences::usedStorage()
{
    assert(logged());
    return getValueConcurrent<long long>(usedStorageKey);
}

void Preferences::setUsedStorage(long long value)
{
    assert(logged());
    value = std::max(value, static_cast<long long>(0));
    setValueConcurrent(usedStorageKey, value);
}

long long Preferences::availableStorage()
{
    mutex.lock();
    assert(logged());
    long long total = getValue<long long>(totalStorageKey);
    long long used = getValue<long long>(usedStorageKey);
    mutex.unlock();
    long long available = total - used;
    return available >= 0 ? available : 0;
}

long long Preferences::cloudDriveStorage()
{
    assert(logged());
    return getValueConcurrent<long long>(cloudDriveStorageKey);
}

void Preferences::setCloudDriveStorage(long long value)
{
    assert(logged());
    setValueConcurrent(cloudDriveStorageKey, value);
}

long long Preferences::vaultStorage()
{
    assert(logged());
    return getValueConcurrent<long long>(vaultStorageKey);
}

void Preferences::setVaultStorage(long long value)
{
    assert(logged());
    setValueConcurrent(vaultStorageKey, value);
}

long long Preferences::rubbishStorage()
{
    assert(logged());
    return getValueConcurrent<long long>(rubbishStorageKey);
}

void Preferences::setRubbishStorage(long long value)
{
    assert(logged());
    setValueConcurrent(rubbishStorageKey, value);
}

long long Preferences::inShareStorage()
{
    assert(logged());
    return getValueConcurrent<long long>(inShareStorageKey);
}

void Preferences::setInShareStorage(long long value)
{
    assert(logged());
    setValueConcurrent(inShareStorageKey, value);
}

long long Preferences::versionsStorage()
{
    assert(logged());
    return getValueConcurrent<long long>(versionsStorageKey);
}

void Preferences::setVersionsStorage(long long value)
{
    assert(logged());
    setValueConcurrent(versionsStorageKey, value);
}

long long Preferences::cloudDriveFiles()
{
    assert(logged());
    return getValueConcurrent<long long>(cloudDriveFilesKey);
}

void Preferences::setCloudDriveFiles(long long value)
{
    assert(logged());
    setValueConcurrent(cloudDriveFilesKey, value);
}

long long Preferences::vaultFiles()
{
    assert(logged());
    return getValueConcurrent<long long>(vaultFilesKey);
}

void Preferences::setVaultFiles(long long value)
{
    assert(logged());
    setValueConcurrent(vaultFilesKey, value);
}

long long Preferences::rubbishFiles()
{
    assert(logged());
    return getValueConcurrent<long long>(rubbishFilesKey);
}

void Preferences::setRubbishFiles(long long value)
{
    assert(logged());
    setValueConcurrent(rubbishFilesKey, value);
}

long long Preferences::inShareFiles()
{
    assert(logged());
    return getValueConcurrent<long long>(inShareFilesKey);
}

void Preferences::setInShareFiles(long long value)
{
    assert(logged());
    setValueConcurrent(inShareFilesKey, value);
}

long long Preferences::cloudDriveFolders()
{
    assert(logged());
    return getValueConcurrent<long long>(cloudDriveFoldersKey);
}

void Preferences::setCloudDriveFolders(long long value)
{
    assert(logged());
    setValueConcurrent(cloudDriveFoldersKey, value);
}

long long Preferences::vaultFolders()
{
    assert(logged());
    return getValueConcurrent<long long>(vaultFoldersKey);
}

void Preferences::setVaultFolders(long long value)
{
    assert(logged());
    setValueConcurrent(vaultFoldersKey, value);
}

long long Preferences::rubbishFolders()
{
    assert(logged());
    return getValueConcurrent<long long>(rubbishFoldersKey);
}

void Preferences::setRubbishFolders(long long value)
{
    assert(logged());
    setValueConcurrent(rubbishFoldersKey, value);
}

long long Preferences::inShareFolders()
{
    assert(logged());
    return getValueConcurrent<long long>(inShareFoldersKey);
}

void Preferences::setInShareFolders(long long value)
{
    assert(logged());
    setValueConcurrent(inShareFoldersKey, value);
}

long long Preferences::totalBandwidth()
{
    assert(logged());
    return getValueConcurrent<long long>(totalBandwidthKey);
}

void Preferences::setTotalBandwidth(long long value)
{
    assert(logged());
    setValueConcurrent(totalBandwidthKey, value);
}

int Preferences::bandwidthInterval()
{
    assert(logged());
    return getValueConcurrent<int>(usedBandwidthIntervalKey);
}

void Preferences::setBandwidthInterval(int value)
{
    assert(logged());
    setValueConcurrent(usedBandwidthIntervalKey, value);
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
    assert(logged());
    return getValueConcurrent<long long>(overStorageDialogExecutionKey, defaultTimeStamp);
}

void Preferences::setOverStorageDialogExecution(long long timestamp)
{
    assert(logged());
    setValueConcurrent(overStorageDialogExecutionKey, timestamp);
}

long long Preferences::getOverStorageNotificationExecution()
{
    assert(logged());
    return getValueConcurrent<long long>(overStorageNotificationExecutionKey, defaultTimeStamp);
}

void Preferences::setOverStorageNotificationExecution(long long timestamp)
{
    assert(logged());
    setValueConcurrent(overStorageNotificationExecutionKey, timestamp);
}

long long Preferences::getAlmostOverStorageNotificationExecution()
{
    assert(logged());
    return getValueConcurrent<long long>(almostOverStorageNotificationExecutionKey, defaultTimeStamp);
}

void Preferences::setAlmostOverStorageNotificationExecution(long long timestamp)
{
    assert(logged());
    setValueConcurrent(almostOverStorageNotificationExecutionKey, timestamp);
}

long long Preferences::getPayWallNotificationExecution()
{
    assert(logged());
    return getValueConcurrent<long long>(payWallNotificationExecutionKey, defaultTimeStamp);
}

void Preferences::setPayWallNotificationExecution(long long timestamp)
{
    assert(logged());
    setValueConcurrent(payWallNotificationExecutionKey, timestamp);
}

long long Preferences::getAlmostOverStorageDismissExecution()
{
    assert(logged());
    return getValueConcurrent<long long>(almostOverStorageDismissExecutionKey, defaultTimeStamp);
}

void Preferences::setAlmostOverStorageDismissExecution(long long timestamp)
{
    assert(logged());
    setValueConcurrent(almostOverStorageDismissExecutionKey, timestamp);
}

long long Preferences::getOverStorageDismissExecution()
{
    assert(logged());
    return getValueConcurrent<long long>(overStorageDismissExecutionKey, defaultTimeStamp);
}

void Preferences::setOverStorageDismissExecution(long long timestamp)
{
    assert(logged());
    setValueConcurrent(overStorageDismissExecutionKey, timestamp);
}

std::chrono::system_clock::time_point Preferences::getTimePoint(const QString& key)
{
    QMutexLocker locker(&mutex);
    assert(logged());
    const long long value{getValue<long long>(key, defaultTimeStamp)};
    std::chrono::milliseconds durationMillis(value);
    return std::chrono::system_clock::time_point{durationMillis};
}

void Preferences::setTimePoint(const QString& key, const std::chrono::system_clock::time_point& timepoint)
{
    QMutexLocker locker(&mutex);
    assert(logged());
    auto timePointMillis = std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count();
    mSettings->setValue(key, static_cast<long long>(timePointMillis));
    setCachedValue(key, static_cast<long long>(timePointMillis));
}

template<typename T>
T Preferences::getValue(const QString &key)
{
    auto cf = cache.find(key);
    if (cf != cache.end())
    {
        assert(cf->second.value<T>() == mSettings->value(key).value<T>());
        return cf->second.value<T>();
    }
    else return mSettings->value(key).value<T>();
}

template<typename T>
T Preferences::getValue(const QString &key, const T &defaultValue)
{
    auto cf = cache.find(key);
    if (cf != cache.end())
    {
        assert(cf->second.value<T>() == mSettings->value(key, defaultValue).template value<T>());
        return cf->second.value<T>();
    }
    else return mSettings->value(key, defaultValue).template value<T>();
}

template<typename T>
T Preferences::getValueConcurrent(const QString &key)
{
    QMutexLocker locker(&mutex);
    return getValue<T>(key);
}

template<typename T>
T Preferences::getValueConcurrent(const QString &key, const T &defaultValue)
{
    QMutexLocker locker(&mutex);
    return getValue<T>(key, defaultValue);
}

void Preferences::setAndCachedValue(const QString &key, const QVariant &value)
{
    mSettings->setValue(key, value);
    setCachedValue(key, value);
}

void Preferences::setValueAndSyncConcurrent(const QString &key, const QVariant &value)
{
    QMutexLocker locker(&mutex);
    setAndCachedValue(key, value);
    mSettings->sync();
}

void Preferences::setValueConcurrent(const QString &key, const QVariant &value)
{
    QMutexLocker locker(&mutex);
    setAndCachedValue(key, value);
}

void Preferences::setCachedValue(const QString &key, const QVariant &value)
{
    if (!key.isEmpty())
    {
        cache[key] = value;
    }
}

void Preferences::cleanCache()
{
    cache.clear();
}

void Preferences::removeFromCache(const QString &key)
{
    cache.erase(key);
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaDialogLastExecution()
{
    return getTimePoint(transferOverQuotaDialogLastExecutionKey);
}

void Preferences::setTransferOverQuotaDialogLastExecution(std::chrono::system_clock::time_point timepoint)
{
    setTimePoint(transferOverQuotaDialogLastExecutionKey, timepoint);
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaOsNotificationLastExecution()
{
    return getTimePoint(transferOverQuotaOsNotificationLastExecutionKey);
}

void Preferences::setTransferOverQuotaOsNotificationLastExecution(std::chrono::system_clock::time_point timepoint)
{
    setTimePoint(transferOverQuotaOsNotificationLastExecutionKey, timepoint);
}

std::chrono::system_clock::time_point Preferences::getTransferAlmostOverQuotaOsNotificationLastExecution()
{
    return getTimePoint(transferAlmostOverQuotaOsNotificationLastExecutionKey);
}

void Preferences::setTransferAlmostOverQuotaOsNotificationLastExecution(std::chrono::system_clock::time_point timepoint)
{
    setTimePoint(transferAlmostOverQuotaOsNotificationLastExecutionKey, timepoint);
}

std::chrono::system_clock::time_point Preferences::getTransferAlmostOverQuotaUiAlertLastExecution()
{
    return getTimePoint(transferAlmostOverQuotaUiAlertLastExecutionKey);
}

void Preferences::setTransferAlmostOverQuotaUiAlertLastExecution(std::chrono::system_clock::time_point timepoint)
{
    setTimePoint(transferAlmostOverQuotaUiAlertLastExecutionKey, timepoint);
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaUiAlertLastExecution()
{
    return getTimePoint(transferOverQuotaUiAlertLastExecutionKey);
}

void Preferences::setTransferOverQuotaUiAlertLastExecution(std::chrono::system_clock::time_point timepoint)
{
    setTimePoint(transferOverQuotaUiAlertLastExecutionKey, timepoint);
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaSyncDialogLastExecution()
{
    return getTimePoint(transferOverQuotaSyncDialogLastExecutionKey);
}

void Preferences::setTransferOverQuotaSyncDialogLastExecution(std::chrono::system_clock::time_point timepoint)
{
    setTimePoint(transferOverQuotaSyncDialogLastExecutionKey, timepoint);
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaDownloadsDialogLastExecution()
{
    return getTimePoint(transferOverQuotaDownloadsDialogLastExecutionKey);
}

void Preferences::setTransferOverQuotaDownloadsDialogLastExecution(std::chrono::system_clock::time_point timepoint)
{
    setTimePoint(transferOverQuotaDownloadsDialogLastExecutionKey, timepoint);
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaImportLinksDialogLastExecution()
{
    return getTimePoint(transferOverQuotaImportLinksDialogLastExecutionKey);
}

void Preferences::setTransferOverQuotaImportLinksDialogLastExecution(std::chrono::system_clock::time_point timepoint)
{
    setTimePoint(transferOverQuotaImportLinksDialogLastExecutionKey, timepoint);
}

std::chrono::system_clock::time_point Preferences::getTransferOverQuotaStreamDialogLastExecution()
{
    return getTimePoint(transferOverQuotaStreamDialogLastExecutionKey);
}

void Preferences::setTransferOverQuotaStreamDialogLastExecution(std::chrono::system_clock::time_point timepoint)
{
    setTimePoint(transferOverQuotaStreamDialogLastExecutionKey, timepoint);
}

std::chrono::system_clock::time_point Preferences::getStorageOverQuotaUploadsDialogLastExecution()
{
    return getTimePoint(storageOverQuotaUploadsDialogLastExecutionKey);
}

void Preferences::setStorageOverQuotaUploadsDialogLastExecution(std::chrono::system_clock::time_point timepoint)
{
    setTimePoint(storageOverQuotaUploadsDialogLastExecutionKey, timepoint);
}

std::chrono::system_clock::time_point Preferences::getStorageOverQuotaSyncsDialogLastExecution()
{
    return getTimePoint(storageOverQuotaSyncsDialogLastExecutionKey);
}

void Preferences::setStorageOverQuotaSyncsDialogLastExecution(std::chrono::system_clock::time_point timepoint)
{

    setTimePoint(storageOverQuotaSyncsDialogLastExecutionKey, timepoint);
}

int Preferences::getStorageState()
{
    assert(logged());
    return getValueConcurrent<int>(storageStateQKey, MegaApi::STORAGE_STATE_UNKNOWN);
}

void Preferences::setStorageState(int value)
{
    assert(logged());
    setValueConcurrent(storageStateQKey, value);
}

int Preferences::getBusinessState()
{
    assert(logged());
    return getValueConcurrent<int>(businessStateQKey, -2);
}

void Preferences::setBusinessState(int value)
{
    assert(logged());
    setValueConcurrent(businessStateQKey, value);
}

int Preferences::getBlockedState()
{
    assert(logged());
    return getValueConcurrent<int>(blockedStateQKey, -2);
}

void Preferences::setBlockedState(int value)
{
    assert(logged());
    setValueConcurrent(blockedStateQKey, value);
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
    assert(logged());
    return getValueConcurrent<long long>(usedBandwidthKey);
}

void Preferences::setUsedBandwidth(long long value)
{
    assert(logged());
    setValueConcurrent(usedBandwidthKey, value);
}

int Preferences::accountType()
{
    assert(logged());
    return getValueConcurrent<int>(accountTypeKey);
}

void Preferences::setAccountType(int value)
{
    assert(logged());
    setValueConcurrent(accountTypeKey, value);
}

long long Preferences::proExpirityTime()
{
    assert(logged());
    return getValueConcurrent<long long>(proExpirityTimeKey);
}

void Preferences::setProExpirityTime(long long value)
{
    assert(logged());
    setValueConcurrent(proExpirityTimeKey, value);
}
/************ NOTIFICATIONS GETTERS/SETTERS ************/

bool Preferences::isNotificationEnabled(NotificationsTypes type, bool includingGeneralSwitch)
{
    bool value(false);

    if(!includingGeneralSwitch || isGeneralSwitchNotificationsOn())
    {
        auto key = notificationsTypeToString(type);

        if(!key.isEmpty())
        {
            value = getValueConcurrent<bool>(key, defaultShowNotifications);
        }
    }

    return value;
}

bool Preferences::isAnyNotificationEnabled(bool includingGeneralSwitch)
{
    bool result(false);

    if(!includingGeneralSwitch || isGeneralSwitchNotificationsOn())
    {
        for(int index = notificationsTypeUT(NotificationsTypes::GENERAL_SWITCH_NOTIFICATIONS) + 1;
            index < notificationsTypeUT(NotificationsTypes::LAST); ++index)
        {
           if(isNotificationEnabled((NotificationsTypes)index,includingGeneralSwitch))
           {
               result = true;
               break;
           }
        }
    }

    return result;
}

bool Preferences::isGeneralSwitchNotificationsOn()
{
    bool generalSwitchNotificationsValue(false);

    auto generalSwitchNotificationsKey = notificationsTypeToString(NotificationsTypes::GENERAL_SWITCH_NOTIFICATIONS);

    if(!generalSwitchNotificationsKey.isEmpty())
    {
        generalSwitchNotificationsValue = getValueConcurrent<bool>(generalSwitchNotificationsKey, defaultShowNotifications);
    }

    return generalSwitchNotificationsValue;
}

void Preferences::enableNotifications(NotificationsTypes type, bool value)
{
    assert(logged());

    auto key = notificationsTypeToString(type);

    if(!key.isEmpty())
    {
        setValueAndSyncConcurrent(key, value);
    }
}

void Preferences::recoverDeprecatedNotificationsSettings()
{
    QVariant deprecatedGlobalNotifications = getValueConcurrent<QVariant>(showDeprecatedNotificationsKey);
    if(!deprecatedGlobalNotifications.isNull())
    {
        assert(logged());
        for(int index = notificationsTypeUT(NotificationsTypes::GENERAL_SWITCH_NOTIFICATIONS) + 1;
            index < notificationsTypeUT(NotificationsTypes::LAST); ++index)
        {
            auto key = notificationsTypeToString((NotificationsTypes)index);

            if(!key.isEmpty())
            {
               setValueAndSyncConcurrent(key,deprecatedGlobalNotifications);
            }
        }

        QMutexLocker locker(&mutex);
        mSettings->remove(showDeprecatedNotificationsKey);
        removeFromCache(showDeprecatedNotificationsKey);
        mSettings->sync();
    }
}

QString Preferences::notificationsTypeToString(NotificationsTypes type)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<NotificationsTypes>();
    return QString::fromUtf8(metaEnum.valueToKey(notificationsTypeUT(type)));
}

/************ END OF NOTIFICATIONS GETTERS/SETTERS ************/

bool Preferences::startOnStartup()
{
    return getValueConcurrent<bool>(startOnStartupKey, defaultStartOnStartup);
}

void Preferences::setStartOnStartup(bool value)
{
    setValueAndSyncConcurrent(startOnStartupKey, value);
}

bool Preferences::usingHttpsOnly()
{
    return getValueConcurrent<bool>(useHttpsOnlyKey, defaultUseHttpsOnly);
}

void Preferences::setUseHttpsOnly(bool value)
{
    setValueAndSyncConcurrent(useHttpsOnlyKey, value);
}

bool Preferences::SSLcertificateException()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }
    bool value = getValue<bool>(SSLcertificateExceptionKey, defaultSSLcertificateException);
    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
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
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }
    mSettings->setValue(SSLcertificateExceptionKey, value);
    setCachedValue(SSLcertificateExceptionKey, value);
    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }
    mSettings->sync();
    mutex.unlock();
}

QString Preferences::language()
{
    return getValueConcurrent<QString>(languageKey, QLocale::system().name());
}

void Preferences::setLanguage(QString &value)
{
    setValueAndSyncConcurrent(languageKey, value);
}

bool Preferences::updateAutomatically()
{
    return getValueConcurrent<bool>(updateAutomaticallyKey, defaultUpdateAutomatically);
}

void Preferences::setUpdateAutomatically(bool value)
{
    setValueAndSyncConcurrent(updateAutomaticallyKey, value);
}

bool Preferences::hasDefaultUploadFolder()
{
    return getValueConcurrent<bool>(hasDefaultUploadFolderKey, uploadFolder() != 0);
}

bool Preferences::hasDefaultDownloadFolder()
{
    return getValueConcurrent<bool>(hasDefaultDownloadFolderKey, !downloadFolder().isEmpty());
}

bool Preferences::hasDefaultImportFolder()
{
    return getValueConcurrent<bool>(hasDefaultImportFolderKey, importFolder() != 0);
}

void Preferences::setHasDefaultUploadFolder(bool value)
{
    setValueAndSyncConcurrent(hasDefaultUploadFolderKey, value);
}

void Preferences::setHasDefaultDownloadFolder(bool value)
{
    setValueAndSyncConcurrent(hasDefaultDownloadFolderKey, value);
}

void Preferences::setHasDefaultImportFolder(bool value)
{
    setValueAndSyncConcurrent(hasDefaultImportFolderKey, value);
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
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    int value = getValue<int>(currentAccountStatusKey, defaultAccountStatus);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
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
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->setValue(currentAccountStatusKey, value);
    setCachedValue(currentAccountStatusKey, value);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }
    mSettings->sync();
    mutex.unlock();
}


bool Preferences::needsFetchNodesInGeneral()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    bool value = getValue<bool>(needsFetchNodesKey, defaultNeedsFetchNodes);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
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
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->setValue(needsFetchNodesKey, value);
    setCachedValue(needsFetchNodesKey, value);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }
    mSettings->sync();
    mutex.unlock();
}


int Preferences::uploadLimitKB()
{
    assert(logged());
    return getValueConcurrent<int>(uploadLimitKBKey, defaultUploadLimitKB);
}

void Preferences::setUploadLimitKB(int value)
{
    assert(logged());
    setValueAndSyncConcurrent(uploadLimitKBKey, value);
}

int Preferences::downloadLimitKB()
{
    assert(logged());
    return getValueConcurrent<int>(downloadLimitKBKey, defaultDownloadLimitKB);
}

int Preferences::parallelUploadConnections()
{
    return getValueConcurrent<int>(parallelUploadConnectionsKey, defaultParallelUploadConnections);
}

int Preferences::parallelDownloadConnections()
{
    return getValueConcurrent<int>(parallelDownloadConnectionsKey, defaultParallelDownloadConnections);
}

void Preferences::setParallelUploadConnections(int value)
{
    assert(logged());
    if (value < 1 || value > 6)
    {
       value = 3;
    }
    setValueAndSyncConcurrent(parallelUploadConnectionsKey, value);
}

void Preferences::setParallelDownloadConnections(int value)
{
    assert(logged());
    if (value < 1 || value > 6)
    {
       value = 4;
    }
    setValueAndSyncConcurrent(parallelDownloadConnectionsKey, value);
}

void Preferences::setDownloadLimitKB(int value)
{
    assert(logged());
    setValueAndSyncConcurrent(downloadLimitKBKey, value);
}

bool Preferences::upperSizeLimit()
{
    return getValueConcurrent<bool>(upperSizeLimitKey, defaultUpperSizeLimit);
}

void Preferences::setUpperSizeLimit(bool value)
{
    setValueAndSyncConcurrent(upperSizeLimitKey, value);
}

long long Preferences::upperSizeLimitValue()
{
    assert(logged());
    return getValueConcurrent<long long>(upperSizeLimitValueKey, defaultUpperSizeLimitValue);
}

void Preferences::setUpperSizeLimitValue(long long value)
{
    assert(logged());
    setValueAndSyncConcurrent(upperSizeLimitValueKey, value);
}

bool Preferences::cleanerDaysLimit()
{
    return getValueConcurrent<bool>(cleanerDaysLimitKey, defaultCleanerDaysLimit);
}

void Preferences::setCleanerDaysLimit(bool value)
{
    setValueAndSyncConcurrent(cleanerDaysLimitKey, value);
}

int Preferences::cleanerDaysLimitValue()
{
    assert(logged());
    return getValueConcurrent<int>(cleanerDaysLimitValueKey, defaultCleanerDaysLimitValue);
}

void Preferences::setCleanerDaysLimitValue(int value)
{
    assert(logged());
    setValueAndSyncConcurrent(cleanerDaysLimitValueKey, value);
}

int Preferences::upperSizeLimitUnit()
{
    assert(logged());
    return getValueConcurrent<int>(upperSizeLimitUnitKey, defaultUpperSizeLimitUnit);
}
void Preferences::setUpperSizeLimitUnit(int value)
{
    assert(logged());
    setValueAndSyncConcurrent(upperSizeLimitUnitKey, value);
}

bool Preferences::lowerSizeLimit()
{
    return getValueConcurrent<bool>(lowerSizeLimitKey, defaultLowerSizeLimit);
}

void Preferences::setLowerSizeLimit(bool value)
{
    setValueAndSyncConcurrent(lowerSizeLimitKey, value);
}

long long Preferences::lowerSizeLimitValue()
{
    assert(logged());
    return getValueConcurrent<long long>(lowerSizeLimitValueKey, defaultLowerSizeLimitValue);
}

void Preferences::setLowerSizeLimitValue(long long value)
{
    assert(logged());
    setValueAndSyncConcurrent(lowerSizeLimitValueKey, value);
}

int Preferences::lowerSizeLimitUnit()
{
    assert(logged());
    return getValueConcurrent<int>(lowerSizeLimitUnitKey, defaultLowerSizeLimitUnit);
}

void Preferences::setLowerSizeLimitUnit(int value)
{
    assert(logged());
    setValueAndSyncConcurrent(lowerSizeLimitUnitKey, value);
}

int Preferences::folderPermissionsValue()
{
    mutex.lock();
    int permissions = getValue<int>(folderPermissionsKey, defaultFolderPermissions);
    mutex.unlock();
    return permissions;
}

void Preferences::setFolderPermissionsValue(int permissions)
{
    setValueAndSyncConcurrent(folderPermissionsKey, permissions);
}

int Preferences::filePermissionsValue()
{
    mutex.lock();
    int permissions = getValue<int>(filePermissionsKey, defaultFilePermissions);
    mutex.unlock();
    return permissions;
}

void Preferences::setFilePermissionsValue(int permissions)
{
    setValueAndSyncConcurrent(filePermissionsKey, permissions);
}

int Preferences::proxyType()
{
    return getValueConcurrent<int>(proxyTypeKey, defaultProxyType);
}

void Preferences::setProxyType(int value)
{
    setValueAndSyncConcurrent(proxyTypeKey, value);
}

int Preferences::proxyProtocol()
{
    return getValueConcurrent<int>(proxyProtocolKey, defaultProxyProtocol);
}

void Preferences::setProxyProtocol(int value)
{
    setValueAndSyncConcurrent(proxyProtocolKey, value);
}

QString Preferences::proxyServer()
{
    return getValueConcurrent<QString>(proxyServerKey, defaultProxyServer);
}

void Preferences::setProxyServer(const QString &value)
{
    setValueAndSyncConcurrent(proxyServerKey, value);
}

int Preferences::proxyPort()
{
    return getValueConcurrent<int>(proxyPortKey, defaultProxyPort);
}

void Preferences::setProxyPort(int value)
{
    setValueAndSyncConcurrent(proxyPortKey, value);
}

bool Preferences::proxyRequiresAuth()
{
    return getValueConcurrent<bool>(proxyRequiresAuthKey, defaultProxyRequiresAuth);
}

void Preferences::setProxyRequiresAuth(bool value)
{
    setValueAndSyncConcurrent(proxyRequiresAuthKey, value);
}

QString Preferences::getProxyUsername()
{
    return getValueConcurrent<QString>(proxyUsernameKey, defaultProxyUsername);
}

void Preferences::setProxyUsername(const QString &value)
{
    setValueAndSyncConcurrent(proxyUsernameKey, value);
}

QString Preferences::getProxyPassword()
{
    return getValueConcurrent<QString>(proxyPasswordKey, defaultProxyPassword);
}

void Preferences::setProxyPassword(const QString &value)
{
    setValueAndSyncConcurrent(proxyPasswordKey, value);
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
    return getValueConcurrent<long long>(installationTimeKey, 0);
}

void Preferences::setInstallationTime(long long time)
{
    setValueAndSyncConcurrent(installationTimeKey, time);
}

long long Preferences::accountCreationTime()
{
    return getValueConcurrent<long long>(accountCreationTimeKey, 0);
}

void Preferences::setAccountCreationTime(long long time)
{
    setValueAndSyncConcurrent(accountCreationTimeKey, time);

}

long long Preferences::hasLoggedIn()
{
    return getValueConcurrent<long long>(hasLoggedInKey, 0);
}

void Preferences::setHasLoggedIn(long long time)
{
    setValueAndSyncConcurrent(hasLoggedInKey, time);
}

bool Preferences::isFirstStartDone()
{
    return getValueConcurrent<bool>(firstStartDoneKey, false);
}

void Preferences::setFirstStartDone(bool value)
{
    setValueAndSyncConcurrent(firstStartDoneKey, value);
}

bool Preferences::isFirstSyncDone()
{
    return getValueConcurrent<bool>(firstSyncDoneKey, false);
}

void Preferences::setFirstSyncDone(bool value)
{
    setValueAndSyncConcurrent(firstSyncDoneKey, value);
}

bool Preferences::isFirstBackupDone()
{
    return getValueConcurrent<bool>(firstBackupDoneKey, false);
}

void Preferences::setFirstBackupDone(bool value)
{
    setValueAndSyncConcurrent(firstBackupDoneKey, value);
}

bool Preferences::isFirstFileSynced()
{
    return getValueConcurrent<bool>(firstFileSyncedKey, false);
}

void Preferences::setFirstFileSynced(bool value)
{
    setValueAndSyncConcurrent(firstFileSyncedKey, value);
}

bool Preferences::isFirstFileBackedUp()
{
    return getValueConcurrent<bool>(firstFileBackedUpKey, false);
}

void Preferences::setFirstFileBackedUp(bool value)
{
    setValueAndSyncConcurrent(firstFileBackedUpKey, value);
}

bool Preferences::isFirstWebDownloadDone()
{
    return getValueConcurrent<bool>(firstWebDownloadKey, false);
}

void Preferences::setFirstWebDownloadDone(bool value)
{
    setValueAndSyncConcurrent(firstWebDownloadKey, value);
}

bool Preferences::isFatWarningShown()
{
    return getValueConcurrent<bool>(fatWarningShownKey, false);
}

void Preferences::setFatWarningShown(bool value)
{
    setValueAndSyncConcurrent(fatWarningShownKey, value);
}

QString Preferences::lastCustomStreamingApp()
{
    return getValueConcurrent<QString>(lastCustomStreamingAppKey);
}

void Preferences::setLastCustomStreamingApp(const QString &value)
{
    setValueAndSyncConcurrent(lastCustomStreamingAppKey, value);
}

long long Preferences::getMaxMemoryUsage()
{
    return getValueConcurrent<long long>(maxMemoryUsageKey, 0);
}

void Preferences::setMaxMemoryUsage(long long value)
{
    setValueAndSyncConcurrent(maxMemoryUsageKey, value);
}

long long Preferences::getMaxMemoryReportTime()
{
    return getValueConcurrent<long long>(maxMemoryReportTimeKey, 0);
}

void Preferences::setMaxMemoryReportTime(long long timestamp)
{
    setValueAndSyncConcurrent(maxMemoryReportTimeKey, timestamp);
}

void Preferences::setLastExecutionTime(qint64 time)
{
    setValueAndSyncConcurrent(lastExecutionTimeKey, time);
}

long long Preferences::lastUpdateTime()
{
    return getValueConcurrent<long long>(lastUpdateTimeKey, 0);
}

void Preferences::setLastUpdateTime(long long time)
{
    assert(logged());
    setValueAndSyncConcurrent(lastUpdateTimeKey, time);
}

int Preferences::lastUpdateVersion()
{
    assert(logged());
    return getValueConcurrent<int>(lastUpdateVersionKey, 0);
}

void Preferences::setLastUpdateVersion(int version)
{
    assert(logged());
    setValueAndSyncConcurrent(lastUpdateVersionKey, version);
}

QString Preferences::downloadFolder()
{
    mutex.lock();
    QString value = QDir::toNativeSeparators(getValue<QString>(downloadFolderKey));
    mutex.unlock();
    return value;
}

void Preferences::setDownloadFolder(QString value)
{
    setValueAndSyncConcurrent(downloadFolderKey, QDir::toNativeSeparators(value));
}

long long Preferences::uploadFolder()
{
    assert(logged());
    return getValueConcurrent<long long>(uploadFolderKey);
}

void Preferences::setUploadFolder(long long value)
{
    assert(logged());
    setValueAndSyncConcurrent(uploadFolderKey, value);
}

long long Preferences::importFolder()
{
    assert(logged());
    return getValueConcurrent<long long>(importFolderKey);
}

void Preferences::setImportFolder(long long value)
{
    assert(logged());
    setValueAndSyncConcurrent(importFolderKey, value);
}

bool Preferences::getImportMegaLinksEnabled()
{
    assert(logged());
    return getValueConcurrent<bool>(importMegaLinksEnabledKey, defaultImportMegaLinksEnabled);
}

void Preferences::setImportMegaLinksEnabled(const bool value)
{
    assert(logged());
    setValueAndSyncConcurrent(importMegaLinksEnabledKey, value);
}

bool Preferences::getDownloadMegaLinksEnabled()
{
    assert(logged());
    return getValueConcurrent<bool>(downloadMegaLinksEnabledKey, defaultDownloadMegaLinksEnabled);
}

void Preferences::setDownloadMegaLinksEnabled(const bool value)
{
    assert(logged());
    setValueAndSyncConcurrent(downloadMegaLinksEnabledKey, value);
}

bool Preferences::neverCreateLink()
{
    return getValueConcurrent<bool>(neverCreateLinkKey, defaultNeverCreateLink);
}

void Preferences::setNeverCreateLink(bool value)
{
    setValueAndSyncConcurrent(neverCreateLinkKey, value);
}


/////////   Sync related stuff /////////////////////

void Preferences::removeAllFolders()
{
    QMutexLocker qm(&mutex);
    assert(logged());

    //remove all configured syncs
    mSettings->beginGroup(syncsGroupByTagKey);
    mSettings->remove(QLatin1String("")); //remove group and all its settings
    mSettings->endGroup();
    mSettings->sync();
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
        mSettings->remove(excludedSyncNamesKey);
        removeFromCache(excludedSyncNamesKey);
    }
    else
    {
        mSettings->setValue(excludedSyncNamesKey, excludedSyncNames.join(QLatin1String("\n")));
        setCachedValue(excludedSyncNamesKey, excludedSyncNames.join(QString::fromAscii("\n")));
    }

    mSettings->sync();
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
        mSettings->remove(excludedSyncPathsKey);
        removeFromCache(excludedSyncPathsKey);
    }
    else
    {
        mSettings->setValue(excludedSyncPathsKey, excludedSyncPaths.join(QString::fromAscii("\n")));
        setCachedValue(excludedSyncPathsKey, excludedSyncPaths.join(QString::fromAscii("\n")));
    }

    mSettings->sync();
    mutex.unlock();
}


bool Preferences::isOneTimeActionDone(int action)
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    bool value = getValue<bool>(oneTimeActionDoneKey + QString::number(action), false);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
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
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->setValue(oneTimeActionDoneKey + QString::number(action), done);
    setCachedValue(oneTimeActionDoneKey + QString::number(action), done);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }
    mSettings->sync();
    mutex.unlock();
}


QStringList Preferences::getPreviousCrashes()
{
    mutex.lock();
    QStringList previousCrashes;
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }
    previousCrashes = mSettings->value(previousCrashesKey).toString().split(QString::fromAscii("\n", QString::SkipEmptyParts));
    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
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
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    if (!crashes.size())
    {
        mSettings->remove(previousCrashesKey);
        removeFromCache(previousCrashesKey);
    }
    else
    {
        mSettings->setValue(previousCrashesKey, crashes.join(QString::fromAscii("\n")));
        setCachedValue(previousCrashesKey, crashes.join(QString::fromAscii("\n")));
    }

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mSettings->sync();
    mutex.unlock();
}

long long Preferences::getLastReboot()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    long long value = getValue<long long>(lastRebootKey);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
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
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->setValue(lastRebootKey, value);
    setCachedValue(lastRebootKey, value);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mSettings->sync();
    mutex.unlock();
}

long long Preferences::getLastExit()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    long long value = getValue<long long>(lastExitKey);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
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
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->setValue(lastExitKey, value);
    setCachedValue(lastExitKey, value);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mSettings->sync();
    mutex.unlock();
}

QSet<MegaHandle> Preferences::getDisabledSyncTags()
{
    QMutexLocker qm(&mutex);
    assert(logged());

    QStringList stringTagList = getValueConcurrent<QString>(disabledSyncsKey).split(QString::fromUtf8("0x1E"), QString::SkipEmptyParts);
    if (!stringTagList.isEmpty())
    {
        QList<mega::MegaHandle> tagList;
        for (auto &tag : stringTagList)
        {
            tagList.append(tag.toULongLong());
        }

        return QSet<mega::MegaHandle>::fromList(tagList);
    }

    return QSet<mega::MegaHandle>();
}

void Preferences::setDisabledSyncTags(QSet<mega::MegaHandle> disabledSyncs)
{
    QMutexLocker qm(&mutex);
    assert(logged());

    QList<mega::MegaHandle> disabledTags = disabledSyncs.toList();
    QStringList tags;

    for(auto &tag : disabledTags)
    {
        tags.append(QString::number(tag));
    }

    setValueAndSyncConcurrent(disabledSyncsKey, tags.join(QString::fromUtf8("0x1E")));
}

bool Preferences::getNotifyDisabledSyncsOnLogin()
{
    return getValueConcurrent<bool>(notifyDisabledSyncsKey, false);
}

void Preferences::setNotifyDisabledSyncsOnLogin(bool notify)
{
    setValueAndSyncConcurrent(notifyDisabledSyncsKey, notify);
}

QString Preferences::getHttpsKey()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    QString value = getValue<QString>(httpsKeyKey, defaultHttpsKey);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return value;
}

void Preferences::setHttpsKey(QString key)
{
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->setValue(httpsKeyKey, key);
    setCachedValue(httpsKeyKey, key);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mSettings->sync();
}

QString Preferences::getHttpsCert()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    QString value = getValue<QString>(httpsCertKey, defaultHttpsCert);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return value;
}

void Preferences::setHttpsCert(QString cert)
{
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->setValue(httpsCertKey, cert);
    setCachedValue(httpsCertKey, cert);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mSettings->sync();
}

QString Preferences::getHttpsCertIntermediate()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    QString value = getValue<QString>(httpsCertIntermediateKey, defaultHttpsCertIntermediate);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return value;
}

void Preferences::setHttpsCertIntermediate(QString intermediate)
{
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->setValue(httpsCertIntermediateKey, intermediate);
    setCachedValue(httpsCertIntermediateKey, intermediate);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mSettings->sync();
}

long long Preferences::getHttpsCertExpiration()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    long long value = getValue<long long>(httpsCertExpirationKey, defaultHttpsCertExpiration);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mutex.unlock();
    return value;
}

void Preferences::setHttpsCertExpiration(long long expiration)
{
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->setValue(httpsCertExpirationKey, expiration);
    setCachedValue(httpsCertExpirationKey, expiration);

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }

    mSettings->sync();
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
    mSettings->setValue(lastPublicHandleKey, (unsigned long long) handle);
    setCachedValue(lastPublicHandleKey, (unsigned long long) handle);
    mSettings->setValue(lastPublicHandleTimestampKey, QDateTime::currentMSecsSinceEpoch());
    setCachedValue(lastPublicHandleTimestampKey, QDateTime::currentMSecsSinceEpoch());
    mSettings->setValue(lastPublicHandleTypeKey, type);
    setCachedValue(lastPublicHandleTypeKey, type);
    mSettings->sync();
    mutex.unlock();
}

int Preferences::getNumUsers()
{
    mutex.lock();
    assert(!logged());
    int value = mSettings->numChildGroups();
    mutex.unlock();
    return value;
}


bool Preferences::enterUser(QString account)
{
    QMutexLocker locker(&mutex);
    assert(!logged());
    if (account.size() && mSettings->containsGroup(account))
    {
        mSettings->beginGroup(account);
        readFolders();
        return true;
    }
    return false;
}

void Preferences::enterUser(int i)
{
    mutex.lock();
    assert(!logged());
    assert(i < mSettings->numChildGroups());
    if (i < mSettings->numChildGroups())
    {
        mSettings->beginGroup(i);
    }

    readFolders();
    mutex.unlock();
}

void Preferences::leaveUser()
{
    mutex.lock();
    assert(logged());
    mSettings->endGroup();

    mutex.unlock();
}

void Preferences::unlink()
{
    mutex.lock();
    assert(logged());
    mSettings->remove(sessionKey); // Remove session from specific account settings
    mSettings->endGroup();
    mutex.unlock();

    resetGlobalSettings();
}

void Preferences::resetGlobalSettings()
{
    mutex.lock();
    QString currentAccount;
    if (logged())
    {
        mSettings->endGroup();
        currentAccount = mSettings->value(currentAccountKey).toString();
    }

    mSettings->remove(currentAccountKey);
    mSettings->remove(needsFetchNodesKey);
    mSettings->remove(currentAccountStatusKey);
    mSettings->remove(sessionKey); // Remove session from global settings
    clearTemporalBandwidth();

    if (!currentAccount.isEmpty())
    {
        mSettings->beginGroup(currentAccount);
    }
    mSettings->sync();
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
    setValueAndSyncConcurrent(isCrashedKey, value);
}

bool Preferences::getGlobalPaused()
{
    return getValueConcurrent<bool>(wasPausedKey, false);
}

void Preferences::setGlobalPaused(bool value)
{
    setValueAndSyncConcurrent(wasPausedKey, value);
}

bool Preferences::getUploadsPaused()
{
    return getValueConcurrent<bool>(wasUploadsPausedKey, false);
}

void Preferences::setUploadsPaused(bool value)
{
    setValueAndSyncConcurrent(wasUploadsPausedKey, value);
}

bool Preferences::getDownloadsPaused()
{
    return getValueConcurrent<bool>(wasDownloadsPausedKey, false);
}

void Preferences::setDownloadsPaused(bool value)
{
    setValueAndSyncConcurrent(wasDownloadsPausedKey, value);
}

long long Preferences::lastStatsRequest()
{
    return getValueConcurrent<long long>(lastStatsRequestKey, 0);
}

void Preferences::setLastStatsRequest(long long value)
{
    setValueAndSyncConcurrent(lastStatsRequestKey, value);
}

bool Preferences::awakeIfActiveEnabled()
{
    mutex.lock();
    assert(logged());
    bool result = getValue(awakeIfActiveKey, defaultAwakeIfActive);
    mutex.unlock();
    return result;
}

void Preferences::setAwakeIfActive(bool value)
{
    setValueAndSyncConcurrent(awakeIfActiveKey, value);
}

bool Preferences::fileVersioningDisabled()
{
    mutex.lock();
    assert(logged());
    bool result = getValue(disableFileVersioningKey, false);
    mutex.unlock();
    return result;
}

void Preferences::disableFileVersioning(bool value)
{
    setValueAndSyncConcurrent(disableFileVersioningKey, value);
}

bool Preferences::overlayIconsDisabled()
{
    mutex.lock();
    bool result = getValue(disableOverlayIconsKey, false);
    mutex.unlock();
    return result;
}

void Preferences::disableOverlayIcons(bool value)
{
    setValueAndSyncConcurrent(disableOverlayIconsKey, value);
}

bool Preferences::leftPaneIconsDisabled()
{
    mutex.lock();
    bool result = getValue(disableLeftPaneIconsKey, false);
    mutex.unlock();
    return result;
}

void Preferences::disableLeftPaneIcons(bool value)
{
    setValueAndSyncConcurrent(disableLeftPaneIconsKey, value);
}

bool Preferences::error()
{
    return errorFlag;
}

QString Preferences::getDataPath()
{
    mutex.lock();
    QString ret = mDataPath;
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

    mSettings->clear();
    mSettings->sync();
    mutex.unlock();
}

void Preferences::sync()
{
    mutex.lock();
    mSettings->sync();
    mutex.unlock();
}

void Preferences::deferSyncs(bool b)
{
    mutex.lock();
    mSettings->deferSyncs(b);
    mutex.unlock();
}

bool Preferences::needsDeferredSync()
{
    mutex.lock();
    bool b = mSettings->needsDeferredSync();
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

void Preferences::monitorUserAttributes()
{
    assert(logged());
    // Setup FIRST_NAME and LAST_NAME monitoring
    updateFullName();
}

void Preferences::login(QString account)
{
    mutex.lock();
    logout();
    mSettings->setValue(currentAccountKey, account);
    setCachedValue(currentAccountKey, account);
    mSettings->beginGroup(account);
    readFolders();
    loadExcludedSyncNames();
    int lastVersion = mSettings->value(lastVersionKey).toInt();
    if (lastVersion != Preferences::VERSION_CODE)
    {
        if ((lastVersion != 0) && (lastVersion < Preferences::VERSION_CODE))
        {
            emit updated(lastVersion);
        }
        mSettings->setValue(lastVersionKey, Preferences::VERSION_CODE);
        setCachedValue(lastVersionKey, Preferences::VERSION_CODE);
    }
    mSettings->sync();
    mutex.unlock();
}

bool Preferences::logged()
{
    mutex.lock();
    bool value = !mSettings->isGroupEmpty();
    mutex.unlock();
    return value;
}

bool Preferences::hasEmail(QString email)
{
    mutex.lock();
    assert(!logged());
    bool value = mSettings->containsGroup(email);
    if (value)
    {
        mSettings->beginGroup(email);
        QString storedEmail = mSettings->value(emailKey).toString();
        value = !storedEmail.compare(email);
        if (!value)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Email key differs from requested email: %1. Removing the old entry: %2")
                         .arg(email).arg(storedEmail).toUtf8().constData());
            mSettings->remove(QString::fromAscii(""));
        }
        mSettings->endGroup();
    }
    mutex.unlock();
    return value;
}

void Preferences::logout()
{
    mutex.lock();
    if (logged())
    {
        mSettings->endGroup();
    }
    clearTemporalBandwidth();
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
    excludedSyncNames = getValue<QString>(excludedSyncNamesKey).split(QString::fromAscii("\n", QString::SkipEmptyParts));
    if (excludedSyncNames.size()==1 && excludedSyncNames.at(0).isEmpty())
    {
        excludedSyncNames.clear();
    }

    excludedSyncPaths = getValue<QString>(excludedSyncPathsKey).split(QString::fromAscii("\n", QString::SkipEmptyParts));
    if (excludedSyncPaths.size()==1 && excludedSyncPaths.at(0).isEmpty())
    {
        excludedSyncPaths.clear();
    }

    if (getValue<int>(lastVersionKey) < 108)
    {
        excludedSyncNames.clear();
        excludedSyncNames.append(QString::fromUtf8("Thumbs.db"));
        excludedSyncNames.append(QString::fromUtf8("desktop.ini"));
        excludedSyncNames.append(QString::fromUtf8("~*"));
        excludedSyncNames.append(QString::fromUtf8(".*"));
    }

    if (getValue<int>(lastVersionKey) < 3400)
    {
        excludedSyncNames.append(QString::fromUtf8("*~.*"));
        // Avoid trigraph replacement by some pre-processors by splitting the string.("??-" --> "~").
        excludedSyncNames.append(QString::fromUtf8("*.sb-????????""-??????"));
        excludedSyncNames.append(QString::fromUtf8("*.tmp"));
    }

    if (getValue<int>(lastVersionKey) < 2907)
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


QMap<mega::MegaHandle, std::shared_ptr<SyncSettings> > Preferences::getLoadedSyncsMap() const
{
    return loadedSyncsMap;
}

void Preferences::readFolders()
{
    mutex.lock();
    assert(logged());

    loadedSyncsMap.clear();

    mSettings->beginGroup(syncsGroupByTagKey);
    int numSyncs = mSettings->numChildGroups();
    for (int i = 0; i < numSyncs; i++)
    {
        mSettings->beginGroup(i);

        auto sc = std::make_shared<SyncSettings>(mSettings->value(configuredSyncsKey).value<QString>());
        if (sc->backupId())
        {
            loadedSyncsMap[sc->backupId()] = sc;
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromAscii("Reading invalid Sync Setting!").toUtf8().constData());
        }

        mSettings->endGroup();
    }
    mSettings->endGroup();
    mutex.unlock();
}


SyncData::SyncData(QString name, QString localFolder, long long  megaHandle, QString megaFolder, long long localfp, bool enabled, bool tempDisabled, int pos, QString syncID)
    : mName(name), mLocalFolder(localFolder), mMegaHandle(megaHandle), mMegaFolder(megaFolder), mLocalfp(localfp),
      mEnabled(enabled), mTemporarilyDisabled(tempDisabled), mPos(pos), mSyncID(syncID)
{

}

void Preferences::removeOldCachedSync(int position, QString email)
{
    QMutexLocker qm(&mutex);
    assert(logged() || !email.isEmpty());

    // if not logged, use email to get into that user group and remove just some specific sync group
    if (!logged() && email.size() && mSettings->containsGroup(email))
    {
        mSettings->beginGroup(email);
        mSettings->beginGroup(syncsGroupKey);
        mSettings->beginGroup(QString::number(position));
        mSettings->remove(QString::fromAscii("")); //Remove all previous values
        mSettings->endGroup();//sync
        mSettings->endGroup();//old syncs
        mSettings->endGroup();//user
        return;
    }

    // otherwise remove oldSync and rewrite all
    auto it = oldSyncs.begin();
    while (it != oldSyncs.end())
    {
        if (it->mPos == position)
        {
            it = oldSyncs.erase(it);
        }
        else
        {
            ++it;
        }
    }
    saveOldCachedSyncs();
}

QList<SyncData> Preferences::readOldCachedSyncs(int *cachedBusinessState, int *cachedBlockedState, int *cachedStorageState, QString email)
{
    QMutexLocker qm(&mutex);
    oldSyncs.clear();

    // if not logged in & email provided, read old syncs from that user and load new-cache sync from prev session
    bool temporarilyLoggedPrefs = false;
    if (!instance()->logged() && !email.isEmpty())
    {
        loadedSyncsMap.clear(); //ensure loaded are empty even when there is no email
        temporarilyLoggedPrefs = instance()->enterUser(email);
        if (temporarilyLoggedPrefs)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Migrating syncs data to SDK cache from previous session")
                         .toUtf8().constData());
        }
        else
        {
            return oldSyncs;
        }
    }

    assert(logged());
    //restore cached status
    if (cachedBusinessState) *cachedBusinessState = getValue<int>(businessStateQKey, -2);
    if (cachedBlockedState) *cachedBlockedState = getValue<int>(blockedStateQKey, -2);
    if (cachedStorageState) *cachedStorageState = getValue<int>(storageStateQKey, MegaApi::STORAGE_STATE_UNKNOWN);

    mSettings->beginGroup(syncsGroupKey);
    int numSyncs = mSettings->numChildGroups();
    for (int i = 0; i < numSyncs; i++)
    {
        mSettings->beginGroup(i);

        bool enabled = mSettings->value(folderActiveKey, true).toBool();

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Reading old cache sync setting ... ").toUtf8().constData());

        if (temporarilyLoggedPrefs) //coming from old session
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromAscii(" ... sync configuration rescued from old session. Set as disabled.")
                         .toUtf8().constData());

            enabled = false; // syncs coming from old sessions are now considered unsafe to continue automatically
            // Note: in this particular case, we are not showing any error in the sync (since that information is not carried out
            // to the SDK)
        }

        oldSyncs.push_back(SyncData(mSettings->value(syncNameKey).toString(),
                                    mSettings->value(localFolderKey).toString(),
                                    mSettings->value(megaFolderHandleKey, static_cast<long long>(INVALID_HANDLE)).toLongLong(),
                                    mSettings->value(megaFolderKey).toString(),
                                    mSettings->value(localFingerprintKey, 0).toLongLong(),
                                    enabled,
                                    mSettings->value(temporaryInactiveKey, false).toBool(),
                                     i,
                                    mSettings->value(syncIdKey, true).toString()
                                    ));

        mSettings->endGroup();
    }
    mSettings->endGroup();

    if (temporarilyLoggedPrefs)
    {
        instance()->leaveUser();
    }

    return oldSyncs;
}

void Preferences::saveOldCachedSyncs()
{
    QMutexLocker qm(&mutex);
    assert(logged());

    if (!logged())
    {
        return;
    }

    mSettings->beginGroup(syncsGroupKey);

    mSettings->remove(QString::fromAscii("")); //Remove all previous values

    int i = 0 ;
    foreach(SyncData osd, oldSyncs) //normally if no errors happened it'll be empty
    {
        mSettings->beginGroup(QString::number(i));

        mSettings->setValue(syncNameKey, osd.mName);
        mSettings->setValue(localFolderKey, osd.mLocalFolder);
        mSettings->setValue(localFingerprintKey, osd.mLocalfp);
        mSettings->setValue(megaFolderHandleKey, osd.mMegaHandle);
        mSettings->setValue(megaFolderKey, osd.mMegaFolder);
        mSettings->setValue(folderActiveKey, osd.mEnabled);
        mSettings->setValue(syncIdKey, osd.mSyncID);

        mSettings->endGroup();
    }

    mSettings->endGroup();
    mSettings->sync();
}


void Preferences::removeAllSyncSettings()
{
    QMutexLocker qm(&mutex);
    assert(logged());

    mSettings->beginGroup(syncsGroupByTagKey);

    mSettings->remove(QString::fromAscii("")); //removes group and all its settings

    mSettings->endGroup();
    mSettings->sync();
}


void Preferences::removeSyncSetting(std::shared_ptr<SyncSettings> syncSettings)
{
    QMutexLocker qm(&mutex);
    assert(logged() && syncSettings);
    if (!syncSettings)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromAscii("Removing invalid Sync Setting!").toUtf8().constData());
        return;
    }

    mSettings->beginGroup(syncsGroupByTagKey);

    mSettings->beginGroup(QString::number(syncSettings->backupId()));

    mSettings->remove(QString::fromAscii("")); //removes group and all its settings

    mSettings->endGroup();

    mSettings->endGroup();
    mSettings->sync();
}

void Preferences::writeSyncSetting(std::shared_ptr<SyncSettings> syncSettings)
{
    if (logged())
    {
        QMutexLocker qm(&mutex);

        mSettings->beginGroup(syncsGroupByTagKey);

        mSettings->beginGroup(QString::number(syncSettings->backupId()));

        mSettings->setValue(configuredSyncsKey, syncSettings->toString());

        mSettings->endGroup();

        mSettings->endGroup();
        mSettings->sync();
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromAscii("Writting sync settings before logged in").toUtf8().constData());
    }
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

template<>
void Preferences::overridePreference(const QSettings &settings, QString &&name, std::chrono::milliseconds &value)
{
    const std::chrono::milliseconds previous{value};
    const long long previousMillis{static_cast<long long>(value.count())};
    const QVariant variant{settings.value(name, previousMillis)};
    value = std::chrono::milliseconds(variant.value<long long>());
    if (previous != value)
    {
        qDebug() << "Preference " << name << " overridden: " << value.count();
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
    overridePreference(settings, QString::fromUtf8("NETWORK_REFRESH_INTERVAL_MS"), Preferences::NETWORK_REFRESH_INTERVAL_MS);

    overridePreference(settings, QString::fromUtf8("TRANSFER_OVER_QUOTA_DIALOG_DISABLE_DURATION_MS"), Preferences::OVER_QUOTA_DIALOG_DISABLE_DURATION);
    overridePreference(settings, QString::fromUtf8("TRANSFER_OVER_QUOTA_OS_NOTIFICATION_DISABLE_DURATION_MS"), Preferences::OVER_QUOTA_OS_NOTIFICATION_DISABLE_DURATION);
    overridePreference(settings, QString::fromUtf8("TRANSFER_OVER_QUOTA_UI_ALERT_DISABLE_DURATION_MS"), Preferences::OVER_QUOTA_UI_ALERT_DISABLE_DURATION);
    overridePreference(settings, QString::fromUtf8("TRANSFER_ALMOST_OVER_QUOTA_UI_ALERT_DISABLE_DURATION_MS"), Preferences::ALMOST_OVER_QUOTA_UI_ALERT_DISABLE_DURATION);
    overridePreference(settings, QString::fromUtf8("TRANSFER_ALMOST_OVER_QUOTA_OS_NOTIFICATION_DISABLE_DURATION_MS"), Preferences::ALMOST_OVER_QUOTA_OS_NOTIFICATION_DISABLE_DURATION);
    overridePreference(settings, QString::fromUtf8("OVER_QUOTA_ACTION_DIALOGS_DISABLE_TIME_MS"), Preferences::OVER_QUOTA_ACTION_DIALOGS_DISABLE_TIME);

    overridePreference(settings, QString::fromUtf8("MIN_UPDATE_STATS_INTERVAL"), Preferences::MIN_UPDATE_STATS_INTERVAL);
    overridePreference(settings, QString::fromUtf8("MIN_UPDATE_CLEANING_INTERVAL_MS"), Preferences::MIN_UPDATE_CLEANING_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("MIN_UPDATE_NOTIFICATION_INTERVAL_MS"), Preferences::MIN_UPDATE_NOTIFICATION_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("MIN_REBOOT_INTERVAL_MS"), Preferences::MIN_REBOOT_INTERVAL_MS);
    overridePreference(settings, QString::fromUtf8("MIN_EXTERNAL_NODES_WARNING_MS"), Preferences::MIN_EXTERNAL_NODES_WARNING_MS);
    overridePreference(settings, QString::fromUtf8("MIN_TRANSFER_NOTIFICATION_INTERVAL_MS"), Preferences::MIN_TRANSFER_NOTIFICATION_INTERVAL_MS);

    overridePreference(settings, QString::fromUtf8("UPDATE_INITIAL_DELAY_SECS"), Preferences::UPDATE_INITIAL_DELAY_SECS);
    overridePreference(settings, QString::fromUtf8("UPDATE_RETRY_INTERVAL_SECS"), Preferences::UPDATE_RETRY_INTERVAL_SECS);
    overridePreference(settings, QString::fromUtf8("UPDATE_TIMEOUT_SECS"), Preferences::UPDATE_TIMEOUT_SECS);
    overridePreference(settings, QString::fromUtf8("MAX_LOGIN_TIME_MS"), Preferences::MAX_LOGIN_TIME_MS);
    overridePreference(settings, QString::fromUtf8("PROXY_TEST_TIMEOUT_MS"), Preferences::PROXY_TEST_TIMEOUT_MS);
    overridePreference(settings, QString::fromUtf8("MAX_IDLE_TIME_MS"), Preferences::MAX_IDLE_TIME_MS);
    overridePreference(settings, QString::fromUtf8("MAX_COMPLETED_ITEMS"), Preferences::MAX_COMPLETED_ITEMS);

    overridePreference(settings, QString::fromUtf8("MUTEX_STEALER_MS"), Preferences::MUTEX_STEALER_MS);
    overridePreference(settings, QString::fromUtf8("MUTEX_STEALER_PERIOD_MS"), Preferences::MUTEX_STEALER_PERIOD_MS);
    overridePreference(settings, QString::fromUtf8("MUTEX_STEALER_PERIOD_ONLY_ONCE"), Preferences::MUTEX_STEALER_PERIOD_ONLY_ONCE);
}

void Preferences::updateFullName()
{
    auto fullNameRequest (UserAttributes::UserAttributesManager::instance()
                      .requestAttribute<UserAttributes::FullName>(email().toUtf8().constData()));
    connect(fullNameRequest.get(), &UserAttributes::FullName::separateNamesReady,
            this, &Preferences::setFullName, Qt::UniqueConnection);

    if(fullNameRequest->isAttributeReady())
    {
        setFullName(fullNameRequest->getFirstName(), fullNameRequest->getLastName());
    }
}

void Preferences::setFullName(const QString& newFirstName, const QString& newLastName)
{
    if (newFirstName != firstName())
    {
        setFirstName(newFirstName);
    }
    if (newLastName != lastName())
    {
        setLastName(newLastName);
    }
}
