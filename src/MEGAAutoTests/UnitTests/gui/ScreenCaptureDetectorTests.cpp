#include "ScreenCaptureDetector.h"
#include <catch.hpp>

#include <QDir>

// Tests for the three pure decisions that gate automatic screenshot upload.
//
// These matter more than their size suggests: a false positive here means a file
// the user never captured is uploaded to their account and, when link copying is
// enabled, exported behind a public link. Every "reject" case below is a file
// that must never leave the machine on its own.
//
// Non-ASCII expectations are written as UTF-8 byte escapes rather than literal
// characters: the build does not force a UTF-8 source charset on MSVC, so a raw
// non-ASCII literal would not survive every compiler. This also cross-checks the
// "\x{...}" codepoint escapes used in the production pattern -- a wrong
// codepoint there fails here rather than silently disabling a locale.
class ScreenCaptureDetectorTests
{
protected:
    void accepts(const char* fileName)
    {
        INFO("should be recognised as a capture: " << fileName);
        CHECK(ScreenCaptureDetector::looksLikeScreenshot(QString::fromUtf8(fileName)));
    }

    void rejects(const char* fileName)
    {
        INFO("must NOT be recognised as a capture: " << fileName);
        CHECK_FALSE(ScreenCaptureDetector::looksLikeScreenshot(QString::fromUtf8(fileName)));
    }

    void isShared(const QString& path)
    {
        INFO("must be treated as shared (name filter applies): " << path.toStdString());
        CHECK(ScreenCaptureDetector::isSharedDirectory(path));
    }

    void isDedicated(const QString& path)
    {
        INFO("should be treated as a dedicated capture folder: " << path.toStdString());
        CHECK_FALSE(ScreenCaptureDetector::isSharedDirectory(path));
    }

    QString home() const
    {
        return QDir::cleanPath(QDir::homePath());
    }
};

TEST_CASE_METHOD(ScreenCaptureDetectorTests, "looksLikeScreenshot() accepts OS capture names")
{
    // English, across platforms.
    accepts("Screenshot 2026-06-10 at 14.05.33.png"); // macOS
    accepts("Screen Shot 2019-01-01 at 09.10.11.png"); // macOS, older naming
    accepts("Screenshot (12).png"); // Windows, Win+PrtScn
    accepts("Screenshot 2026-06-10 140533.png"); // Windows Snipping Tool
    accepts("Screenshot from 2026-06-10 14-05-33.png"); // GNOME
    accepts("Screenshot_20260817_125236.png"); // KDE Spectacle
    accepts("Screenshot_2026-06-10-14-05-33.png"); // Xfce
    accepts("screenshot.png"); // lower case, no timestamp

    // Localized names. A capture tool writes these for a user whose desktop is
    // not in English; they must not be second-class.
    accepts("Captura de pantalla 2026-06-10 a las 14.05.33.png"); // es
    accepts("Capture d'ecran 2026-06-10 a 14.05.33.png"); // fr
    accepts("Bildschirmfoto vom 2026-06-10 14-05-33.png"); // de
    accepts("Schermafbeelding 2026-06-10 om 14.05.33.png"); // nl
    accepts("Zrzut ekranu z 2026-06-10 14-05-33.png"); // pl
    accepts("Schermata 2026-06-10 alle 14.05.33.png"); // it
    accepts("Sk\xC3\xA4rmbild 2026-06-10 kl. 14.05.33.png"); // sv
    accepts("N\xC3\xA4ytt\xC3\xB6kuva 2026-06-10 klo 14.05.33.png"); // fi
    accepts("Ekran g\xC3\xB6r\xC3\xBCnt\xC3\xBCs\xC3\xBC 2026-06-10.png"); // tr
    accepts("\xD0\xA1\xD0\xBD\xD0\xB8\xD0\xBC\xD0\xBE\xD0\xBA "
            "\xD1\x8D\xD0\xBA\xD1\x80\xD0\xB0\xD0\xBD\xD0\xB0 2026-06-10.png"); // ru
    accepts("\xD0\x97\xD0\xBD\xD1\x96\xD0\xBC\xD0\xBE\xD0\xBA "
            "\xD0\xB5\xD0\xBA\xD1\x80\xD0\xB0\xD0\xBD\xD0\xB0 2026.png"); // uk
    accepts("\xE5\xB1\x8F\xE5\xB9\x95\xE6\x88\xAA\xE5\x9B\xBE 2026-06-10.png"); // zh-Hans
    accepts("\xE8\x9E\xA2\xE5\xB9\x95\xE6\x88\xAA\xE5\x9C\x96 2026.png"); // zh-Hant
    accepts("\xE3\x82\xB9\xE3\x82\xAF\xE3\x83\xAA\xE3\x83\xBC\xE3\x83\xB3"
            "\xE3\x82\xB7\xE3\x83\xA7\xE3\x83\x83\xE3\x83\x88 2026.png"); // ja
    accepts("\xEC\x8A\xA4\xED\x81\xAC\xEB\xA6\xB0\xEC\x83\xB7 2026.png"); // ko
}

TEST_CASE_METHOD(ScreenCaptureDetectorTests, "looksLikeScreenshot() rejects the user's own files")
{
    // Camera and phone exports. These carry a date and a time, which an
    // unanchored pattern accepted -- and in a shared folder such as the
    // Pictures root that meant uploading and publicly exporting family photos.
    rejects("IMG 2026-06-10 14.05.jpg");
    rejects("IMG_20260610_140533.jpg");
    rejects("DSC00123.jpg");
    rejects("2026-06-10 14-05-33.png"); // bare timestamp, no capture prefix
    rejects("vacation 2026-06-10 at 14.05.jpg");
    rejects("WhatsApp Image 2026-06-10 at 14.05.33.jpeg");
    rejects("scan 2026-06-10 14.05.png");

    // "captur" appearing anywhere used to be enough.
    rejects("family_capture.jpg");
    rejects("recaptured moment.png");
    rejects("photo-captura-familia.jpg");

    // The capture word must be at the START of the name, not merely present.
    rejects("my screenshot of the dog.jpg");
    rejects("edited Screenshot 2026-06-10.png");

    // Near misses that are not captures.
    rejects("Screen recording 2026-06-10.png");
    rejects("");
}

TEST_CASE_METHOD(ScreenCaptureDetectorTests, "isSharedDirectory() fails closed")
{
    // A folder is exempt from the name filter only when it is positively
    // recognised as capture-only. Anything else holds files of all kinds, so
    // new images there must still pass looksLikeScreenshot().
    isShared(home() + QString::fromUtf8("/Desktop"));
    isShared(home() + QString::fromUtf8("/Pictures"));
    isShared(home());

    // A capture location the user relocated onto a folder of their own files.
    // The previous list-membership test treated every one of these as a
    // dedicated capture folder and skipped the name filter entirely.
    isShared(home() + QString::fromUtf8("/Documents"));
    isShared(home() + QString::fromUtf8("/Captures"));
    isShared(home() + QString::fromUtf8("/Downloads"));

    // Only a folder named "Screenshots" accepts any new image, which is what
    // lets localized capture names through in a dedicated folder.
    isDedicated(home() + QString::fromUtf8("/Pictures/Screenshots"));
    isDedicated(QString::fromUtf8("/mnt/data/Screenshots"));

    // macOS and Windows filesystems are case-insensitive, so casing must not
    // decide whether the safety filter applies.
    isDedicated(home() + QString::fromUtf8("/Pictures/screenshots"));
    isDedicated(home() + QString::fromUtf8("/Pictures/SCREENSHOTS"));
}

TEST_CASE_METHOD(ScreenCaptureDetectorTests, "isSharedDirectory() ignores path spelling")
{
    // Callers normalize with QDir::cleanPath() before comparing, so a trailing
    // separator or a redundant segment must not change the verdict. Getting
    // this wrong is what silently disabled the name filter on the Desktop when
    // macOS reported its capture location as "~/Desktop/".
    const QString desktop = home() + QString::fromUtf8("/Desktop");
    isShared(QDir::cleanPath(desktop + QString::fromUtf8("/")));
    isShared(QDir::cleanPath(desktop + QString::fromUtf8("/.")));

    const QString dedicated = home() + QString::fromUtf8("/Pictures/Screenshots");
    isDedicated(QDir::cleanPath(dedicated + QString::fromUtf8("/")));
    isDedicated(QDir::cleanPath(dedicated + QString::fromUtf8("/.")));
}

// Not covered here: platformCaptureDirectories(). It reaches
// Platform::getInstance(), which needs more application setup than this harness
// provides and segfaults inside it -- the same way the pre-existing
// ScaleFactorManager cases in this suite do. Its normalization is exercised
// indirectly by the isSharedDirectory() spelling cases above, which assert the
// property that normalization exists to guarantee.

TEST_CASE_METHOD(ScreenCaptureDetectorTests, "isTooBroadToWatch() refuses whole-home watches")
{
    // XDG_PICTURES_DIR="$HOME/" makes PicturesLocation the home directory
    // itself. Watching that would put every new image anywhere in the user's
    // home under automatic upload and public export, with only a filename
    // heuristic in front of it.
    CHECK(ScreenCaptureDetector::isTooBroadToWatch(home()));
    CHECK(ScreenCaptureDetector::isTooBroadToWatch(home() + QString::fromUtf8("/")));
    CHECK(ScreenCaptureDetector::isTooBroadToWatch(QDir::rootPath()));

    // Ordinary capture folders are unaffected -- including a dedicated folder
    // directly under home, which is what still works when Pictures is $HOME.
    CHECK_FALSE(ScreenCaptureDetector::isTooBroadToWatch(home() + QString::fromUtf8("/Pictures")));
    CHECK_FALSE(ScreenCaptureDetector::isTooBroadToWatch(
        home() + QString::fromUtf8("/Pictures/Screenshots")));
    CHECK_FALSE(
        ScreenCaptureDetector::isTooBroadToWatch(home() + QString::fromUtf8("/Screenshots")));
}
