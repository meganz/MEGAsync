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
    "cares.dll",
    "concrt140.dll",
    "libcurl.dll",
    "libeay32.dll",
    "libsodium.dll",
    "libuv.dll",
    "MEGAsync.exe",
    "msvcp140.dll",
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
    "bearer/qgenericbearer.dll",
    "bearer/qnativewifibearer.dll",
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
    "platforms/qwindows.dll"
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
    "cares.dll",
    "concrt140.dll",
    "libcurl.dll",
    "libeay32.dll",
    "libsodium.dll",
    "libuv.dll",
    "MEGAsync.exe",
    "msvcp140.dll",
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
    "bearer/qgenericbearer.dll",
    "bearer/qnativewifibearer.dll",
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
    "platforms/qwindows.dll"
};

const char *TARGET_PATHS_OSX[] = {
    "Contents/_CodeSignature/CodeResources",
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
    cerr << "Sign an update:" << endl;
    cerr << "    " << appname << " -s <win|osx> <update folder> <keyfile> <version_code>" << endl;
    cerr << "Generate a keypair" << endl;
    cerr << "    " << appname << " -g" << endl;
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
        cerr << "Invalid signature size: " << signatureSize;
        return 0;
    }
    return signatureSize;
}

int main(int argc, char *argv[])
{
    HashSignature signatureGenerator(new Hash());
    AsymmCipher aprivk;
    vector<string> downloadURLs;
    vector<string> signatures;
    byte signature[SIGNATURE_LENGTH];
    unsigned signatureSize;
    string pubk;
    string privk;
    bool win = true;

    if ((argc == 2) && !strcmp(argv[1], "-g"))
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

        delete pubkstr;
        delete privkstr;
        return 0;
    }
    else if ((argc == 6) && !strcmp(argv[1], "-s")
             && (!strcmp(argv[2], "win") || !strcmp(argv[2], "osx")))
    {
        //Sign an update
        win = !strcmp(argv[2], "win");

        //Prepare the update folder path
        string updateFolder(argv[3]);
        if (updateFolder[updateFolder.size()-1] != '/')
        {
            updateFolder.append("/");
        }

        //Read keys
        ifstream keyFile(argv[4], std::ios::in);
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

        long versionCode = strtol (argv[5], NULL, 10);
        if (!versionCode)
        {
            cerr << "Invalid version code" << endl;
            return 5;
        }

        //Initialize AsymmCypher
        string privks;
        privks.resize(privk.size()/4*3+3);
        privks.resize(Base64::atob(privk.data(), (byte *)privks.data(), privks.size()));
        aprivk.setkey(AsymmCipher::PRIVKEY,(byte*)privks.data(), privks.size());

        //Generate update file signature
        signatureGenerator.add((const byte *)argv[5], strlen(argv[5]));

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
            string filePath = updateFolder + (win ? UPDATE_FILES_WIN : UPDATE_FILES_OSX)[i];
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

            string fileUrl((win ? SERVER_BASE_URL_WIN : SERVER_BASE_URL_OSX));
            fileUrl.append((win ? UPDATE_FILES_WIN : UPDATE_FILES_OSX)[i]);
            downloadURLs.push_back(fileUrl);

            signatureGenerator.add((const byte*)fileUrl.data(), fileUrl.size());
            signatureGenerator.add((const byte*)(win ? TARGET_PATHS_WIN : TARGET_PATHS_OSX)[i],
                                   strlen((win ? TARGET_PATHS_WIN : TARGET_PATHS_OSX)[i]));
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
            cerr << "Error signing the update file: invalid signature size: " << signatureSize << endl;
            return 7;
        }

        string updateFileSignature;
        updateFileSignature.resize((signatureSize*4)/3+4);
        updateFileSignature.resize(Base64::btoa((byte *)signature, signatureSize, (char *)updateFileSignature.data()));

        //Print update file
        cout << argv[5] << endl;
        cout << updateFileSignature << endl;
        for (unsigned int i = 0; i < numFiles; i++)
        {
            cout << downloadURLs[i] << endl;
            cout << (win ? TARGET_PATHS_WIN : TARGET_PATHS_OSX)[i] << endl;
            cout << signatures[i] << endl;
        }

        return 0;
    }

    printUsage(argv[0]);
    return 1;
}
