#include "Preferences.h"
#include "platform/Platform.h"

#include <QDesktopServices>
#include <assert.h>

using namespace mega;

#ifdef WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#endif

const char Preferences::CLIENT_KEY[] = "FhMgXbqb";
const char Preferences::USER_AGENT[] = "MEGAsync/3.1.4.0";
const int Preferences::VERSION_CODE = 3104;
const int Preferences::BUILD_ID = 0;
// Do not change the location of VERSION_STRING, create_tarball.sh parses this file
const QString Preferences::VERSION_STRING = QString::fromAscii("3.1.4");
const QString Preferences::SDK_ID = QString::fromAscii("82c965");
const QString Preferences::CHANGELOG = QString::fromUtf8(
            "- Support for Apple File System (macOS High Sierra)\n"
            "- Updated translations"
            "- Bug fixes");

const QString Preferences::TRANSLATION_FOLDER = QString::fromAscii("://translations/");
const QString Preferences::TRANSLATION_PREFIX = QString::fromAscii("MEGASyncStrings_");

const int Preferences::STATE_REFRESH_INTERVAL_MS        = 10000;
const int Preferences::FINISHED_TRANSFER_REFRESH_INTERVAL_MS        = 10000;

const long long Preferences::MIN_UPDATE_STATS_INTERVAL  = 300000;
const long long Preferences::MIN_UPDATE_STATS_INTERVAL_OVERQUOTA    = 30000;
const long long Preferences::MIN_UPDATE_NOTIFICATION_INTERVAL_MS    = 172800000;
const long long Preferences::MIN_REBOOT_INTERVAL_MS                 = 300000;
const long long Preferences::MIN_EXTERNAL_NODES_WARNING_MS          = 60000;

const unsigned int Preferences::UPDATE_INITIAL_DELAY_SECS           = 60;
const unsigned int Preferences::UPDATE_RETRY_INTERVAL_SECS          = 7200;
const unsigned int Preferences::UPDATE_TIMEOUT_SECS                 = 600;
const unsigned int Preferences::MAX_LOGIN_TIME_MS                   = 40000;
const unsigned int Preferences::PROXY_TEST_TIMEOUT_MS               = 10000;
const unsigned int Preferences::LOCAL_HTTPS_TEST_TIMEOUT_MS         = 10000;
const unsigned int Preferences::MAX_IDLE_TIME_MS                    = 600000;
const unsigned int Preferences::MAX_COMPLETED_ITEMS                 = 1000;

const qint16 Preferences::HTTPS_PORT = 6342;

const QString Preferences::defaultHttpsKey = QString::fromUtf8(
            "-----BEGIN PRIVATE KEY-----\n"
            "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCxk9BUskgy2dHN\n"
            "udvrzqsj3614UniXjAw3skMP+xUULBgaTcGgQkYRMM6I7ro/9Ropq5ypK5p8cmUq\n"
            "TYhu/9Z4RP/asHDIMC7lIqIvgoZXQWdJHAhJ9KBgnYi+jQN0Q6IQBad9WR/dKMcu\n"
            "lnN0oMyn82rO9owV47dYVzdQnPE1k1z1HqzKyqrBjHvGfSQyMA2YCuSl1K3EzQY8\n"
            "NFYXeu3ZjwFCPvuDmRpLqkgKBrvwHPu0zaQN7WliNBz+8JnBt6PUNNvTZ0y/iYUR\n"
            "T3k4SHlhi/xXdV9jqjAN433RMIrGIWpFnBoJpaTFbTsU3z6Nj5BU/TYQpaBOieiT\n"
            "fO21acl7AgMBAAECggEAePMy7N1Zq7kM29EB6AUmMBMD9nZFyQMMt0rlvpbH7qtx\n"
            "50ia32sEimTx5/aiSTnKjiNjWx5l5OmN2lhg2ynKjLSCiBOxXcjjyBHk4fNHSVs0\n"
            "3GkJhEXojqX+I7usZJ5EXiFbyVbRCzPhJuw2y3NSsfpr/3eSRr6JfNQ6yt96s3/+\n"
            "jCI05FP+3lR4BMiBIqHwEUXJeEU1tpR33qgg10ii18NgqQcLir2I/xaT7nzCvkBl\n"
            "bxZYLEtM5dWL1J79qCOS/WlDvFSjlzh0BkkAhNYy7SChjPGOu0r/k07A2ec9KxO0\n"
            "971B0tip4O+w8OjHDZvV80QpCXGKcQfEtd+/mBScgQKBgQDlmQ9PYOb8GSC2Yttg\n"
            "XyIkisCqbUamIsojFL70U0wnkygykQXTkxiHYPtMVg4WyyLfF7oPqn0D0YOYEd+e\n"
            "RFYiHdr+CA4IopsWZ5I4HqYtrbwtapMRQPFRzNIMNdMpFlfBGRV9ZKQQtXHV445w\n"
            "n32KCUZVO26A6YYvmzmo7QR5WwKBgQDF/13BvTPlm+k0m2hheUWpiQqHrYW/oAai\n"
            "rK7iVmT4Qln1YOCUvt17mDRxXj17NEKR4uVcDix5yrbFuClpeDZZkq1/2HIB5bD9\n"
            "1uQWVzlYnCytaYusfq3XKfcwG/GIt6mi3lstZV4XdEFxjczdWt7EeWwAAr+DfYw6\n"
            "pAYruMLKYQKBgDYs75fjXZ2OsFFY8jrDN+M3ek68ijcZbmjotEYigY21A38rCRzr\n"
            "UJZhI+rXQ2vNcuUBTD32FJmaDlsLnKBTr59NPCdE70rKU53twmrLkJqmrAhrZVhk\n"
            "4oxSsB2Bddn0E7DUomV8IdpvdfTqROn+ODkiBx6Fb4WrlKYXEnvxsWSRAoGAV1av\n"
            "F+AK/XTJ6R/Ian7hQMinsXPUtNO1OZrsxgCQJ4a1Qe1LA7Ix5uwb7gpBGpDR8KJi\n"
            "xDmoWs0V1J/I/LI/X0G5cNScbcPRUBezozs0m6bAeno9V4jFzEzBsiIRaFqD5Mkq\n"
            "9Rpq5/OrTpjbTqVf8NES1+peanU+HzvtUOn+WuECgYEAxj++fk8A+dimgwM1qzh0\n"
            "chTWqZygTQNvMAId11nRVJqC2nVOt1vSnM4HBBd+6qxG6eKOxzG4VyYJuHd18AT8\n"
            "RplMrMlFmIJpvXRoFIgoPPS7qUnZcjEidPndXG42+xbD8eDsHnG4LMHB9+J9xLwA\n"
            "xs+OLGD+kjzh+HkpkzYSDp4=\n"
            "-----END PRIVATE KEY-----\n");

const QString Preferences::defaultHttpsCert = QString::fromUtf8(
            "-----BEGIN CERTIFICATE-----\n"
            "MIIGADCCBOigAwIBAgIQL41amoCH4B2agSUpD8Wd2DANBgkqhkiG9w0BAQsFADBC\n"
            "MQswCQYDVQQGEwJVUzEWMBQGA1UEChMNR2VvVHJ1c3QgSW5jLjEbMBkGA1UEAxMS\n"
            "UmFwaWRTU0wgU0hBMjU2IENBMB4XDTE3MDQwNTAwMDAwMFoXDTE5MDcwNTIzNTk1\n"
            "OVowLTErMCkGA1UEAwwibG9jYWxob3N0Lm1lZ2FzeW5jbG9vcGJhY2subWVnYS5u\n"
            "ejCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALGT0FSySDLZ0c252+vO\n"
            "qyPfrXhSeJeMDDeyQw/7FRQsGBpNwaBCRhEwzojuuj/1GimrnKkrmnxyZSpNiG7/\n"
            "1nhE/9qwcMgwLuUioi+ChldBZ0kcCEn0oGCdiL6NA3RDohAFp31ZH90oxy6Wc3Sg\n"
            "zKfzas72jBXjt1hXN1Cc8TWTXPUerMrKqsGMe8Z9JDIwDZgK5KXUrcTNBjw0Vhd6\n"
            "7dmPAUI++4OZGkuqSAoGu/Ac+7TNpA3taWI0HP7wmcG3o9Q029NnTL+JhRFPeThI\n"
            "eWGL/Fd1X2OqMA3jfdEwisYhakWcGgmlpMVtOxTfPo2PkFT9NhCloE6J6JN87bVp\n"
            "yXsCAwEAAaOCAwUwggMBMC0GA1UdEQQmMCSCImxvY2FsaG9zdC5tZWdhc3luY2xv\n"
            "b3BiYWNrLm1lZ2EubnowCQYDVR0TBAIwADArBgNVHR8EJDAiMCCgHqAchhpodHRw\n"
            "Oi8vZ3Auc3ltY2IuY29tL2dwLmNybDBvBgNVHSAEaDBmMGQGBmeBDAECATBaMCoG\n"
            "CCsGAQUFBwIBFh5odHRwczovL3d3dy5yYXBpZHNzbC5jb20vbGVnYWwwLAYIKwYB\n"
            "BQUHAgIwIAweaHR0cHM6Ly93d3cucmFwaWRzc2wuY29tL2xlZ2FsMB8GA1UdIwQY\n"
            "MBaAFJfCJ1CewsnsDIgyyHyt4qYBT9pvMA4GA1UdDwEB/wQEAwIFoDAdBgNVHSUE\n"
            "FjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwVwYIKwYBBQUHAQEESzBJMB8GCCsGAQUF\n"
            "BzABhhNodHRwOi8vZ3Auc3ltY2QuY29tMCYGCCsGAQUFBzAChhpodHRwOi8vZ3Au\n"
            "c3ltY2IuY29tL2dwLmNydDCCAXwGCisGAQQB1nkCBAIEggFsBIIBaAFmAHUA3esd\n"
            "K3oNT6Ygi4GtgWhwfi6OnQHVXIiNPRHEzbbsvswAAAFbO+z6WwAABAMARjBEAiBy\n"
            "3jqWJoo6+o4nXCv1R1vSrXwOub7i4zSnjzKQmNuAXAIgF8Fmvm1EB48X9g1qfx+k\n"
            "sZsdKJRwGcJEFf55onTQlkgAdQCkuQmQtBhYFIe7E6LMZ3AKPDWYBPkb37jjd80O\n"
            "yA3cEAAAAVs77PqaAAAEAwBGMEQCID/pb8/gqDng+2dJex8pHY2qxCyen3u4Su7d\n"
            "zPkeHKVFAiBYpUot0v/eWwy6yTODgT9FdDPe6TJfkaounob/gDBTZgB2AO5Lvbd1\n"
            "zmC64UJpH6vhnmajD35fsHLYgwDEe4l6qP3LAAABWzvs/FoAAAQDAEcwRQIgKHq8\n"
            "JFAOYbf912dJdbP7h1KkahVBQqOOMJgra1HY6b4CIQD5ybPjzCdqla5srDUIwvpm\n"
            "B2dPoTqSNx70+pVrmAI60zANBgkqhkiG9w0BAQsFAAOCAQEAeeqe3o38ZcVaOiSM\n"
            "sv8o1aHJN25jYRVtvm04wlrXRJg90CmfNiaLD+7UeuDtZxcvYmkNA6Vz4NVj5s8b\n"
            "kRexf64UIhb1fWUr+kgI/gWWcsGAwqn9i3Hs6CIAMvic9bhedCBesef01hTqD1sq\n"
            "8cN+YZy2fLRcx3NIQ9UpCsSUDkf+1bCjCVbBtUqhJ8zUG6l/TwLEsnXR0GQcBzc1\n"
            "Elav7ka8GpvbJTuivY4e537WWqM4RAmO5xI+uk9ANwkPFRy7awRSuF2mJTz49GOg\n"
            "MGA+fCc/TGwZF1syej90ZTKYbRRkyrkSHjehfXW8fr23Y9/OW9u8nV8jm7WPKjYX\n"
            "Nf2RSQ==\n"
            "-----END CERTIFICATE-----\n");

const QString Preferences::defaultHttpsCertIntermediate = QString::fromUtf8(
            "-----BEGIN CERTIFICATE-----\n"
            "MIIETTCCAzWgAwIBAgIDAjpxMA0GCSqGSIb3DQEBCwUAMEIxCzAJBgNVBAYTAlVTMRYwFAYDVQQK\n"
            "Ew1HZW9UcnVzdCBJbmMuMRswGQYDVQQDExJHZW9UcnVzdCBHbG9iYWwgQ0EwHhcNMTMxMjExMjM0\n"
            "NTUxWhcNMjIwNTIwMjM0NTUxWjBCMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNR2VvVHJ1c3QgSW5j\n"
            "LjEbMBkGA1UEAxMSUmFwaWRTU0wgU0hBMjU2IENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\n"
            "CgKCAQEAu1jBEgEul9h9GKrIwuWF4hdsYC7JjTEFORoGmFbdVNcRjFlbPbFUrkshhTIWX1SG5tmx\n"
            "2GCJa1i+ctqgAEJ2sSdZTM3jutRc2aZ/uyt11UZEvexAXFm33Vmf8Wr3BvzWLxmKlRK6msrVMNI4\n"
            "/Bk7WxU7NtBDTdFlodSLwWBBs9ZwF8w5wJwMoD23ESJOztmpetIqYpygC04q18NhWoXdXBC5VD0t\n"
            "A/hJ8LySt7ecMcfpuKqCCwW5Mc0IW7siC/acjopVHHZDdvDibvDfqCl158ikh4tq8bsIyTYYZe5Q\n"
            "Q7hdctUoOeFTPiUs2itP3YqeUFDgb5rE1RkmiQF1cwmbOwIDAQABo4IBSjCCAUYwHwYDVR0jBBgw\n"
            "FoAUwHqYaI2J+6sFZAwRfap9ZbjKzE4wHQYDVR0OBBYEFJfCJ1CewsnsDIgyyHyt4qYBT9pvMBIG\n"
            "A1UdEwEB/wQIMAYBAf8CAQAwDgYDVR0PAQH/BAQDAgEGMDYGA1UdHwQvMC0wK6ApoCeGJWh0dHA6\n"
            "Ly9nMS5zeW1jYi5jb20vY3Jscy9ndGdsb2JhbC5jcmwwLwYIKwYBBQUHAQEEIzAhMB8GCCsGAQUF\n"
            "BzABhhNodHRwOi8vZzIuc3ltY2IuY29tMEwGA1UdIARFMEMwQQYKYIZIAYb4RQEHNjAzMDEGCCsG\n"
            "AQUFBwIBFiVodHRwOi8vd3d3Lmdlb3RydXN0LmNvbS9yZXNvdXJjZXMvY3BzMCkGA1UdEQQiMCCk\n"
            "HjAcMRowGAYDVQQDExFTeW1hbnRlY1BLSS0xLTU2OTANBgkqhkiG9w0BAQsFAAOCAQEANevhiyBW\n"
            "lLp6vXmp9uP+bji0MsGj21hWID59xzqxZ2nVeRQb9vrsYPJ5zQoMYIp0TKOTKqDwUX/N6fmS/Zar\n"
            "RfViPT9gRlATPSATGC6URq7VIf5Dockj/lPEvxrYrDrK3maXI67T30pNcx9vMaJRBBZqAOv5jUOB\n"
            "8FChH6bKOvMoPF9RrNcKRXdLDlJiG9g4UaCSLT+Qbsh+QJ8gRhVd4FB84XavXu0R0y8TubglpK9Y\n"
            "Ca81tGJUheNI3rzSkHp6pIQNo0LyUcDUrVNlXWz4Px8G8k/Ll6BKWcZ40egDuYVtLLrhX7atKz4l\n"
            "ecWLVtXjCYDqwSfC2Q7sRwrp0Mr82A==\n"
            "-----END CERTIFICATE----- \n");

const long long Preferences::defaultHttpsCertExpiration = 1562378399;
const long long Preferences::LOCAL_HTTPS_CERT_MAX_EXPIRATION_SECS = 3888000; // 45 days

QStringList Preferences::HTTPS_ALLOWED_ORIGINS;
bool Preferences::HTTPS_ORIGIN_CHECK_ENABLED = true;

#ifdef WIN32
    const QString Preferences::UPDATE_CHECK_URL                 = QString::fromUtf8("http://g.static.mega.co.nz/upd/wsync/v.txt");
#else
    const QString Preferences::UPDATE_CHECK_URL                 = QString::fromUtf8("http://g.static.mega.co.nz/upd/msync/v.txt");
#endif

const char Preferences::UPDATE_PUBLIC_KEY[] = "EACTzXPE8fdMhm6LizLe1FxV2DncybVh2cXpW3momTb8tpzRNT833r1RfySz5uHe8gdoXN1W0eM5Bk8X-LefygYYDS9RyXrRZ8qXrr9ITJ4r8ATnFIEThO5vqaCpGWTVi5pOPI5FUTJuhghVKTyAels2SpYT5CmfSQIkMKv7YVldaV7A-kY060GfrNg4--ETyIzhvaSZ_jyw-gmzYl_dwfT9kSzrrWy1vQG8JPNjKVPC4MCTZJx9SNvp1fVi77hhgT-Mc5PLcDIfjustlJkDBHtmGEjyaDnaWQf49rGq94q23mLc56MSjKpjOR1TtpsCY31d1Oy2fEXFgghM0R-1UkKswVuWhEEd8nO2PimJOl4u9ZJ2PWtJL1Ro0Hlw9OemJ12klIAxtGV-61Z60XoErbqThwWT5Uu3D2gjK9e6rL9dufSoqjC7UA2C0h7KNtfUcUHw0UWzahlR8XBNFXaLWx9Z8fRtA_a4seZcr0AhIA7JdQG5i8tOZo966KcFnkU77pfQTSprnJhCfEmYbWm9EZA122LJBWq2UrSQQN3pKc9goNaaNxy5PYU1yXyiAfMVsBDmDonhRWQh2XhdV-FWJ3rOGMe25zOwV4z1XkNBuW4T1JF2FgqGR6_q74B2ccFC8vrNGvlTEcs3MSxTI_EKLXQvBYy7hxG8EPUkrMVCaWzzTQAFEQ";
const QString Preferences::CRASH_REPORT_URL                 = QString::fromUtf8("http://g.api.mega.co.nz/hb?crashdump");
const QString Preferences::UPDATE_FOLDER_NAME               = QString::fromAscii("update");
const QString Preferences::UPDATE_BACKUP_FOLDER_NAME        = QString::fromAscii("backup");
const QString Preferences::PROXY_TEST_URL                   = QString::fromUtf8("http://eu.static.mega.co.nz/?");
const QString Preferences::PROXY_TEST_SUBSTRING             = QString::fromUtf8("<title>MEGA</title>");
const QString Preferences::LOCAL_HTTPS_TEST_URL             = QString::fromUtf8("https://localhost.megasyncloopback.mega.nz:") + QString::number(Preferences::HTTPS_PORT);
const QString Preferences::LOCAL_HTTPS_TEST_SUBSTRING       = Preferences::VERSION_STRING;
const QString Preferences::LOCAL_HTTPS_TEST_POST_DATA       = QString::fromUtf8("{\"a\":\"v\"}");
const QString Preferences::syncsGroupKey            = QString::fromAscii("Syncs");
const QString Preferences::currentAccountKey        = QString::fromAscii("currentAccount");
const QString Preferences::emailKey                 = QString::fromAscii("email");
const QString Preferences::emailHashKey             = QString::fromAscii("emailHash");
const QString Preferences::privatePwKey             = QString::fromAscii("privatePw");
const QString Preferences::totalStorageKey          = QString::fromAscii("totalStorage");
const QString Preferences::usedStorageKey           = QString::fromAscii("usedStorage");
const QString Preferences::cloudDriveStorageKey     = QString::fromAscii("cloudDriveStorage");
const QString Preferences::inboxStorageKey          = QString::fromAscii("inboxStorage");
const QString Preferences::rubbishStorageKey        = QString::fromAscii("rubbishStorage");
const QString Preferences::inShareStorageKey        = QString::fromAscii("inShareStorage");
const QString Preferences::cloudDriveFilesKey       = QString::fromAscii("cloudDriveFiles");
const QString Preferences::inboxFilesKey            = QString::fromAscii("inboxFiles");
const QString Preferences::rubbishFilesKey          = QString::fromAscii("rubbishFiles");
const QString Preferences::inShareFilesKey          = QString::fromAscii("inShareFiles");
const QString Preferences::cloudDriveFoldersKey     = QString::fromAscii("cloudDriveFolders");
const QString Preferences::inboxFoldersKey          = QString::fromAscii("inboxFolders");
const QString Preferences::rubbishFoldersKey        = QString::fromAscii("rubbishFolders");
const QString Preferences::inShareFoldersKey        = QString::fromAscii("inShareFolders");
const QString Preferences::totalBandwidthKey        = QString::fromAscii("totalBandwidth");
const QString Preferences::usedBandwidthKey         = QString::fromAscii("usedBandwidth");
const QString Preferences::accountTypeKey           = QString::fromAscii("accountType");
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
const QString Preferences::lastVersionKey           = QString::fromAscii("lastVersion");
const QString Preferences::lastStatsRequestKey      = QString::fromAscii("lastStatsRequest");
const QString Preferences::lastUpdateTimeKey        = QString::fromAscii("lastUpdateTime");
const QString Preferences::lastUpdateVersionKey     = QString::fromAscii("lastUpdateVersion");
const QString Preferences::previousCrashesKey       = QString::fromAscii("previousCrashes");
const QString Preferences::lastRebootKey            = QString::fromAscii("lastReboot");
const QString Preferences::lastExitKey              = QString::fromAscii("lastExit");
const QString Preferences::disableOverlayIconsKey   = QString::fromAscii("disableOverlayIcons");
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
const QString Preferences::httpsKeyKey              = QString::fromAscii("httpsKey");
const QString Preferences::httpsCertKey             = QString::fromAscii("httpsCert");
const QString Preferences::httpsCertIntermediateKey = QString::fromAscii("httpsCertIntermediate");
const QString Preferences::httpsCertExpirationKey   = QString::fromAscii("httpsCertExpiration");

const bool Preferences::defaultShowNotifications    = false;
const bool Preferences::defaultStartOnStartup       = true;
const bool Preferences::defaultUpdateAutomatically  = true;
const bool Preferences::defaultUpperSizeLimit       = false;
const bool Preferences::defaultLowerSizeLimit       = false;
const bool Preferences::defaultUseHttpsOnly         = false;
const bool Preferences::defaultSSLcertificateException = false;
const int  Preferences::defaultUploadLimitKB        = -1;
const int  Preferences::defaultDownloadLimitKB      = 0;
const int  Preferences::defaultParallelUploadConnections      = 3;
const int  Preferences::defaultParallelDownloadConnections    = 4;
const int Preferences::defaultTransferDownloadMethod      = MegaApi::TRANSFER_METHOD_AUTO;
const int Preferences::defaultTransferUploadMethod        = MegaApi::TRANSFER_METHOD_AUTO;
const long long  Preferences::defaultUpperSizeLimitValue              = 0;
const long long  Preferences::defaultLowerSizeLimitValue              = 0;
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
        Preferences::HTTPS_ALLOWED_ORIGINS.append(QString::fromUtf8("https://mega.nz"));
        Preferences::HTTPS_ALLOWED_ORIGINS.append(QString::fromUtf8("https://mega.co.nz"));
        Preferences::HTTPS_ALLOWED_ORIGINS.append(QString::fromUtf8("chrome-extension://kpgogfgfingilcbkpahnggpfdabapnol"));
        Preferences::HTTPS_ALLOWED_ORIGINS.append(QString::fromUtf8("chrome-extension://bigefpfhnfcobdlfbedofhhaibnlghod"));

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

    if (settings->value(lastVersionKey).toInt() < 108)
    {
        excludedSyncNames.clear();
        excludedSyncNames.append(QString::fromUtf8("Thumbs.db"));
        excludedSyncNames.append(QString::fromUtf8("desktop.ini"));
        excludedSyncNames.append(QString::fromUtf8("~*"));
        excludedSyncNames.append(QString::fromUtf8(".*"));
    }

    if (settings->value(lastVersionKey).toInt() < 2907)
    {
        //This string is no longer excluded by default since 2907
        excludedSyncNames.removeAll(QString::fromUtf8("Icon?"));
    }

    QSet<QString> excludedSyncNamesSet = QSet<QString>::fromList(excludedSyncNames);
    excludedSyncNames = excludedSyncNamesSet.toList();
    qSort(excludedSyncNames.begin(), excludedSyncNames.end(), caseInsensitiveLessThan);

    setExcludedSyncNames(excludedSyncNames);
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
