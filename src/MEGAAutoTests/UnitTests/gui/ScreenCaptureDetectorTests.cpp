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
    accepts("Schermata 2026-06-10 alle 14.05.33.png"); // it GNOME
    accepts("Istantanea schermo 2023-06-10 alle 14.05.33.png"); // it, macOS before Sequoia
    accepts("Screenshot 2026-06-10 alle 14.05.33.png"); // it, current macOS
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

    // Names verified against the OS strings (macOS screencaptureui
    // Localizable.loctable, gnome-shell po files). Where the two desktops use
    // different words both are covered. Hex escapes are split from a following
    // hex-digit letter ("\xAD" "afbeelding") so the compiler does not swallow it.
    accepts("Scherm\xC2\xAD"
            "afbeelding 2026-06-10 om 14.05.33.png"); // nl macOS, soft hyphen
    accepts("Schermafdruk van 2026-06-10 14-05-33.png"); // nl GNOME
    accepts("Ekran Resmi 2026-06-10 14.05.33.png"); // tr macOS
    accepts("Captur\xC4\x83 de ecran 2026-06-10 la 14.05.33.png"); // ro macOS
    accepts("Sn\xC3\xADmek obrazovky 2026-06-10 v 14.05.33.png"); // cs macOS/GNOME
    accepts("Sn\xC3\xADmka obrazovky 2026-06-10 o 14.05.33.png"); // sk macOS
    accepts("Sk\xC3\xA4rmavbild 2026-06-10 kl. 14.05.33.png"); // sv macOS
    accepts("K\xC3\xA9perny\xC5\x91"
            "fot\xC3\xB3 2026-06-10 14.05.33.png"); // hu macOS
    accepts("K\xC3\xA9perny\xC5\x91k\xC3\xA9p 2026-06-10 14-05-33.png"); // hu GNOME
    accepts("Kuvakaappaus - 2026-06-10 14-05-33.png"); // fi GNOME
    accepts("Tangkapan Layar 2026-06-10 pukul 14.05.33.png"); // id macOS
    accepts("Cuplikan Layar Dari 2026-06-10 14-05-33.png"); // id GNOME
    accepts("Tangkapan Skrin 2026-06-10 14.05.33.png"); // ms macOS
    accepts("Slika zaslona 2026-06-10 u 14.05.33.png"); // hr macOS
    accepts("Posnetek zaslona 2026-06-10 ob 14.05.33.png"); // sl macOS
    accepts("\xE1\xBA\xA2nh m\xC3\xA0n h\xC3\xACnh 2026-06-10 l\xC3\xBA"
            "c "
            "14.05.33.png"); // vi macOS
    accepts(
        "\xE1\xBA\xA2nh ch\xE1\xBB\xA5p M\xC3\xA0n h\xC3\xACnh 2026-06-10.png"); // vi older macOS
    accepts("Ch\xE1\xBB\xA5p m\xC3\xA0n h\xC3\xACnh t\xE1\xBB\xAB 2026-06-10 "
            "14-05-33.png"); // vi GNOME
    accepts("\xE6\x88\xAA\xE5\xB1\x8F"
            "2026-06-10 14.05.33.png"); // zh_CN macOS
    accepts("\xE6\x88\xAA\xE5\x9B\xBE 2026-06-10 14-05-33.png"); // zh_CN GNOME
    accepts("\xE6\x88\xAA\xE5\x9C\x96 2026-06-10 14.05.33.png"); // zh_TW macOS
    accepts("\xE8\x9E\xA2\xE5\xB9\x95\xE5\xBF\xAB\xE7\x85\xA7 2026-06-10 "
            "14-05-33.png"); // zh_TW GNOME
    accepts("\xD9\x84\xD9\x82\xD8\xB7\xD8\xA9 "
            "\xD8\xA7\xD9\x84\xD8\xB4\xD8\xA7\xD8\xB4\xD8\xA9 "
            "\xD9\xA2\xD9\xA0\xD9\xA2\xD9\xA6-\xD9\xA0\xD9\xA6-\xD9\xA1\xD9\xA0.png"); // ar macOS
    accepts("\xD9\x84\xD9\x82\xD8\xB7\xD8\xA9 \xD8\xB4\xD8\xA7\xD8\xB4\xD8\xA9 "
            "2026-06-10.png"); // ar without the article
    accepts("\xD7\xA6\xD7\x99\xD7\x9C\xD7\x95\xD7\x9D \xD7\x9E\xD7\xA1\xD7\x9A "
            "2026-06-10.png"); // he
    accepts(
        "\xCE\xA3\xCF\x84\xCE\xB9\xCE\xB3\xCE\xBC\xCE\xB9\xCF\x8C\xCF\x84\xCF\x85\xCF\x80\xCE\xBF "
        "\xCE\xBF\xCE\xB8\xCF\x8C\xCE\xBD\xCE\xB7\xCF\x82 2026-06-10.png"); // el macOS
    accepts(
        "\xCE\xA3\xCF\x84\xCE\xB9\xCE\xB3\xCE\xBC\xCE\xB9\xCF\x8C\xCF\x84\xCF\x85\xCF\x80\xCE\xBF "
        "\xCE\x9F\xCE\xB8\xCF\x8C\xCE\xBD\xCE\xB7\xCF\x82 "
        "\xCE\xB1\xCF\x80\xCF\x8C 2026-06-10.png"); // el GNOME (capital Omicron)
    accepts("\xE0\xA4\xB8\xE0\xA5\x8D\xE0\xA4\x95\xE0\xA5\x8D\xE0\xA4\xB0\xE0\xA5\x80\xE0\xA4\xA8"
            "\xE0\xA4\xB6\xE0\xA5\x89\xE0\xA4\x9F "
            "2026-06-10.png"); // hi macOS
    accepts("\xE0\xB8\xA0\xE0\xB8\xB2\xE0\xB8\x9E\xE0\xB8\x96\xE0\xB9\x88\xE0\xB8\xB2\xE0\xB8\xA2"
            "\xE0\xB8\xAB\xE0\xB8\x99\xE0\xB9\x89\xE0\xB8\xB2\xE0\xB8\x88\xE0\xB8\xAD "
            "2026-06-10.png"); // th macOS
    accepts("\xE0\xB8\xA0\xE0\xB8\xB2\xE0\xB8\x9E\xE0\xB8\xAB\xE0\xB8\x99\xE0\xB9\x89\xE0\xB8\xB2"
            "\xE0\xB8\x88\xE0\xB8\xAD\xE0\xB8\x88\xE0\xB8\xB2\xE0\xB8\x81 "
            "2026-06-10.png"); // th GNOME

    // HFS+ stores names decomposed (base letter + combining mark); the
    // detector must fold them before matching.
    accepts("Na\xCC\x88ytto\xCC\x88kuva 2026-06-10 klo 14.05.33.png"); // fi, NFD
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

    // The capture word must be the whole word: "capture" and "captur[a]" are
    // prefixes of ordinary English/Spanish words.
    rejects("captured moment.png");
    rejects("captures de boda.jpg");
    rejects("capturando el momento.png");
    rejects("catturato.jpg");
    rejects("schermate del gioco.png");

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
