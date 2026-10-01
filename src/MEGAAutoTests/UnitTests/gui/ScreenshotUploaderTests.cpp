#include "ScreenshotUploader.h"
#include <catch.hpp>

#include <QString>

#ifdef Q_OS_MACOS
#include "mega/localpath.h"
#endif

// The uploader recognizes the transfer that carries a capture by comparing the
// path it handed to the SDK with the path the SDK reports back
// (MegaTransfer::getPath()). Those are not always the same string: on macOS the
// SDK reports names decomposed (NFD), while Qt, and so the capture detector,
// produces them precomposed (NFC). A capture whose name had an accented letter
// was never recognized, and every later capture stayed queued behind it.
//
// Non-ASCII input is written as UTF-8 byte escapes, split from a following
// hex-digit letter, for the reasons given in ScreenCaptureDetectorTests.cpp.
namespace
{
// The French macOS capture name ("Capture d'ecran 2026-09-30 a 15.11.56.png",
// with its accents), precomposed as Qt reports it...
const char* const FRENCH_CAPTURE_NFC = "/Users/test/Desktop/Capture d\xE2\x80\x99\xC3\xA9"
                                       "cran 2026-09-30 \xC3\xA0 15.11.56.png";
// ...and decomposed, as the SDK reports it on macOS.
const char* const FRENCH_CAPTURE_NFD = "/Users/test/Desktop/Capture d\xE2\x80\x99"
                                       "e\xCC\x81"
                                       "cran 2026-09-30 a\xCC\x80 15.11.56.png";
}

TEST_CASE("isSameLocalPath() matches a capture whatever its Unicode normalization")
{
    const QString french = QString::fromUtf8(FRENCH_CAPTURE_NFC);
    CHECK(ScreenshotUploader::isSameLocalPath(french, FRENCH_CAPTURE_NFD));
    CHECK(ScreenshotUploader::isSameLocalPath(french, FRENCH_CAPTURE_NFC));

    // Hangul syllables decompose into jamo, so a Korean name differs throughout.
    const QString korean = QString::fromUtf8(
        "/Users/test/Desktop/\xEC\x8A\xA4\xED\x81\xAC\xEB\xA6\xB0\xEC\x83\xB7 2026.png");
    const QByteArray koreanDecomposed = korean.normalized(QString::NormalizationForm_D).toUtf8();
    CHECK(ScreenshotUploader::isSameLocalPath(korean, koreanDecomposed.constData()));

    CHECK(ScreenshotUploader::isSameLocalPath(QString::fromUtf8("/a/Screenshot 1.png"),
                                              "/a/Screenshot 1.png"));
}

TEST_CASE("isSameLocalPath() still tells different files apart")
{
    const QString french = QString::fromUtf8(FRENCH_CAPTURE_NFC);
    // The next capture, one second later.
    CHECK_FALSE(ScreenshotUploader::isSameLocalPath(french,
                                                    "/Users/test/Desktop/Capture d\xE2\x80\x99"
                                                    "e\xCC\x81"
                                                    "cran 2026-09-30 a\xCC\x80 15.11.57.png"));
    CHECK_FALSE(ScreenshotUploader::isSameLocalPath(QString::fromUtf8("/a/Screenshot 1.png"),
                                                    "/b/Screenshot 1.png"));
    CHECK_FALSE(ScreenshotUploader::isSameLocalPath(QString(), nullptr));
}

#ifdef Q_OS_MACOS
TEST_CASE("isSameLocalPath() accepts the path exactly as the SDK reports it on macOS")
{
    // MegaTransfer::getPath() of an upload is LocalPath::toPath(false) of the
    // path the upload was started with.
    const QString fromQt = QString::fromUtf8(FRENCH_CAPTURE_NFC);
    const std::string fromSdk =
        mega::LocalPath::fromAbsolutePath(fromQt.toUtf8().toStdString()).toPath(false);
    CHECK(ScreenshotUploader::isSameLocalPath(fromQt, fromSdk.c_str()));
}
#endif
