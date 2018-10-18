#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "mega/types.h"
#include "mega/crypto/cryptopp.h"
#include "mega.h"

#define KEY_LENGTH 4096
#define SIGNATURE_LENGTH 512

using namespace mega;
using namespace std;

const char SERVER_BASE_URL_WIN[] = "http://g.static.mega.co.nz/upd/wsync/";
const char SERVER_BASE_URL_OSX[] = "http://g.static.mega.co.nz/upd/msync/MEGAsync.app/";

const char *TARGET_PATHS_WIN[] = {
    "api-ms-win-core-console-l1-1-0.dll",
    "api-ms-win-core-datetime-l1-1-0.dll",
    "api-ms-win-core-debug-l1-1-0.dll",
    "api-ms-win-core-errorhandling-l1-1-0.dll",
    "api-ms-win-core-file-l1-1-0.dll",
    "api-ms-win-core-file-l1-2-0.dll",
    "api-ms-win-core-file-l2-1-0.dll",
    "api-ms-win-core-handle-l1-1-0.dll",
    "api-ms-win-core-heap-l1-1-0.dll",
    "api-ms-win-core-interlocked-l1-1-0.dll",
    "api-ms-win-core-libraryloader-l1-1-0.dll",
    "api-ms-win-core-localization-l1-2-0.dll",
    "api-ms-win-core-memory-l1-1-0.dll",
    "api-ms-win-core-namedpipe-l1-1-0.dll",
    "api-ms-win-core-processenvironment-l1-1-0.dll",
    "api-ms-win-core-processthreads-l1-1-0.dll",
    "api-ms-win-core-processthreads-l1-1-1.dll",
    "api-ms-win-core-profile-l1-1-0.dll",
    "api-ms-win-core-rtlsupport-l1-1-0.dll",
    "api-ms-win-core-string-l1-1-0.dll",
    "api-ms-win-core-synch-l1-1-0.dll",
    "api-ms-win-core-synch-l1-2-0.dll",
    "api-ms-win-core-sysinfo-l1-1-0.dll",
    "api-ms-win-core-timezone-l1-1-0.dll",
    "api-ms-win-core-util-l1-1-0.dll",
    "api-ms-win-crt-conio-l1-1-0.dll",
    "api-ms-win-crt-convert-l1-1-0.dll",
    "api-ms-win-crt-environment-l1-1-0.dll",
    "api-ms-win-crt-filesystem-l1-1-0.dll",
    "api-ms-win-crt-heap-l1-1-0.dll",
    "api-ms-win-crt-locale-l1-1-0.dll",
    "api-ms-win-crt-math-l1-1-0.dll",
    "api-ms-win-crt-multibyte-l1-1-0.dll",
    "api-ms-win-crt-private-l1-1-0.dll",
    "api-ms-win-crt-process-l1-1-0.dll",
    "api-ms-win-crt-runtime-l1-1-0.dll",
    "api-ms-win-crt-stdio-l1-1-0.dll",
    "api-ms-win-crt-string-l1-1-0.dll",
    "api-ms-win-crt-time-l1-1-0.dll",
    "api-ms-win-crt-utility-l1-1-0.dll",
    "bearer/qgenericbearer.dll",
    "bearer/qnativewifibearer.dll",
    "cares.dll",
    "concrt140.dll",
    "iconengines/qsvgicon.dll",
    "imageformats/qdds.dll",
    "imageformats/qgif.dll",
    "imageformats/qicns.dll",
    "imageformats/qico.dll",
    "imageformats/qjpeg.dll",
    "imageformats/qsvg.dll",
    "imageformats/qtga.dll",
    "imageformats/qtiff.dll",
    "imageformats/qwbmp.dll",
    "imageformats/qwebp.dll",
    "libcurl.dll",
    "libeay32.dll",
    "libsodium.dll",
    "MEGAsync.exe",
    "MEGAupdater.exe",
    "msvcp140.dll",
    "platforms/qwindows.dll",
    "qt.conf",
    "Qt5Concurrent.dll",
    "Qt5Core.dll",
    "Qt5Gui.dll",
    "Qt5Network.dll",
    "Qt5Svg.dll",
    "Qt5Widgets.dll",
    "Qt5Xml.dll",
    "ShellExtX32.dll",
    "ShellExtX64.dll",
    "ssleay32.dll",
    "ucrtbase.dll",
    "uninst.exe",
    "vccorlib140.dll",
    "vcruntime140.dll",
    "avcodec-57.dll",
    "avformat-57.dll",
    "avutil-55.dll",
    "swscale-4.dll",
    "swresample-2.dll"
};

const char *UPDATE_FILES_WIN[] = {
    "api-ms-win-core-console-l1-1-0.dll",
    "api-ms-win-core-datetime-l1-1-0.dll",
    "api-ms-win-core-debug-l1-1-0.dll",
    "api-ms-win-core-errorhandling-l1-1-0.dll",
    "api-ms-win-core-file-l1-1-0.dll",
    "api-ms-win-core-file-l1-2-0.dll",
    "api-ms-win-core-file-l2-1-0.dll",
    "api-ms-win-core-handle-l1-1-0.dll",
    "api-ms-win-core-heap-l1-1-0.dll",
    "api-ms-win-core-interlocked-l1-1-0.dll",
    "api-ms-win-core-libraryloader-l1-1-0.dll",
    "api-ms-win-core-localization-l1-2-0.dll",
    "api-ms-win-core-memory-l1-1-0.dll",
    "api-ms-win-core-namedpipe-l1-1-0.dll",
    "api-ms-win-core-processenvironment-l1-1-0.dll",
    "api-ms-win-core-processthreads-l1-1-0.dll",
    "api-ms-win-core-processthreads-l1-1-1.dll",
    "api-ms-win-core-profile-l1-1-0.dll",
    "api-ms-win-core-rtlsupport-l1-1-0.dll",
    "api-ms-win-core-string-l1-1-0.dll",
    "api-ms-win-core-synch-l1-1-0.dll",
    "api-ms-win-core-synch-l1-2-0.dll",
    "api-ms-win-core-sysinfo-l1-1-0.dll",
    "api-ms-win-core-timezone-l1-1-0.dll",
    "api-ms-win-core-util-l1-1-0.dll",
    "api-ms-win-crt-conio-l1-1-0.dll",
    "api-ms-win-crt-convert-l1-1-0.dll",
    "api-ms-win-crt-environment-l1-1-0.dll",
    "api-ms-win-crt-filesystem-l1-1-0.dll",
    "api-ms-win-crt-heap-l1-1-0.dll",
    "api-ms-win-crt-locale-l1-1-0.dll",
    "api-ms-win-crt-math-l1-1-0.dll",
    "api-ms-win-crt-multibyte-l1-1-0.dll",
    "api-ms-win-crt-private-l1-1-0.dll",
    "api-ms-win-crt-process-l1-1-0.dll",
    "api-ms-win-crt-runtime-l1-1-0.dll",
    "api-ms-win-crt-stdio-l1-1-0.dll",
    "api-ms-win-crt-string-l1-1-0.dll",
    "api-ms-win-crt-time-l1-1-0.dll",
    "api-ms-win-crt-utility-l1-1-0.dll",
    "bearer/qgenericbearer.dll",
    "bearer/qnativewifibearer.dll",
    "cares.dll",
    "concrt140.dll",
    "iconengines/qsvgicon.dll",
    "imageformats/qdds.dll",
    "imageformats/qgif.dll",
    "imageformats/qicns.dll",
    "imageformats/qico.dll",
    "imageformats/qjpeg.dll",
    "imageformats/qsvg.dll",
    "imageformats/qtga.dll",
    "imageformats/qtiff.dll",
    "imageformats/qwbmp.dll",
    "imageformats/qwebp.dll",
    "libcurl.dll",
    "libeay32.dll",
    "libsodium.dll",
    "MEGAsync.exe",
    "MEGAupdater.exe",
    "msvcp140.dll",
    "platforms/qwindows.dll",
    "qt.conf",
    "Qt5Concurrent.dll",
    "Qt5Core.dll",
    "Qt5Gui.dll",
    "Qt5Network.dll",
    "Qt5Svg.dll",
    "Qt5Widgets.dll",
    "Qt5Xml.dll",
    "ShellExtX32.dll",
    "ShellExtX64.dll",
    "ssleay32.dll",
    "ucrtbase.dll",
    "uninst.exe",
    "vccorlib140.dll",
    "vcruntime140.dll",
    "avcodec-57.dll",
    "avformat-57.dll",
    "avutil-55.dll",
    "swscale-4.dll",
    "swresample-2.dll"
};

const char *TARGET_PATHS_OSX[] = {
    "Contents/_CodeSignature/CodeResources",
    "Contents/Frameworks/libavcodec.57.dylib",
    "Contents/Frameworks/libavformat.57.dylib",
    "Contents/Frameworks/libavutil.55.dylib",
    "Contents/Frameworks/libswscale.4.dylib",
    "Contents/Frameworks/QtCore.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtCore.framework/Versions/5/QtCore",
    "Contents/Frameworks/QtCore.framework/Versions/5/Resources/Info.plist",
    "Contents/Frameworks/QtGui.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtGui.framework/Versions/5/QtGui",
    "Contents/Frameworks/QtGui.framework/Versions/5/Resources/Info.plist",
    "Contents/Frameworks/QtMacExtras.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtMacExtras.framework/Versions/5/QtMacExtras",
    "Contents/Frameworks/QtMacExtras.framework/Versions/5/Resources/Info.plist",
    "Contents/Frameworks/QtNetwork.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork",
    "Contents/Frameworks/QtNetwork.framework/Versions/5/Resources/Info.plist",
    "Contents/Frameworks/QtPrintSupport.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport",
    "Contents/Frameworks/QtPrintSupport.framework/Versions/5/Resources/Info.plist",
    "Contents/Frameworks/QtWidgets.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets",
    "Contents/Frameworks/QtWidgets.framework/Versions/5/Resources/Info.plist",
    "Contents/Info.plist",
    "Contents/MacOS/MEGAclient",
    "Contents/MacOS/MEGAupdater",
    "Contents/PkgInfo",
    "Contents/PlugIns/bearer/libqcorewlanbearer.dylib",
    "Contents/PlugIns/bearer/libqgenericbearer.dylib",
    "Contents/PlugIns/imageformats/libqdds.dylib",
    "Contents/PlugIns/imageformats/libqgif.dylib",
    "Contents/PlugIns/imageformats/libqicns.dylib",
    "Contents/PlugIns/imageformats/libqico.dylib",
    "Contents/PlugIns/imageformats/libqjp2.dylib",
    "Contents/PlugIns/imageformats/libqjpeg.dylib",
    "Contents/PlugIns/imageformats/libqmng.dylib",
    "Contents/PlugIns/imageformats/libqtga.dylib",
    "Contents/PlugIns/imageformats/libqtiff.dylib",
    "Contents/PlugIns/imageformats/libqwbmp.dylib",
    "Contents/PlugIns/imageformats/libqwebp.dylib",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/_CodeSignature/CodeResources",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Info.plist",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/MacOS/MEGAShellExtFinder",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ar.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/Assets.car",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/Base.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/bg.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/context-ico.icns",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/cs.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/de.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/error.icns",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/es.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/fi.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/fr.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/he.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/hu.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/id.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/it.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ja.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ko.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/nl.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ok.icns",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/pl.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/pt-BR.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/pt-PT.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ro.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ru.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/sk.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/sl.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/sr.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/sv.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/sync.icns",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/th.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/tr.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/uk.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/vi.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/zh-Hans.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/zh-Hant.lproj/Localizable.strings",
    "Contents/PlugIns/platforms/libqcocoa.dylib",
    "Contents/PlugIns/printsupport/libcocoaprintersupport.dylib",
    "Contents/Resources/app.icns",
    "Contents/Resources/appicon32.tiff",
    "Contents/Resources/empty.lproj",
    "Contents/Resources/folder.icns",
    "Contents/Resources/folder_yosemite.icns",
    "Contents/Resources/qt.conf"
};

const char *UPDATE_FILES_OSX[] = {
    "Contents/_CodeSignature/CodeResources",
    "Contents/Frameworks/libavcodec.57.dylib",
    "Contents/Frameworks/libavformat.57.dylib",
    "Contents/Frameworks/libavutil.55.dylib",
    "Contents/Frameworks/libswscale.4.dylib",
    "Contents/Frameworks/QtCore.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtCore.framework/Versions/5/QtCore",
    "Contents/Frameworks/QtCore.framework/Versions/5/Resources/Info.plist",
    "Contents/Frameworks/QtGui.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtGui.framework/Versions/5/QtGui",
    "Contents/Frameworks/QtGui.framework/Versions/5/Resources/Info.plist",
    "Contents/Frameworks/QtMacExtras.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtMacExtras.framework/Versions/5/QtMacExtras",
    "Contents/Frameworks/QtMacExtras.framework/Versions/5/Resources/Info.plist",
    "Contents/Frameworks/QtNetwork.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork",
    "Contents/Frameworks/QtNetwork.framework/Versions/5/Resources/Info.plist",
    "Contents/Frameworks/QtPrintSupport.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport",
    "Contents/Frameworks/QtPrintSupport.framework/Versions/5/Resources/Info.plist",
    "Contents/Frameworks/QtWidgets.framework/Versions/5/_CodeSignature/CodeResources",
    "Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets",
    "Contents/Frameworks/QtWidgets.framework/Versions/5/Resources/Info.plist",
    "Contents/Info.plist",
    "Contents/MacOS/MEGAclient",
    "Contents/MacOS/MEGAupdater",
    "Contents/PkgInfo",
    "Contents/PlugIns/bearer/libqcorewlanbearer.dylib",
    "Contents/PlugIns/bearer/libqgenericbearer.dylib",
    "Contents/PlugIns/imageformats/libqdds.dylib",
    "Contents/PlugIns/imageformats/libqgif.dylib",
    "Contents/PlugIns/imageformats/libqicns.dylib",
    "Contents/PlugIns/imageformats/libqico.dylib",
    "Contents/PlugIns/imageformats/libqjp2.dylib",
    "Contents/PlugIns/imageformats/libqjpeg.dylib",
    "Contents/PlugIns/imageformats/libqmng.dylib",
    "Contents/PlugIns/imageformats/libqtga.dylib",
    "Contents/PlugIns/imageformats/libqtiff.dylib",
    "Contents/PlugIns/imageformats/libqwbmp.dylib",
    "Contents/PlugIns/imageformats/libqwebp.dylib",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/_CodeSignature/CodeResources",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Info.plist",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/MacOS/MEGAShellExtFinder",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ar.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/Assets.car",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/Base.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/bg.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/context-ico.icns",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/cs.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/de.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/error.icns",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/es.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/fi.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/fr.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/he.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/hu.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/id.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/it.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ja.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ko.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/nl.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ok.icns",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/pl.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/pt-BR.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/pt-PT.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ro.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/ru.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/sk.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/sl.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/sr.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/sv.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/sync.icns",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/th.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/tr.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/uk.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/vi.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/zh-Hans.lproj/Localizable.strings",
    "Contents/PlugIns/MEGAShellExtFinder.appex/Contents/Resources/zh-Hant.lproj/Localizable.strings",
    "Contents/PlugIns/platforms/libqcocoa.dylib",
    "Contents/PlugIns/printsupport/libcocoaprintersupport.dylib",
    "Contents/Resources/app.icns",
    "Contents/Resources/appicon32.tiff",
    "Contents/Resources/empty.lproj",
    "Contents/Resources/folder.icns",
    "Contents/Resources/folder_yosemite.icns",
    "Contents/Resources/qt.conf"
};

template <typename T, std::size_t N>
char (&static_sizeof_array( T(&)[N] ))[N];
#define SIZEOF_ARRAY( x ) sizeof(static_sizeof_array(x))

void printUsage(const char* appname)
{
    cerr << "Usage: " << endl;
    cerr << "Generate a keypair" << endl;
    cerr << "    " << appname << " -g" << endl;
    cerr << "Sign an update:" << endl;
    cerr << "    " << appname << " -s <win|osx> <update folder> <keyfile> <version_code>" << endl;
    cerr << "  or:" << endl;
    cerr << "    " << appname << " <update folder> <keyfile> --file <contentsfile> --base-url <base_url>" << endl;
    cerr << "    e.g:" << endl;
    cerr << "        " << appname << " /tmp/updatefiles /tmp/key.pem --file /tmp/files.txt --base-url http://g.static.mega.co.nz/upd/wsync/" << endl;
}

unsigned signFile(const char * filePath, AsymmCipher* key, byte* signature, unsigned signbuflen)
{
    HashSignature signatureGenerator(new Hash());
    char buffer[1024];

    ifstream input(filePath, std::ios::in | std::ios::binary);
    if (input.fail())
    {
        return 0;
    }

    while (input.good())
    {
        input.read(buffer, sizeof(buffer));
        signatureGenerator.add((byte *)buffer, (unsigned)input.gcount());
    }

    if (input.bad())
    {
        return 0;
    }

    unsigned signatureSize = signatureGenerator.get(key, signature, signbuflen);
    if (signatureSize < signbuflen)
    {
        int padding = signbuflen - signatureSize;
        for (int i = signbuflen - 1; i >= 0; i--)
        {
            if (i >= padding)
            {
                signature[i] = signature[i - padding];
            }
            else
            {
                signature[i] = 0;
            }
        }
        signatureSize = signbuflen;
    }
    return signatureSize;
}


bool generateHash(const char * filePath, string *hash)
{
    HashSHA256 hashGenerator;
    char buffer[1024];

    ifstream input(filePath, std::ios::in | std::ios::binary);
    if (input.fail())
    {
        return false;
    }

    while (input.good())
    {
        input.read(buffer, sizeof(buffer));
        hashGenerator.add((byte *)buffer, (unsigned)input.gcount());
    }

    if (input.bad())
    {
        return false;
    }

    string binaryhash;
    hashGenerator.get(&binaryhash);

    static const char hexchars[] = "0123456789abcdef";
    ostringstream oss;
    for (size_t i=0;i<binaryhash.size();++i)
    {
        oss.put(hexchars[(binaryhash[i] >> 4) & 0x0F]);
        oss.put(hexchars[binaryhash[i] & 0x0F]);
    }
    *hash = oss.str();

    return true;
}

bool extractarg(vector<const char*>& args, const char *what)
{
    for (int i = int(args.size()); i--; )
    {
        if (!strcmp(args[i], what))
        {
            args.erase(args.begin() + i);
            return true;
        }
    }
    return false;
}

bool extractargparam(vector<const char*>& args, const char *what, std::string& param)
{
    for (int i = int(args.size()) - 1; --i >= 0; )
    {
        if (!strcmp(args[i], what) && args.size() > i)
        {
            param = args[i + 1];
            args.erase(args.begin() + i, args.begin() + i + 2);
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[])
{
    vector<const char*> args(argv + 1, argv + argc);

    string fileInput;
    bool externalfile = extractargparam(args, "--file", fileInput);
    bool generate = extractarg(args, "-g");
    string os;
    bool bos = extractargparam(args, "-s", os);
    string baseUrl;
    bool bUrl = extractargparam(args, "--base-url", baseUrl);


    HashSignature signatureGenerator(new Hash());
    AsymmCipher aprivk;
    vector<string> downloadURLs;
    vector<string> signatures;
    byte signature[SIGNATURE_LENGTH];
    unsigned signatureSize;
    string pubk;
    string privk;
    bool win = true;

    if (generate)
    {
        //Generate a keypair
        CryptoPP::Integer pubk[AsymmCipher::PUBKEY];
        string pubks;
        string privks;

        AsymmCipher asymkey;
        asymkey.genkeypair(asymkey.key,pubk,KEY_LENGTH);
        AsymmCipher::serializeintarray(pubk,AsymmCipher::PUBKEY,&pubks);
        AsymmCipher::serializeintarray(asymkey.key,AsymmCipher::PRIVKEY,&privks);

        int len = pubks.size();
        char* pubkstr = new char[len*4/3+4];
        Base64::btoa((const byte *)pubks.data(),len,pubkstr);

        len = privks.size();
        char *privkstr = new char[len*4/3+4];
        Base64::btoa((const byte *)privks.data(),len,privkstr);

        cout << pubkstr << endl;
        cout << privkstr << endl;

        delete [] pubkstr;
        delete [] privkstr;
        return 0;
    }
    else if (((args.size() == 3)  && ((bos && (os == "win" || os == "osx")))
              || (args.size() == 2 && bUrl && externalfile)))
    {
        //Sign an update
        win = os == "win";

        if (!bUrl)
        {
            baseUrl = string ((win ? SERVER_BASE_URL_WIN : SERVER_BASE_URL_OSX));
        }

        //Prepare the update folder path
        string updateFolder(args.at(0));
        if (updateFolder[updateFolder.size()-1] != '/')
        {
            updateFolder.append("/");
        }

        //Read keys
        ifstream keyFile(args.at(1), std::ios::in);
        if (keyFile.bad())
        {
            printUsage(argv[0]);
            return 2;
        }
        getline(keyFile, pubk);
        getline(keyFile, privk);
        if (!pubk.size() || !privk.size())
        {
            cerr << "Invalid key file" << endl;
            keyFile.close();
            return 3;
        }
        keyFile.close();

        //Initialize AsymmCypher
        string privks;
        privks.resize(privk.size()/4*3+3);
        privks.resize(Base64::atob(privk.data(), (byte *)privks.data(), privks.size()));
        aprivk.setkey(AsymmCipher::PRIVKEY,(byte*)privks.data(), privks.size());

        //Generate update file signature
        vector<string> filesVector;
        vector<string> targetPathsVector;
        vector<string> hashesVector;
        string sversioncode;

        if (externalfile)
        {
            filesVector.clear();
            targetPathsVector.clear();
            ifstream infile(fileInput.c_str());
            string line;
            while (getline(infile, line))
            {
                if (line.length() > 0 && line[0] != '#')
                {
                    string fileToDl, targetpah;
                    size_t pos = line.find(";");
                    fileToDl = line.substr(0, pos);
                    if (pos != string::npos && ((pos + 1) < line.size()))
                    {
                        string rest = line.substr(pos + 1);
                        pos = rest.find(";");
                        targetpah = rest.substr(0, pos);
                        if (pos != string::npos && ((pos + 1) < rest.size()))
                        {
                            hashesVector.push_back(rest.substr(pos + 1));
                        }
                        else
                        {
                            hashesVector.push_back("UNKNOWN");
                        }
                    }
                    else
                    {
                        targetpah = fileToDl;
                    }
                    filesVector.push_back(fileToDl.c_str());
                    targetPathsVector.push_back(targetpah.c_str());
                }
                else
                {
                    if (line.find("#version=") == 0)
                    {
                        sversioncode=line.substr(9);
                    }
                }
            }
        }
        else
        {
            unsigned int numFiles;
            if (win)
            {
                numFiles = SIZEOF_ARRAY(UPDATE_FILES_WIN);
            }
            else
            {
                numFiles = SIZEOF_ARRAY(UPDATE_FILES_OSX);
            }
            for (unsigned int i = 0; i < numFiles; i++)
            {
                filesVector.push_back( (win ? UPDATE_FILES_WIN : UPDATE_FILES_OSX)[i]);
                targetPathsVector.push_back( (win ? TARGET_PATHS_WIN : TARGET_PATHS_OSX)[i]);
                hashesVector.push_back("UNKNOWN");
            }
            sversioncode=args.at(2);
        }

        long versionCode;
        versionCode = strtol (sversioncode.c_str(), NULL, 10);
        if (!versionCode)
        {
            cerr << "Invalid version code" << endl;
            return 5;
        }

        signatureGenerator.add((const byte *)sversioncode.c_str(), strlen(sversioncode.c_str()));

        for (unsigned int i = 0; i < filesVector.size(); i++)
        {
            string filePath = updateFolder + filesVector.at(i);

            if (hashesVector.at(i) != "UNKNOWN")
            {
                string hashFile;
                generateHash(filePath.c_str(),&hashFile);

                if (hashFile != hashesVector.at(i))
                {
                    cerr << "Error checking hash for file: " << filePath << endl
                          << " calculated=" << hashFile << endl
                          << "   expected=" << hashesVector.at(i) << endl;
                    return 6;
                }
            }


            signatureSize = signFile(filePath.data(), &aprivk, signature, sizeof(signature));
            if (!signatureSize)
            {
                cerr << "Error signing file: " << filePath << endl;
                return 4;
            }

            string s;
            s.resize((signatureSize*4)/3+4);
            s.resize(Base64::btoa((byte *)signature, signatureSize, (char *)s.data()));
            signatures.push_back(s);

            string fileurl = baseUrl + filesVector.at(i);
            downloadURLs.push_back(fileurl);

            signatureGenerator.add((const byte*)fileurl.data(), fileurl.size());
            signatureGenerator.add((const byte*)targetPathsVector.at(i).data(),
                                   targetPathsVector.at(i).size());
            signatureGenerator.add((const byte*)s.data(), s.length());
        }

        signatureSize = signatureGenerator.get(&aprivk, signature, sizeof(signature));
        if (!signatureSize)
        {
            cerr << "Error signing the update file" << endl;
            return 6;
        }

        if (signatureSize < sizeof(signature))
        {
            int padding = sizeof(signature) - signatureSize;
            for (int i = sizeof(signature) - 1; i >= 0; i--)
            {
                if (i >= padding)
                {
                    signature[i] = signature[i - padding];
                }
                else
                {
                    signature[i] = 0;
                }
            }
            signatureSize = sizeof(signature);
        }

        string updateFileSignature;
        updateFileSignature.resize((signatureSize*4)/3+4);
        updateFileSignature.resize(Base64::btoa((byte *)signature, signatureSize, (char *)updateFileSignature.data()));

        //Print update file
        cout << versionCode << endl;
        cout << updateFileSignature << endl;
        for (unsigned int i = 0; i < targetPathsVector.size(); i++)
        {
            cout << downloadURLs[i] << endl;
            cout << targetPathsVector.at(i) << endl;
            cout << signatures[i] << endl;
        }

        return 0;
    }

    printUsage(argv[0]);
    return 1;
}
