#include "Utilities.h"
#include "control/Preferences.h"

#include <QApplication>
#include <QImageReader>
#include <QDirIterator>
#include <QDesktopServices>
#include <QTextStream>
#include <QDateTime>
#include <iostream>
#include <QDesktopWidget>
#include "MegaApplication.h"
#include "control/gzjoin.h"
#include "platform/Platform.h"

#ifndef WIN32
#include "megaapi.h"
#include <utime.h>
#else
#include <windows.h>
#endif

using namespace std;
using namespace mega;

QHash<QString, QString> Utilities::extensionIcons;
QHash<QString, Utilities::FileType> Utilities::fileTypes;
QHash<QString, QString> Utilities::languageNames;

std::unique_ptr<ThreadPool> ThreadPoolSingleton::instance = nullptr;

const unsigned long long KB = 1024;
const unsigned long long MB = 1024 * KB;
const unsigned long long GB = 1024 * MB;
const unsigned long long TB = 1024 * GB;

void Utilities::initializeExtensions()
{
    extensionIcons[QString::fromAscii("3ds")] = extensionIcons[QString::fromAscii("3dm")]  = extensionIcons[QString::fromAscii("max")] =
                            extensionIcons[QString::fromAscii("obj")]  = QString::fromAscii("3D.png");

    extensionIcons[QString::fromAscii("aep")] = extensionIcons[QString::fromAscii("aet")]  = QString::fromAscii("aftereffects.png");

    extensionIcons[QString::fromAscii("mp3")] = extensionIcons[QString::fromAscii("wav")]  = extensionIcons[QString::fromAscii("3ga")]  =
                            extensionIcons[QString::fromAscii("aif")]  = extensionIcons[QString::fromAscii("aiff")] =
                            extensionIcons[QString::fromAscii("flac")] = extensionIcons[QString::fromAscii("iff")]  =
                            extensionIcons[QString::fromAscii("m4a")]  = extensionIcons[QString::fromAscii("wma")]  =  QString::fromAscii("audio.png");

    extensionIcons[QString::fromAscii("dxf")] = extensionIcons[QString::fromAscii("dwg")] =  QString::fromAscii("cad.png");


    extensionIcons[QString::fromAscii("zip")] = extensionIcons[QString::fromAscii("rar")] = extensionIcons[QString::fromAscii("tgz")]  =
                            extensionIcons[QString::fromAscii("gz")]  = extensionIcons[QString::fromAscii("bz2")]  =
                            extensionIcons[QString::fromAscii("tbz")] = extensionIcons[QString::fromAscii("tar")]  =
                            extensionIcons[QString::fromAscii("7z")]  = extensionIcons[QString::fromAscii("sitx")] =  QString::fromAscii("compressed.png");

    extensionIcons[QString::fromAscii("sql")] = extensionIcons[QString::fromAscii("accdb")] = extensionIcons[QString::fromAscii("db")]  =
                            extensionIcons[QString::fromAscii("dbf")]  = extensionIcons[QString::fromAscii("mdb")]  =
                            extensionIcons[QString::fromAscii("pdb")] = QString::fromAscii("web_lang.png");

    extensionIcons[QString::fromAscii("folder")] = QString::fromAscii("folder.png");

    extensionIcons[QString::fromAscii("xls")] = extensionIcons[QString::fromAscii("xlsx")] = extensionIcons[QString::fromAscii("xlt")]  =
                            extensionIcons[QString::fromAscii("xltm")]  = QString::fromAscii("excel.png");

    extensionIcons[QString::fromAscii("exe")] = extensionIcons[QString::fromAscii("com")] = extensionIcons[QString::fromAscii("bin")]  =
                            extensionIcons[QString::fromAscii("apk")]  = extensionIcons[QString::fromAscii("app")]  =
                             extensionIcons[QString::fromAscii("msi")]  = extensionIcons[QString::fromAscii("cmd")]  =
                            extensionIcons[QString::fromAscii("gadget")] = QString::fromAscii("executable.png");

    extensionIcons[QString::fromAscii("fnt")] = extensionIcons[QString::fromAscii("otf")] = extensionIcons[QString::fromAscii("ttf")]  =
                            extensionIcons[QString::fromAscii("fon")]  = QString::fromAscii("font.png");

    extensionIcons[QString::fromAscii("gif")] = extensionIcons[QString::fromAscii("tiff")]  = extensionIcons[QString::fromAscii("tif")]  =
                            extensionIcons[QString::fromAscii("bmp")]  = extensionIcons[QString::fromAscii("png")] =
                            extensionIcons[QString::fromAscii("tga")]  = QString::fromAscii("image.png");

    extensionIcons[QString::fromAscii("ai")] = extensionIcons[QString::fromAscii("ait")] = QString::fromAscii("illustrator.png");
    extensionIcons[QString::fromAscii("jpg")] = extensionIcons[QString::fromAscii("jpeg")] = QString::fromAscii("image.png");
    extensionIcons[QString::fromAscii("indd")] = QString::fromAscii("indesign.png");

    extensionIcons[QString::fromAscii("jar")] = extensionIcons[QString::fromAscii("java")]  = extensionIcons[QString::fromAscii("class")]  = QString::fromAscii("web_data.png");

    extensionIcons[QString::fromAscii("pdf")] = QString::fromAscii("pdf.png");
    extensionIcons[QString::fromAscii("abr")] = extensionIcons[QString::fromAscii("psb")]  = extensionIcons[QString::fromAscii("psd")]  =
                            QString::fromAscii("photoshop.png");

    extensionIcons[QString::fromAscii("pps")] = extensionIcons[QString::fromAscii("ppt")]  = extensionIcons[QString::fromAscii("pptx")] = QString::fromAscii("powerpoint.png");

    extensionIcons[QString::fromAscii("prproj")] = extensionIcons[QString::fromAscii("ppj")]  = QString::fromAscii("premiere.png");

    extensionIcons[QString::fromAscii("3fr")] = extensionIcons[QString::fromAscii("arw")]  = extensionIcons[QString::fromAscii("bay")]  =
                            extensionIcons[QString::fromAscii("cr2")]  = extensionIcons[QString::fromAscii("dcr")] =
                            extensionIcons[QString::fromAscii("dng")] = extensionIcons[QString::fromAscii("fff")]  =
                            extensionIcons[QString::fromAscii("mef")] = extensionIcons[QString::fromAscii("mrw")]  =
                            extensionIcons[QString::fromAscii("nef")] = extensionIcons[QString::fromAscii("pef")]  =
                            extensionIcons[QString::fromAscii("rw2")] = extensionIcons[QString::fromAscii("srf")]  =
                            extensionIcons[QString::fromAscii("orf")] = extensionIcons[QString::fromAscii("rwl")]  =
                            QString::fromAscii("raw.png");

    extensionIcons[QString::fromAscii("ods")]  = extensionIcons[QString::fromAscii("ots")]  =
                            extensionIcons[QString::fromAscii("gsheet")]  = extensionIcons[QString::fromAscii("nb")] =
                            extensionIcons[QString::fromAscii("xlr")] = QString::fromAscii("spreadsheet.png");

    extensionIcons[QString::fromAscii("torrent")] = QString::fromAscii("torrent.png");
    extensionIcons[QString::fromAscii("dmg")] = QString::fromAscii("dmg.png");

    extensionIcons[QString::fromAscii("txt")] = extensionIcons[QString::fromAscii("rtf")]  = extensionIcons[QString::fromAscii("ans")]  =
                            extensionIcons[QString::fromAscii("ascii")]  = extensionIcons[QString::fromAscii("log")] =
                            extensionIcons[QString::fromAscii("odt")] = extensionIcons[QString::fromAscii("wpd")]  =
                            QString::fromAscii("text.png");

    extensionIcons[QString::fromAscii("svgz")]  = extensionIcons[QString::fromAscii("svg")]  =
                            extensionIcons[QString::fromAscii("cdr")]  = extensionIcons[QString::fromAscii("eps")] =
                            QString::fromAscii("vector.png");

     extensionIcons[QString::fromAscii("mkv")]  = extensionIcons[QString::fromAscii("webm")]  =
                            extensionIcons[QString::fromAscii("avi")]  = extensionIcons[QString::fromAscii("mp4")] =
                            extensionIcons[QString::fromAscii("m4v")] = extensionIcons[QString::fromAscii("mpg")]  =
                            extensionIcons[QString::fromAscii("mpeg")] = extensionIcons[QString::fromAscii("mov")]  =
                            extensionIcons[QString::fromAscii("3g2")] = extensionIcons[QString::fromAscii("3gp")]  =
                            extensionIcons[QString::fromAscii("asf")] = extensionIcons[QString::fromAscii("wmv")]  =
                            extensionIcons[QString::fromAscii("flv")] = extensionIcons[QString::fromAscii("vob")] =
                            QString::fromAscii("video.png");

     extensionIcons[QString::fromAscii("html")]  = extensionIcons[QString::fromAscii("xml")] = extensionIcons[QString::fromAscii("shtml")]  =
                            extensionIcons[QString::fromAscii("dhtml")] = extensionIcons[QString::fromAscii("js")] =
                            extensionIcons[QString::fromAscii("css")]  = QString::fromAscii("web_data.png");

     extensionIcons[QString::fromAscii("php")]  = extensionIcons[QString::fromAscii("php3")]  =
                            extensionIcons[QString::fromAscii("php4")]  = extensionIcons[QString::fromAscii("php5")] =
                            extensionIcons[QString::fromAscii("phtml")] = extensionIcons[QString::fromAscii("inc")]  =
                            extensionIcons[QString::fromAscii("asp")] = extensionIcons[QString::fromAscii("pl")]  =
                            extensionIcons[QString::fromAscii("cgi")] = extensionIcons[QString::fromAscii("py")]  =
                            QString::fromAscii("web_lang.png");

     extensionIcons[QString::fromAscii("doc")]  = extensionIcons[QString::fromAscii("docx")] = extensionIcons[QString::fromAscii("dotx")]  =
                            extensionIcons[QString::fromAscii("wps")] = QString::fromAscii("word.png");

     extensionIcons[QString::fromAscii("odt")]  = extensionIcons[QString::fromAscii("ods")] = extensionIcons[QString::fromAscii("odp")]  =
                            extensionIcons[QString::fromAscii("odb")] = extensionIcons[QString::fromAscii("odg")] = QString::fromAscii("openoffice.png");

     extensionIcons[QString::fromAscii("sketch")] = QString::fromAscii("sketch.png");
     extensionIcons[QString::fromAscii("xd")] = QString::fromAscii("experiencedesign.png");
     extensionIcons[QString::fromAscii("pages")] = QString::fromAscii("pages.png");
     extensionIcons[QString::fromAscii("numbers")] = QString::fromAscii("numbers.png");
     extensionIcons[QString::fromAscii("key")] = QString::fromAscii("keynote.png");
}

void Utilities::initializeFileTypes()
{
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.txt"), QString())]
            = FileType::TYPE_TEXT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.wav"), QString())]
            = FileType::TYPE_AUDIO;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.mkv"), QString())]
            = FileType::TYPE_VIDEO;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.tar"), QString())]
            = FileType::TYPE_ARCHIVE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.odt"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.png"), QString())]
            = FileType::TYPE_IMAGE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.bin"), QString())]
            = FileType::TYPE_OTHER;
}


void Utilities::queueFunctionInAppThread(std::function<void()> fun) {
   QObject temporary;
   QObject::connect(&temporary, &QObject::destroyed, qApp, std::move(fun), Qt::QueuedConnection);
}

void Utilities::getFolderSize(QString folderPath, long long *size)
{
    if (!folderPath.size())
    {
        return;
    }

    QDir dir(folderPath);
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    for (int i = 0; i < entries.size(); i++)
    {
        QFileInfo info = entries[i];
        if (info.isFile())
        {
            (*size) += info.size();
        }
        else if (info.isDir())
        {
            getFolderSize(info.absoluteFilePath(), size);
        }
    }
}

qreal Utilities::getDevicePixelRatio()
{
#if QT_VERSION >= 0x050000
    return qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? qApp->devicePixelRatio() : 1.0;
#else
    return 1.0;
#endif
}

QString Utilities::getExtensionPixmapName(QString fileName, QString prefix)
{
    if (extensionIcons.isEmpty())
    {
        initializeExtensions();
        initializeFileTypes();
    }

    QFileInfo f(fileName);
    if (extensionIcons.contains(f.suffix().toLower()))
    {
        return QString(extensionIcons[f.suffix().toLower()]).insert(0, prefix);
    }
    else
    {
        return prefix + QString::fromAscii("generic.png");
    }
}

Utilities::FileType Utilities::getFileType(QString fileName, QString prefix)
{
    auto extensionPixmapName = getExtensionPixmapName(fileName, prefix);
    return fileTypes.value(extensionPixmapName,FileType::TYPE_OTHER);
}

QString Utilities::languageCodeToString(QString code)
{
    if (languageNames.isEmpty())
    {
        languageNames[QString::fromAscii("ar")] = QString::fromUtf8("العربية"); // arabic
        languageNames[QString::fromAscii("de")] = QString::fromUtf8("Deutsch");
        languageNames[QString::fromAscii("en")] = QString::fromUtf8("English");
        languageNames[QString::fromAscii("es")] = QString::fromUtf8("Español");
        languageNames[QString::fromAscii("fr")] = QString::fromUtf8("Français");
        languageNames[QString::fromAscii("id")] = QString::fromUtf8("Bahasa Indonesia");
        languageNames[QString::fromAscii("it")] = QString::fromUtf8("Italiano");
        languageNames[QString::fromAscii("ja")] = QString::fromUtf8("日本語"); // japanese
        languageNames[QString::fromAscii("ko")] = QString::fromUtf8("한국어"); // korean
        languageNames[QString::fromAscii("nl")] = QString::fromUtf8("Nederlands");
        languageNames[QString::fromAscii("pl")] = QString::fromUtf8("Polski");
        languageNames[QString::fromAscii("pt")] = QString::fromUtf8("Português");
        languageNames[QString::fromAscii("ro")] = QString::fromUtf8("Română");
        languageNames[QString::fromAscii("ru")] = QString::fromUtf8("Pусский");
        languageNames[QString::fromAscii("th")] = QString::fromUtf8("ภาษาไทย"); // thai
        languageNames[QString::fromAscii("vi")] = QString::fromUtf8("Tiếng Việt");
        languageNames[QString::fromAscii("zh_CN")] = QString::fromUtf8("简体中文");
        languageNames[QString::fromAscii("zh_TW")] = QString::fromUtf8("中文繁體");


        // Currently unsupported
        // languageNames[QString::fromAscii("mi")] = QString::fromUtf8("Māori");
        // languageNames[QString::fromAscii("ca")] = QString::fromUtf8("Català");
        // languageNames[QString::fromAscii("eu")] = QString::fromUtf8("Euskara");
        // languageNames[QString::fromAscii("af")] = QString::fromUtf8("Afrikaans");
        // languageNames[QString::fromAscii("no")] = QString::fromUtf8("Norsk");
        // languageNames[QString::fromAscii("bs")] = QString::fromUtf8("Bosanski");
        // languageNames[QString::fromAscii("da")] = QString::fromUtf8("Dansk");
        // languageNames[QString::fromAscii("el")] = QString::fromUtf8("ελληνικά");
        // languageNames[QString::fromAscii("lt")] = QString::fromUtf8("Lietuvos");
        // languageNames[QString::fromAscii("lv")] = QString::fromUtf8("Latviešu");
        // languageNames[QString::fromAscii("mk")] = QString::fromUtf8("македонски");
        // languageNames[QString::fromAscii("hi")] = QString::fromUtf8("हिंदी");
        // languageNames[QString::fromAscii("ms")] = QString::fromUtf8("Bahasa Malaysia");
        // languageNames[QString::fromAscii("cy")] = QString::fromUtf8("Cymraeg");
        // languageNames[QString::fromAscii("ee")] = QString::fromUtf8("Eesti");
        // languageNames[QString::fromAscii("fa")] = QString::fromUtf8("فارسی");
        // languageNames[QString::fromAscii("hr")] = QString::fromUtf8("Hrvatski");
        // languageNames[QString::fromAscii("ka")] = QString::fromUtf8("ქართული");
        // languageNames[QString::fromAscii("cs")] = QString::fromUtf8("Čeština");
        // languageNames[QString::fromAscii("sk")] = QString::fromUtf8("Slovenský");
        // languageNames[QString::fromAscii("sl")] = QString::fromUtf8("Slovenščina");
        // languageNames[QString::fromAscii("hu")] = QString::fromUtf8("Magyar");
        // languageNames[QString::fromAscii("fi")] = QString::fromUtf8("Suomi");
        // languageNames[QString::fromAscii("sr")] = QString::fromUtf8("српски");
        // languageNames[QString::fromAscii("sv")] = QString::fromUtf8("Svenska");
        // languageNames[QString::fromAscii("bg")] = QString::fromUtf8("български");
        // languageNames[QString::fromAscii("he")] = QString::fromUtf8("עברית");
        // languageNames[QString::fromAscii("tr")] = QString::fromUtf8("Türkçe");
        // languageNames[QString::fromAscii("tl")] = QString::fromUtf8("Tagalog");
        // languageNames[QString::fromAscii("uk")] = QString::fromUtf8("Українська");
        // languageNames[QString::fromAscii("pt")] = QString::fromUtf8("Português");


    }
    return languageNames.value(code);
}


struct IconCache
{
    std::map<QString, QIcon> mIcons;

    QIcon& getDirect(QString resourceName)
    {
        auto i = mIcons.find(resourceName);
        if (i == mIcons.end())
        {
            auto pair = mIcons.emplace(resourceName, QIcon());
            i = pair.first;
            i->second.addFile(resourceName, QSize(), QIcon::Normal, QIcon::Off);
        }
        return i->second;
    }

    QIcon& getByExtension(const QString &fileName, const QString &prefix)
    {
        return getDirect(Utilities::getExtensionPixmapName(fileName, prefix));
    }
};

IconCache gIconCache;

QString Utilities::getExtensionPixmapNameSmall(QString fileName)
{
    return getExtensionPixmapName(fileName, QString::fromAscii(":/images/small_"));
}

QString Utilities::getExtensionPixmapNameMedium(QString fileName)
{
    return getExtensionPixmapName(fileName, QString::fromAscii(":/images/drag_"));
}

QIcon Utilities::getCachedPixmap(QString fileName)
{
    return gIconCache.getDirect(fileName);
}

QIcon Utilities::getExtensionPixmapSmall(QString fileName)
{
    return gIconCache.getDirect(getExtensionPixmapNameSmall(fileName));
}

QIcon Utilities::getExtensionPixmapMedium(QString fileName)
{
    return gIconCache.getDirect(getExtensionPixmapNameMedium(fileName));
}

QString Utilities::getAvatarPath(QString email)
{
    if (!email.size())
    {
        return QString();
    }

    QString avatarsPath = QString::fromUtf8("%1/avatars/%2.jpg")
            .arg(Preferences::instance()->getDataPath()).arg(email);
    return QDir::toNativeSeparators(avatarsPath);
}

bool Utilities::removeRecursively(QString path)
{
    if (!path.size())
    {
        return false;
    }

    QDir dir(path);
    bool success = false;
    QString qpath = QDir::toNativeSeparators(dir.absolutePath());
    MegaApi::removeRecursively(qpath.toUtf8().constData());
    success = dir.rmdir(dir.absolutePath());
    return success;
}

void Utilities::copyRecursively(QString srcPath, QString dstPath)
{
    if (!srcPath.size() || !dstPath.size())
    {
        return;
    }

    QFileInfo source(srcPath);
    if (!source.exists())
    {
        return;
    }

    if (srcPath == dstPath)
    {
        return;
    }

    QFile dst(dstPath);
    if (dst.exists())
    {
        return;
    }

    if (source.isFile())
    {
        QFile src(srcPath);
        src.copy(dstPath);
#ifndef _WIN32
       QFileInfo info(src);
       time_t t = info.lastModified().toTime_t();
       struct utimbuf times = { t, t };
       utime(dstPath.toUtf8().constData(), &times);
#endif
    }
    else if (source.isDir())
    {
        QDir dstDir(dstPath);
        dstDir.mkpath(QString::fromAscii("."));
        QDirIterator di(srcPath, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
        while (di.hasNext())
        {
            di.next();
            if (!di.fileInfo().isSymLink())
            {
                copyRecursively(di.filePath(), dstPath + QDir::separator() + di.fileName());
            }
        }
    }
}

bool Utilities::verifySyncedFolderLimits(QString path)
{
#ifdef WIN32
    if (path.startsWith(QString::fromAscii("\\\\?\\")))
    {
        path = path.mid(4);
    }
#endif

    QString rootPath = QDir::toNativeSeparators(QDir::rootPath());
    if (rootPath == path)
    {
        return false;
    }
    return true;
}

void replaceLeadingZeroCharacterWithSpace(QString& string)
{
    if(!string.isEmpty() && string.at(0) == QLatin1Char('0'))
    {
        string.replace(0, 1, QLatin1Char(' '));
    }
}

QString Utilities::getTimeString(long long secs, bool secondPrecision, bool color)
{
    int seconds = (int) secs % 60;
    int minutes = (int) ((secs / 60) % 60);
    int hours   = (int) (secs / (60 * 60)) % 24;
    int days = (int)(secs / (60 * 60 * 24));
    QString colorString (color ? QLatin1String("color:#777777;") : QString());


    int items = 0;
    QString time;

    if (days)
    {
        items++;
        time.append(QString::fromUtf8(" %1 ").arg(days, 2, 10, QLatin1Char('0')));
        time.append(QString::fromUtf8("<span style=\"%1 text-decoration:none;\">d</span>").arg(colorString));
    }

    if (items || hours)
    {
        items++;
        time.append(QString::fromUtf8(" %1 ").arg(hours, 2, 10, QLatin1Char('0')));
        time.append(QString::fromUtf8("<span style=\"%1 text-decoration:none;\">h</span>").arg(colorString));
    }

    if (items == 2)
    {
        time = time.trimmed();
        replaceLeadingZeroCharacterWithSpace(time);
        return time;
    }

    if (items || minutes)
    {
        items++;
        time.append(QString::fromUtf8(" %1 ").arg(minutes, 2, 10, QLatin1Char('0')));
        time.append(QString::fromUtf8("<span style=\"%1 text-decoration:none;\">m</span>").arg(colorString));
    }

    if (items == 2)
    {
        time = time.trimmed();
        replaceLeadingZeroCharacterWithSpace(time);
        return time;
    }

    if (secondPrecision)
    {
        time.append(QString::fromUtf8(" %1 ").arg(seconds, 2, 10, QLatin1Char('0')));
        time.append(QString::fromUtf8("<span style=\"%1 text-decoration:none;\">s</span>").arg(colorString));
    }
    time = time.trimmed();
    replaceLeadingZeroCharacterWithSpace(time);
    return time;
}

struct Postfix
{
    double value;
    std::string letter;
};

const std::vector<Postfix> postfixes = {{1e12, "T"},
                                     {1e9,  "G"},
                                     {1e6,  "M"},
                                     {1e3,  "K"}};

constexpr auto maxStringSize = 4;

QString Utilities::getQuantityString(unsigned long long quantity)
{
    for (const auto& postfix : postfixes)
    {
        if(static_cast<double>(quantity) >= postfix.value)
        {
            const double value{static_cast<double>(quantity) / postfix.value};
            // QString::number(value, 'G', 3) is another way to do it but it rounds the result

            QString valueString{QString::number(value).left(maxStringSize)};
            if(valueString.contains(QStringLiteral(".")))
            {
                valueString.remove(QRegExp(QStringLiteral("0+$"))); // Remove any number of trailing 0's
                valueString.remove(QRegExp(QStringLiteral("\\.$"))); // If the last character is just a '.' then remove it
            }
            return valueString + QString::fromStdString(postfix.letter);
        }
    }
    return QString::number(quantity);
}

QString Utilities::getFinishedTimeString(long long secs)
{
    // <2 seconds: just now!
    if (secs < 2)
    {
        return QCoreApplication::translate("Utilities", "just now");
    }
    // < 1 minute
    else if (secs < 60)
    {
        return QCoreApplication::translate("Utilities", "%1 seconds ago").arg(secs);
    }
    // < 1 hour
    else if (secs < 3600)
    {
        // Compute minutes. We can safely cast because secs < 3600, thus minutes < 60
        int minutes = static_cast<int>(secs/60);
        if (minutes == 1)
        {
            return QCoreApplication::translate("Utilities", "1 minute ago");
        }
        else
        {
            return QCoreApplication::translate("Utilities", "%1 minutes ago").arg(minutes);
        }
    }
    // < 1 day
    else if (secs < 86400)
    {
        // Compute hours. We can safely cast because secs < 86400, thus hours < 24
        int hours = static_cast<int>(secs/3600);
        if (hours == 1)
        {
            return QCoreApplication::translate("Utilities", "1 hour ago");
        }
        else
        {
            return QCoreApplication::translate("Utilities", "%1 hours ago").arg(hours);
        }
    }
    // < 1 month
    else if (secs < 2628000)
    {
        // Compute days. We can safely cast because secs < 2628000, thus days < 31
        int days = static_cast<int>(secs/86400);
        if (days == 1)
        {
            return QCoreApplication::translate("Utilities", "1 day ago");
        }
        else
        {
            return QCoreApplication::translate("Utilities", "%1 days ago").arg(days);
        }
    }
    // < 1 year
    else if (secs < 31536000)
    {
        // Compute months. We can safely cast because secs < 31536000, thus days < 12
        int months = static_cast<int>(secs/2628000);
        if (months == 1)
        {
            return QCoreApplication::translate("Utilities", "1 month ago");
        }
        else
        {
            return QCoreApplication::translate("Utilities", "%1 months ago").arg(months);
        }
    }
    // We might not need century precision... give years.
    else
    {
        // Compute years. We need at least 64 bits to avoid any overflow.
        long long years = secs/31536000;
        if (years == 1)
        {
            return QCoreApplication::translate("Utilities", "1 year ago");
        }
        else
        {
            return QCoreApplication::translate("Utilities", "%1 years ago").arg(years);
        }
    }
}

QString Utilities::getSizeString(unsigned long long bytes)
{
    QString language = ((MegaApplication*)qApp)->getCurrentLanguageCode();
    QLocale locale(language);

    auto size = getSizeStringWithoutUnits(bytes);

    if (bytes >= TB)
    {
        return size + QString::fromAscii(" ")
                + QCoreApplication::translate("Utilities", "TB");
    }

    if (bytes >= GB)
    {
        return size + QString::fromAscii(" ")
                + QCoreApplication::translate("Utilities", "GB");
    }

    if (bytes >= MB)
    {
        return size + QString::fromAscii(" ")
                + QCoreApplication::translate("Utilities", "MB");
    }

    if (bytes >= KB)
    {
        return size + QString::fromAscii(" ")
                + QCoreApplication::translate("Utilities", "KB");
    }

    return size + QStringLiteral(" ")
            + QCoreApplication::translate("Utilities", "Bytes");
}

QString Utilities::getSizeString(long long bytes)
{
    if (bytes >= 0)
    {
        return getSizeString(static_cast<unsigned long long>(bytes));
    }
    QString language = ((MegaApplication*)qApp)->getCurrentLanguageCode();
    QLocale locale(language);
    return locale.toString(bytes) + QStringLiteral(" ")
            + QCoreApplication::translate("Utilities", "Bytes");
}

QString Utilities::getSizeStringWithoutUnits(unsigned long long bytes)
{
    QString language = ((MegaApplication*)qApp)->getCurrentLanguageCode();
    QLocale locale(language);
    if (bytes >= TB)
    {
        return locale.toString( ((int)((10 * bytes) / TB))/10.0);
    }

    if (bytes >= GB)
    {
        return locale.toString( ((int)((10 * bytes) / GB))/10.0);
    }

    if (bytes >= MB)
    {
        return locale.toString( ((int)((10 * bytes) / MB))/10.0);
    }

    if (bytes >= KB)
    {
        return locale.toString( ((int)((10 * bytes) / KB))/10.0);
    }

    return locale.toString(bytes) + QStringLiteral(" ");
}

QString Utilities::getSizeStringWithoutUnits(long long bytes)
{
    if (bytes >= 0)
    {
        return getSizeStringWithoutUnits(static_cast<unsigned long long>(bytes));
    }

    return QStringLiteral(" ");
}


QString Utilities::extractJSONString(QString json, QString name)
{
    QString pattern = name + QString::fromUtf8("\":\"");
    int pos = json.indexOf(pattern);
    if (pos < 0)
    {
        return QString();
    }

    int end = json.indexOf(QString::fromUtf8("\""), pos + pattern.size());
    if (end < 0)
    {
        return QString();
    }

    return json.mid(pos + pattern.size(), end - pos - pattern.size());
}

long long Utilities::extractJSONNumber(QString json, QString name)
{
    QString pattern = name + QString::fromUtf8("\":");
    int pos = json.indexOf(pattern);
    if (pos < 0)
    {
        return 0;
    }

    int end = pos + pattern.size();
    int count = 0;
    while (json[end].isDigit())
    {
        end++;
        count++;
    }

    return json.mid(pos + pattern.size(), count).toLongLong();
}

QString Utilities::getDefaultBasePath()
{
#ifdef WIN32
    #if QT_VERSION < 0x050000
        QString defaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }

        defaultPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }

        defaultPath = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }
    #else
        QStringList defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }
    #endif
#else
    #if QT_VERSION < 0x050000
        QString defaultPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }

        defaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }

        defaultPath = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
        if (!defaultPath.isEmpty())
        {
            return defaultPath;
        }
    #else
        QStringList defaultPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }
    #endif
#endif

    QDir dataDir = QDir(Preferences::instance()->getDataPath());
    QString rootPath = QDir::toNativeSeparators(dataDir.rootPath());
    if (rootPath.size() && rootPath.at(rootPath.size() - 1) == QDir::separator())
    {
        rootPath.resize(rootPath.size() - 1);
        return rootPath;
    }
    return QString();
}

void Utilities::getPROurlWithParameters(QString &url)
{
    Preferences *preferences = Preferences::instance();
    MegaApi *megaApi = ((MegaApplication *)qApp)->getMegaApi();

    if (!preferences || !megaApi)
    {
        return;
    }

    QString userAgent = QString::fromUtf8(QUrl::toPercentEncoding(QString::fromUtf8(megaApi->getUserAgent())));
    url.append(QString::fromUtf8("/uao=%1").arg(userAgent));

    MegaHandle aff;
    int affType;
    long long timestamp;
    preferences->getLastHandleInfo(aff, affType, timestamp);

    if (aff != INVALID_HANDLE)
    {
        char *base64aff = MegaApi::handleToBase64(aff);
        url.append(QString::fromUtf8("/aff=%1/aff_time=%2/aff_type=%3").arg(QString::fromUtf8(base64aff))
                                                                       .arg(timestamp / 1000)
                                                                       .arg(affType));
        delete [] base64aff;
    }
}

QString Utilities::joinLogZipFiles(MegaApi *megaApi, const QDateTime *timestampSince, QString appenHashReference)
{
    if (!megaApi)
    {
        return QString();
    }

    QDir logDir{MegaApplication::applicationDataPath().append(QString::fromUtf8("/") + LOGS_FOLDER_LEAFNAME_QSTRING)};
    if (logDir.exists())
    {
        QString fileFormat{QDir::separator() + QString::fromUtf8("%1%2%3")
                                                    .arg(QDateTime::currentDateTimeUtc().toString(QString::fromAscii("yyMMdd_hhmmss")))
                                                    .arg(megaApi->getMyUser() ? QString::fromUtf8("_") + QString::fromUtf8(std::unique_ptr<MegaUser>(megaApi->getMyUser())->getEmail()) : QString::fromUtf8(""))
                                                    .arg(!appenHashReference.isEmpty() ? QString::fromUtf8("_") + appenHashReference : QString::fromUtf8(""))};

        QFileInfo joinLogsFile(logDir.absolutePath().append(fileFormat).append(QString::fromUtf8(".gz")));
#ifdef _WIN32
        FILE * pFile = nullptr;
        errno_t er = _wfopen_s(&pFile, joinLogsFile.absoluteFilePath().toStdWString().c_str(), L"a+b");
        if (er)
        {
            megaApi->log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error opening file for joining log zip files (%1) : %2")
                         .arg(er).arg(joinLogsFile.filePath()).toUtf8().constData());
            pFile = nullptr; //just in case
        }

#else
        FILE * pFile = fopen(joinLogsFile.absoluteFilePath().toUtf8().constData(), "a+b");
#endif
        if (!pFile)
        {
            std::cerr << "Error opening file for joining log zip files " << std::endl;
            megaApi->log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error opening file for joining log zip files: %1").arg(joinLogsFile.filePath()).toUtf8().constData());
            return QString();
        }

        unsigned long crc, tot;
        gzinit(&crc, &tot, pFile);

        QFileInfoList logFiles = logDir.entryInfoList(QStringList() << QString::fromUtf8("MEGAsync.[0-9]*.log"), QDir::Files);
        int nLogFiles = logFiles.count();

        std::sort(logFiles.begin(), logFiles.end(), [](const QFileInfo &v1, const QFileInfo &v2){
            return v1.fileName().remove(QRegExp(QString::fromUtf8("[^\\d]"))).toInt() > v2.fileName().remove(QRegExp(QString::fromUtf8("[^\\d]"))).toInt();} );

        foreach (QFileInfo i, logFiles)
        {
            --nLogFiles;

            if (timestampSince)
            {
                if ( i.lastModified() < *timestampSince && i.fileName() != QString::fromUtf8("MEGAsync.0.log")) //keep at least the last log
                {
                    continue;
                }
            }

            try
            {
#ifdef _WIN32
                gzcopy(i.absoluteFilePath().toStdWString().c_str(), nLogFiles, &crc, &tot, pFile);
#else
                gzcopy(i.absoluteFilePath().toUtf8().constData(), nLogFiles, &crc, &tot, pFile);
#endif
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error joining zip files for bug report " << e.what() << std::endl;
                megaApi->log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error joining zip files for bug report : %1")
                             .arg(QString::fromUtf8(e.what())).toUtf8().constData());

                fclose(pFile);
                QFile::remove(joinLogsFile.absoluteFilePath());
                return QString();
            }
        }

        fclose(pFile);
        return joinLogsFile.absoluteFilePath();
    }

    return QString();
}

void Utilities::adjustToScreenFunc(QPoint position, QWidget *what)
{
    QDesktopWidget *desktop = QApplication::desktop();
    int screenIndex = desktop->screenNumber(position);
    auto screenGeometry = desktop->availableGeometry(screenIndex);
    if (screenGeometry.isValid())
    {
        int newx = what->x(), newy = what->y();
        if (what->x() < screenGeometry.x())
        {
            newx = screenGeometry.x();
        }
        if (what->y() < screenGeometry.y())
        {
            newy = screenGeometry.y();
        }
        if (what->x() + what->width() > screenGeometry.right())
        {
            newx = screenGeometry.right() - what->width();
        }
        if (what->y() + what->height() > screenGeometry.bottom())
        {
            newy = screenGeometry.bottom() - what->height();
        }
        if (newx != what->x() || newy != what->y())
        {
            what->move(newx, newy);
        }
    }
}

QString Utilities::minProPlanNeeded(std::shared_ptr<MegaPricing> pricing, long long usedStorage)
{
    if (!pricing)
    {
        return QString::fromUtf8("Pro");
    }

    int planNeeded = -1;
    int amountPlanNeeded = 0;
    int products = pricing->getNumProducts();
    for (int i = 0; i < products; i++)
    {
        //Skip business & non monthly plans to offer
        if (!pricing->isBusinessType(i) && pricing->getMonths(i) == 1)
        {
            if (usedStorage < (pricing->getGBStorage(i) * (long long)GB))
            {
                int currentAmountMonth = pricing->getAmountMonth(i);
                if (planNeeded == -1 || currentAmountMonth < amountPlanNeeded)
                {
                    planNeeded = i;
                    amountPlanNeeded = currentAmountMonth;
                }
            }
        }
    }

    return getReadablePROplanFromId(pricing->getProLevel(planNeeded));
}

QString Utilities::getReadableStringFromTs(MegaIntegerList *list)
{
    if (!list || !list->size())
    {
        return QString();
    }

    QString readableTimes;
    for (int it = qMax(0, list->size() - 3); it < list->size(); it++)//Display only the most recent 3 times
    {
        int64_t ts = list->get(it);
        QDateTime date = QDateTime::fromSecsSinceEpoch(ts);
        readableTimes.append(QLocale().toString(date.date(), QLocale::LongFormat));

        if (it != list->size() - 1)
        {
            readableTimes.append(QStringLiteral(", "));
        }
    }

    return readableTimes;
}

QString Utilities::getReadablePROplanFromId(int identifier)
{
    switch (identifier)
    {
        case MegaAccountDetails::ACCOUNT_TYPE_LITE:
            return QCoreApplication::translate("Utilities","Pro Lite");
            break;
        case MegaAccountDetails::ACCOUNT_TYPE_PROI:
            return QCoreApplication::translate("Utilities","Pro I");
            break;
        case MegaAccountDetails::ACCOUNT_TYPE_PROII:
            return QCoreApplication::translate("Utilities","Pro II");
            break;
        case MegaAccountDetails::ACCOUNT_TYPE_PROIII:
            return QCoreApplication::translate("Utilities","Pro III");
            break;
    }

    return QString::fromUtf8("Pro");
}

void Utilities::animateFadein(QWidget *object, int msecs)
{
    animateProperty(object, msecs, "opacity", 0.0, 1.0);
}

void Utilities::animateFadeout(QWidget *object, int msecs)
{
    animateProperty(object, msecs, "opacity", 1.0, 0.0);
}

void Utilities::animatePartialFadein(QWidget *object, int msecs)
{
    animateProperty(object, msecs, "opacity", 0.5, 1.0);
}

void Utilities::animatePartialFadeout(QWidget *object, int msecs)
{
    animateProperty(object, msecs, "opacity", 1.0, 0.5);
}

void Utilities::animateProperty(QWidget *object, int msecs, const char * property, QVariant startValue, QVariant endValue, QEasingCurve curve)
{
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
    object->setGraphicsEffect(effect);
    auto animation = new QPropertyAnimation(effect, property);
    animation->setDuration(msecs);
    animation->setStartValue(startValue);
    animation->setEndValue(endValue);
    animation->setEasingCurve(curve);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

// Returns remaining days until given Unix timestamp in seconds.
void Utilities::getDaysToTimestamp(int64_t secsTimestamps, int64_t &remainDays)
{
    // Reference timestamp shouldn't be 0
    if (!secsTimestamps)
    {
        return;
    }

    int64_t hours = 0;
    getDaysAndHoursToTimestamp(secsTimestamps, remainDays, hours);
}

// Returns remaining days / hours until given Unix timestamp in seconds.
// Note: remainingHours and remaininDays represent the same value.
// i.e. for 1 day & 3 hours remaining, remainingHours will be 27, not 3.
void Utilities::getDaysAndHoursToTimestamp(int64_t secsTimestamps, int64_t &remainDays, int64_t &remainHours)
{
    // Reference timestamp shouldn't be 0
    if (!secsTimestamps)
    {
        return;
    }

    // Get seconcs diff between now and secsTimestamps
    int64_t tDiff = secsTimestamps - QDateTime::currentDateTimeUtc().toSecsSinceEpoch();

    // Compute in hours, then in days
    remainHours = tDiff / 3600;
    remainDays  = remainHours / 24;
}

QProgressDialog *Utilities::showProgressDialog(ProgressHelper *progressHelper, QWidget *parent)
{
    QProgressDialog *progressDialog = new QProgressDialog(progressHelper->description(), QString()/*no cancel button*/, 0, 100, parent);
    progressDialog->setWindowFlags(progressDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setMinimumDuration(0);
    progressDialog->setValue(0);
    progressDialog->setAutoClose(false);
    progressDialog->setAutoReset(false);
    QObject::connect(progressDialog, SIGNAL(close()), progressDialog, SLOT(deleteLater()));
    progressDialog->show();
    auto startTime = QDateTime::currentMSecsSinceEpoch();

    QObject::connect(progressHelper, &ProgressHelper::progress, progressDialog, [progressDialog](double percentage){
        progressDialog->setValue(static_cast<int>(percentage * 100));
    });

    QObject::connect(progressHelper, &ProgressHelper::completed, progressDialog, [progressDialog, startTime](){
        progressDialog->setValue(100);

        //delay closing if thing went too fast, to avoid show/close glitch
        auto closeDelay = max(qint64(0), 350 - (QDateTime::currentMSecsSinceEpoch() - startTime) );
        QTimer::singleShot(closeDelay, [progressDialog] () {progressDialog->close(); });
    });

    return progressDialog;
}

long long Utilities::getSystemsAvailableMemory()
{
    long long availMemory = 0;
#ifdef _WIN32
    MEMORYSTATUSEX statex;
    memset(&statex, 0, sizeof (statex));
    statex.dwLength = sizeof (statex);
    if (GlobalMemoryStatusEx(&statex))
    {
        availMemory = std::min(statex.ullAvailPhys, statex.ullTotalVirtual);
    }
    else
    {
        std::cerr << "Error getting RAM usage info" << std::endl;
    }
#else
    long long pages = sysconf(_SC_PHYS_PAGES);
    long long page_size = sysconf(_SC_PAGE_SIZE);
    availMemory = (pages * page_size);
#endif
    return availMemory;
}

void Utilities::sleepMilliseconds(unsigned int milliseconds)
{
#ifdef WIN32
    Sleep(milliseconds);
#else
    // usleep accepts unsigned int argument.
    //Init to max allowed value, will be changed only if no overflow
    unsigned int usecs(std::numeric_limits<unsigned int>::max());

    // Avoid overflow. If no overflow, use passed value.
    if (milliseconds < (usecs/1000))
    {
        usecs = milliseconds * 1000;
    }

    // Call safely
    usleep(usecs);
#endif
}

int Utilities::partPer(unsigned long long  part, unsigned long long total, uint ref)
{
    // Use maximum precision
    long double partd(part);
    long double totald(total);
    long double refd(ref);

    // We can safely cast because the result should reasonably fit in an int.
    return (static_cast<int>((partd * refd) / totald));
}

void MegaListenerFuncExecuter::setExecuteInAppThread(bool executeInAppThread)
{
    mExecuteInAppThread = executeInAppThread;
}

void MegaListenerFuncExecuter::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    if (mExecuteInAppThread)
    {
        MegaRequest *requestCopy = request->copy();
        MegaError *errorCopy = e->copy();
        QObject temporary;
        QObject::connect(&temporary, &QObject::destroyed, qApp, [this, api, requestCopy, errorCopy](){

            if (onRequestFinishCallback)
            {
                onRequestFinishCallback(api, requestCopy, errorCopy);
            }

            if (mAutoremove)
            {
                delete this;
            }

        }, Qt::QueuedConnection);
    }
    else
    {
        if (onRequestFinishCallback)
        {
            onRequestFinishCallback(api, request, e);
        }

        if (mAutoremove)
        {
            delete this;
        }
    }
}
