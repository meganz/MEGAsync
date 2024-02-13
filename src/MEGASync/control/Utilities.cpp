
#include "Utilities.h"
#include "control/Preferences/Preferences.h"

#include <QApplication>
#include <QImageReader>
#include <QDirIterator>
#include <QTextStream>
#include <QDateTime>
#include <iostream>
#include <QDesktopWidget>
#include <QScreen>
#include "MegaApplication.h"
#include "control/gzjoin.h"
#include "platform/Platform.h"

#ifndef WIN32
#include "megaapi.h"
#include <utime.h>
#else
#include <windows.h>
#include <shellapi.h>
#endif

using namespace std;
using namespace mega;

QHash<QString, QString> Utilities::extensionIcons;
QHash<QString, Utilities::FileType> Utilities::fileTypes;
QHash<QString, QString> Utilities::languageNames;

std::unique_ptr<ThreadPool> ThreadPoolSingleton::instance = nullptr;

const QString Utilities::SUPPORT_URL = QString::fromUtf8("https://mega.nz/contact");
const QString Utilities::BACKUP_CENTER_URL = QString::fromLatin1("mega://#fm/devices");
const QString Utilities::SYNC_SUPPORT_URL = QString::fromLatin1("https://help.mega.io/installs-apps/desktop-syncing/sync-v2");

const unsigned long long KB = 1024;
const unsigned long long MB = 1024 * KB;
const unsigned long long GB = 1024 * MB;
const unsigned long long TB = 1024 * GB;

// Human-friendly list of forbidden chars for New Remote Folder
const QLatin1String Utilities::FORBIDDEN_CHARS("\\ / : \" * < > \? |");
// Forbidden chars PCRE using a capture list: [\\/:"\*<>?|]
const QRegularExpression Utilities::FORBIDDEN_CHARS_RX(QLatin1String("[\\\\/:\"*<>\?|]"));

void Utilities::initializeExtensions()
{
    extensionIcons[QString::fromLatin1("3ds")] = extensionIcons[QString::fromLatin1("3dm")]  = extensionIcons[QString::fromLatin1("max")] =
                            extensionIcons[QString::fromLatin1("obj")]  = QString::fromLatin1("3D.png");

    extensionIcons[QString::fromLatin1("aep")] = extensionIcons[QString::fromLatin1("aet")]  = QString::fromLatin1("aftereffects.png");

    extensionIcons[QString::fromLatin1("mp3")] = extensionIcons[QString::fromLatin1("wav")]  = extensionIcons[QString::fromLatin1("3ga")]  =
                            extensionIcons[QString::fromLatin1("aif")]  = extensionIcons[QString::fromLatin1("aiff")] =
                            extensionIcons[QString::fromLatin1("flac")] = extensionIcons[QString::fromLatin1("iff")]  = extensionIcons[QString::fromLatin1("ogg")] =
                            extensionIcons[QString::fromLatin1("m4a")]  = extensionIcons[QString::fromLatin1("wma")]  =  QString::fromLatin1("audio.png");

    extensionIcons[QString::fromLatin1("dxf")] = extensionIcons[QString::fromLatin1("dwg")] =  QString::fromLatin1("cad.png");


    extensionIcons[QString::fromLatin1("zip")] = extensionIcons[QString::fromLatin1("rar")] = extensionIcons[QString::fromLatin1("tgz")]  =
                            extensionIcons[QString::fromLatin1("gz")]  = extensionIcons[QString::fromLatin1("bz2")]  =
                            extensionIcons[QString::fromLatin1("tbz")] = extensionIcons[QString::fromLatin1("tar")]  =
                            extensionIcons[QString::fromLatin1("7z")]  = extensionIcons[QString::fromLatin1("sitx")] =  QString::fromLatin1("compressed.png");

    extensionIcons[QString::fromLatin1("sql")] = extensionIcons[QString::fromLatin1("accdb")] = extensionIcons[QString::fromLatin1("db")]  =
                            extensionIcons[QString::fromLatin1("dbf")]  = extensionIcons[QString::fromLatin1("mdb")]  =
                            extensionIcons[QString::fromLatin1("pdb")] = QString::fromLatin1("web_lang.png");

    extensionIcons[QString::fromLatin1("folder")] = QString::fromLatin1("folder.png");

    extensionIcons[QString::fromLatin1("xls")] = extensionIcons[QString::fromLatin1("xlsx")] = extensionIcons[QString::fromLatin1("xlt")]  =
                            extensionIcons[QString::fromLatin1("xltm")]  = QString::fromLatin1("excel.png");

    extensionIcons[QString::fromLatin1("exe")] = extensionIcons[QString::fromLatin1("com")] = extensionIcons[QString::fromLatin1("bin")]  =
                            extensionIcons[QString::fromLatin1("apk")]  = extensionIcons[QString::fromLatin1("app")]  =
                             extensionIcons[QString::fromLatin1("msi")]  = extensionIcons[QString::fromLatin1("cmd")]  =
                            extensionIcons[QString::fromLatin1("gadget")] = QString::fromLatin1("executable.png");

    extensionIcons[QString::fromLatin1("fnt")] = extensionIcons[QString::fromLatin1("otf")] = extensionIcons[QString::fromLatin1("ttf")]  =
                            extensionIcons[QString::fromLatin1("fon")]  = QString::fromLatin1("font.png");

    extensionIcons[QString::fromLatin1("gif")] = extensionIcons[QString::fromLatin1("tiff")]  = extensionIcons[QString::fromLatin1("tif")]  =
                            extensionIcons[QString::fromLatin1("bmp")]  = extensionIcons[QString::fromLatin1("png")] =
                            extensionIcons[QString::fromLatin1("tga")]  = QString::fromLatin1("image.png");

    extensionIcons[QString::fromLatin1("ai")] = extensionIcons[QString::fromLatin1("ait")] = QString::fromLatin1("illustrator.png");
    extensionIcons[QString::fromLatin1("jpg")] = extensionIcons[QString::fromLatin1("jpeg")] = extensionIcons[QString::fromLatin1("heic")] =
                            extensionIcons[QString::fromLatin1("webp")] = QString::fromLatin1("image.png");
    extensionIcons[QString::fromLatin1("indd")] = QString::fromLatin1("indesign.png");

    extensionIcons[QString::fromLatin1("jar")] = extensionIcons[QString::fromLatin1("java")]  = extensionIcons[QString::fromLatin1("class")]  = QString::fromLatin1("web_data.png");

    extensionIcons[QString::fromLatin1("pdf")] = QString::fromLatin1("pdf.png");
    extensionIcons[QString::fromLatin1("abr")] = extensionIcons[QString::fromLatin1("psb")]  = extensionIcons[QString::fromLatin1("psd")]  =
                            QString::fromLatin1("photoshop.png");

    extensionIcons[QString::fromLatin1("pps")] = extensionIcons[QString::fromLatin1("ppt")]  = extensionIcons[QString::fromLatin1("pptx")] = QString::fromLatin1("powerpoint.png");

    extensionIcons[QString::fromLatin1("prproj")] = extensionIcons[QString::fromLatin1("ppj")]  = QString::fromLatin1("premiere.png");

    extensionIcons[QString::fromLatin1("3fr")] = extensionIcons[QString::fromLatin1("arw")]  = extensionIcons[QString::fromLatin1("bay")]  =
                            extensionIcons[QString::fromLatin1("cr2")]  = extensionIcons[QString::fromLatin1("dcr")] =
                            extensionIcons[QString::fromLatin1("dng")] = extensionIcons[QString::fromLatin1("fff")]  =
                            extensionIcons[QString::fromLatin1("mef")] = extensionIcons[QString::fromLatin1("mrw")]  =
                            extensionIcons[QString::fromLatin1("nef")] = extensionIcons[QString::fromLatin1("pef")]  =
                            extensionIcons[QString::fromLatin1("rw2")] = extensionIcons[QString::fromLatin1("srf")]  =
                            extensionIcons[QString::fromLatin1("orf")] = extensionIcons[QString::fromLatin1("rwl")]  =
                            extensionIcons[QString::fromLatin1("ari")] = extensionIcons[QString::fromLatin1("braw")]  =
                            extensionIcons[QString::fromLatin1("crw")] = extensionIcons[QString::fromLatin1("cr3")]  =
                            extensionIcons[QString::fromLatin1("cap")] =
                            extensionIcons[QString::fromLatin1("dcs")] = extensionIcons[QString::fromLatin1("drf")]  =
                            extensionIcons[QString::fromLatin1("eip")] = extensionIcons[QString::fromLatin1("erf")]  =
                            extensionIcons[QString::fromLatin1("gpr")] = extensionIcons[QString::fromLatin1("iiq")]  =
                            extensionIcons[QString::fromLatin1("k25")] = extensionIcons[QString::fromLatin1("kdc")]  =
                            extensionIcons[QString::fromLatin1("mdc")] = extensionIcons[QString::fromLatin1("mos")]  =
                            extensionIcons[QString::fromLatin1("nrw")] = extensionIcons[QString::fromLatin1("obm")]  =
                            extensionIcons[QString::fromLatin1("ptx")] = extensionIcons[QString::fromLatin1("pxn")]  =
                            extensionIcons[QString::fromLatin1("r3d")] = extensionIcons[QString::fromLatin1("raf")]  =
                            extensionIcons[QString::fromLatin1("raw")] = extensionIcons[QString::fromLatin1("rwz")]  =
                            extensionIcons[QString::fromLatin1("sr2")] = extensionIcons[QString::fromLatin1("srw")]  =
                            extensionIcons[QString::fromLatin1("tif")] = extensionIcons[QString::fromLatin1("x3f")]  =
                            QString::fromLatin1("raw.png");

    extensionIcons[QString::fromLatin1("ods")]  = extensionIcons[QString::fromLatin1("ots")]  =
                            extensionIcons[QString::fromLatin1("gsheet")]  = extensionIcons[QString::fromLatin1("nb")] =
                            extensionIcons[QString::fromLatin1("xlr")] = QString::fromLatin1("spreadsheet.png");

    extensionIcons[QString::fromLatin1("torrent")] = QString::fromLatin1("torrent.png");
    extensionIcons[QString::fromLatin1("dmg")] = QString::fromLatin1("dmg.png");

    extensionIcons[QString::fromLatin1("txt")] = extensionIcons[QString::fromLatin1("rtf")]  = extensionIcons[QString::fromLatin1("ans")]  =
                            extensionIcons[QString::fromLatin1("ascii")]  = extensionIcons[QString::fromLatin1("log")] =
                            extensionIcons[QString::fromLatin1("odt")] = extensionIcons[QString::fromLatin1("wpd")]  =
                            QString::fromLatin1("text.png");

    extensionIcons[QString::fromLatin1("svgz")]  = extensionIcons[QString::fromLatin1("svg")]  =
                            extensionIcons[QString::fromLatin1("cdr")]  = extensionIcons[QString::fromLatin1("eps")] =
                            QString::fromLatin1("vector.png");

     extensionIcons[QString::fromLatin1("mkv")]  = extensionIcons[QString::fromLatin1("webm")]  =
                            extensionIcons[QString::fromLatin1("avi")]  = extensionIcons[QString::fromLatin1("mp4")] =
                            extensionIcons[QString::fromLatin1("m4v")] = extensionIcons[QString::fromLatin1("mpg")]  =
                            extensionIcons[QString::fromLatin1("mpeg")] = extensionIcons[QString::fromLatin1("mov")]  =
                            extensionIcons[QString::fromLatin1("3g2")] = extensionIcons[QString::fromLatin1("3gp")]  =
                            extensionIcons[QString::fromLatin1("asf")] = extensionIcons[QString::fromLatin1("wmv")]  =
                            extensionIcons[QString::fromLatin1("flv")] = extensionIcons[QString::fromLatin1("vob")] =
                            QString::fromLatin1("video.png");

     extensionIcons[QString::fromLatin1("html")]  = extensionIcons[QString::fromLatin1("xml")] = extensionIcons[QString::fromLatin1("shtml")]  =
                            extensionIcons[QString::fromLatin1("dhtml")] = extensionIcons[QString::fromLatin1("js")] =
                            extensionIcons[QString::fromLatin1("css")]  = QString::fromLatin1("web_data.png");

     extensionIcons[QString::fromLatin1("php")]  = extensionIcons[QString::fromLatin1("php3")]  =
                            extensionIcons[QString::fromLatin1("php4")]  = extensionIcons[QString::fromLatin1("php5")] =
                            extensionIcons[QString::fromLatin1("phtml")] = extensionIcons[QString::fromLatin1("inc")]  =
                            extensionIcons[QString::fromLatin1("asp")] = extensionIcons[QString::fromLatin1("pl")]  =
                            extensionIcons[QString::fromLatin1("cgi")] = extensionIcons[QString::fromLatin1("py")]  =
                            QString::fromLatin1("web_lang.png");

     extensionIcons[QString::fromLatin1("doc")]  = extensionIcons[QString::fromLatin1("docx")] = extensionIcons[QString::fromLatin1("dotx")]  =
                            extensionIcons[QString::fromLatin1("wps")] = QString::fromLatin1("word.png");

     extensionIcons[QString::fromLatin1("odt")]  = extensionIcons[QString::fromLatin1("ods")] = extensionIcons[QString::fromLatin1("odp")]  =
                            extensionIcons[QString::fromLatin1("odb")] = extensionIcons[QString::fromLatin1("odg")] = QString::fromLatin1("openoffice.png");

     extensionIcons[QString::fromLatin1("sketch")] = QString::fromLatin1("sketch.png");
     extensionIcons[QString::fromLatin1("xd")] = QString::fromLatin1("experiencedesign.png");
     extensionIcons[QString::fromLatin1("pages")] = QString::fromLatin1("pages.png");
     extensionIcons[QString::fromLatin1("numbers")] = QString::fromLatin1("numbers.png");
     extensionIcons[QString::fromLatin1("key")] = QString::fromLatin1("keynote.png");
}

void Utilities::initializeFileTypes()
{
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.wav"), QString())]
            = FileType::TYPE_AUDIO;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.mkv"), QString())]
            = FileType::TYPE_VIDEO;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.tar"), QString())]
            = FileType::TYPE_ARCHIVE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.torrent"), QString())]
            = FileType::TYPE_ARCHIVE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.dmg"), QString())]
            = FileType::TYPE_ARCHIVE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.xd"), QString())]
            = FileType::TYPE_ARCHIVE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.sketch"), QString())]
            = FileType::TYPE_ARCHIVE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.txt"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.odt"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.pdf"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.doc"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.ods"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.odt"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.ppt"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.pages"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.numbers"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.key"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.xml"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.xls"), QString())]
            = FileType::TYPE_DOCUMENT;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.png"), QString())]
            = FileType::TYPE_IMAGE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.jpg"), QString())]
            = FileType::TYPE_IMAGE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.ai"), QString())]
            = FileType::TYPE_IMAGE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.abr"), QString())]
            = FileType::TYPE_IMAGE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.3fr"), QString())]
            = FileType::TYPE_IMAGE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.svg"), QString())]
            = FileType::TYPE_IMAGE;
    fileTypes[getExtensionPixmapName(QLatin1Literal("a.bin"), QString())]
            = FileType::TYPE_OTHER;
}


void Utilities::queueFunctionInAppThread(std::function<void()> fun) {
   QObject temporary;
   QObject::connect(&temporary, &QObject::destroyed, qApp, std::move(fun), Qt::QueuedConnection);
}

void Utilities::queueFunctionInObjectThread(QObject* object, std::function<void()> fun) {
   QObject temporary;
   QObject::connect(&temporary, &QObject::destroyed, object, std::move(fun), Qt::QueuedConnection);
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
        return prefix + QString::fromLatin1("generic.png");
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
        languageNames[QString::fromLatin1("ar")] = QString::fromUtf8("العربية"); // arabic
        languageNames[QString::fromLatin1("de")] = QString::fromUtf8("Deutsch");
        languageNames[QString::fromLatin1("en")] = QString::fromUtf8("English");
        languageNames[QString::fromLatin1("es")] = QString::fromUtf8("Español");
        languageNames[QString::fromLatin1("fr")] = QString::fromUtf8("Français");
        languageNames[QString::fromLatin1("id")] = QString::fromUtf8("Bahasa Indonesia");
        languageNames[QString::fromLatin1("it")] = QString::fromUtf8("Italiano");
        languageNames[QString::fromLatin1("ja")] = QString::fromUtf8("日本語"); // japanese
        languageNames[QString::fromLatin1("ko")] = QString::fromUtf8("한국어"); // korean
        languageNames[QString::fromLatin1("nl")] = QString::fromUtf8("Nederlands");
        languageNames[QString::fromLatin1("pl")] = QString::fromUtf8("Polski");
        languageNames[QString::fromLatin1("pt")] = QString::fromUtf8("Português");
        languageNames[QString::fromLatin1("ro")] = QString::fromUtf8("Română");
        languageNames[QString::fromLatin1("ru")] = QString::fromUtf8("Pусский");
        languageNames[QString::fromLatin1("th")] = QString::fromUtf8("ภาษาไทย"); // thai
        languageNames[QString::fromLatin1("vi")] = QString::fromUtf8("Tiếng Việt");
        languageNames[QString::fromLatin1("zh_CN")] = QString::fromUtf8("简体中文");
        languageNames[QString::fromLatin1("zh_TW")] = QString::fromUtf8("中文繁體");


        // Currently unsupported
        // languageNames[QString::fromLatin1("mi")] = QString::fromUtf8("Māori");
        // languageNames[QString::fromLatin1("ca")] = QString::fromUtf8("Català");
        // languageNames[QString::fromLatin1("eu")] = QString::fromUtf8("Euskara");
        // languageNames[QString::fromLatin1("af")] = QString::fromUtf8("Afrikaans");
        // languageNames[QString::fromLatin1("no")] = QString::fromUtf8("Norsk");
        // languageNames[QString::fromLatin1("bs")] = QString::fromUtf8("Bosanski");
        // languageNames[QString::fromLatin1("da")] = QString::fromUtf8("Dansk");
        // languageNames[QString::fromLatin1("el")] = QString::fromUtf8("ελληνικά");
        // languageNames[QString::fromLatin1("lt")] = QString::fromUtf8("Lietuvos");
        // languageNames[QString::fromLatin1("lv")] = QString::fromUtf8("Latviešu");
        // languageNames[QString::fromLatin1("mk")] = QString::fromUtf8("македонски");
        // languageNames[QString::fromLatin1("hi")] = QString::fromUtf8("हिंदी");
        // languageNames[QString::fromLatin1("ms")] = QString::fromUtf8("Bahasa Malaysia");
        // languageNames[QString::fromLatin1("cy")] = QString::fromUtf8("Cymraeg");
        // languageNames[QString::fromLatin1("ee")] = QString::fromUtf8("Eesti");
        // languageNames[QString::fromLatin1("fa")] = QString::fromUtf8("فارسی");
        // languageNames[QString::fromLatin1("hr")] = QString::fromUtf8("Hrvatski");
        // languageNames[QString::fromLatin1("ka")] = QString::fromUtf8("ქართული");
        // languageNames[QString::fromLatin1("cs")] = QString::fromUtf8("Čeština");
        // languageNames[QString::fromLatin1("sk")] = QString::fromUtf8("Slovenský");
        // languageNames[QString::fromLatin1("sl")] = QString::fromUtf8("Slovenščina");
        // languageNames[QString::fromLatin1("hu")] = QString::fromUtf8("Magyar");
        // languageNames[QString::fromLatin1("fi")] = QString::fromUtf8("Suomi");
        // languageNames[QString::fromLatin1("sr")] = QString::fromUtf8("српски");
        // languageNames[QString::fromLatin1("sv")] = QString::fromUtf8("Svenska");
        // languageNames[QString::fromLatin1("bg")] = QString::fromUtf8("български");
        // languageNames[QString::fromLatin1("he")] = QString::fromUtf8("עברית");
        // languageNames[QString::fromLatin1("tr")] = QString::fromUtf8("Türkçe");
        // languageNames[QString::fromLatin1("tl")] = QString::fromUtf8("Tagalog");
        // languageNames[QString::fromLatin1("uk")] = QString::fromUtf8("Українська");
        // languageNames[QString::fromLatin1("pt")] = QString::fromUtf8("Português");


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
    return getExtensionPixmapName(fileName, QString::fromLatin1(":/images/small_"));
}

QString Utilities::getExtensionPixmapNameMedium(QString fileName)
{
    return getExtensionPixmapName(fileName, QString::fromLatin1(":/images/drag_"));
}

double Utilities::toDoubleInUnit(unsigned long long bytes, unsigned long long unit)
{
    double decimalMultiplier = 100.0;
    double multipliedValue = decimalMultiplier * static_cast<double>(bytes);
    return static_cast<int>(multipliedValue / static_cast<double>(unit)) / decimalMultiplier;
}

QString Utilities::getTimeFormat(const TimeInterval &interval)
{
    QString timeFormat;

    if (interval.days)
    {
        timeFormat = QCoreApplication::translate("Utilities", "[DAYS] [HOURS]");
        if (!interval.hours)
        {
            timeFormat.remove(QString::fromLatin1("[HOURS]"));
        }
    }
    else if (interval.hours)
    {
        timeFormat = QCoreApplication::translate("Utilities", "[HOURS] [MINUTES]");
        if (!interval.minutes)
        {
            timeFormat.remove(QString::fromLatin1("[MINUTES]"));
        }
    }
    else
    {
        timeFormat = QCoreApplication::translate("Utilities", "[MINUTES] [SECONDS]");
        if (!interval.minutes)
        {
            timeFormat.remove(QString::fromLatin1("[MINUTES]"));
        }
        if (!interval.useSecondPrecision)
        {
            timeFormat.remove(QString::fromLatin1("[SECONDS]"));
        }
    }

    return timeFormat.trimmed();
}

QString Utilities::filledTimeString(const QString &timeFormat, const TimeInterval &interval, bool color)
{
    QString daysFormat = QCoreApplication::translate("Utilities", "%1 [A]d[/A]");
    QString hoursFormat = QCoreApplication::translate("Utilities", "%1 [A]h[/A]");
    QString minutesFormat = QCoreApplication::translate("Utilities", "%1 [A]m[/A]");
    QString secondsFormat = QCoreApplication::translate("Utilities", "%1 [A]s[/A]");

    QString timeString = timeFormat;

    QString timeCountStr = daysFormat.arg(interval.days, 2, 10, QLatin1Char('0'));
    timeString.replace(QString::fromLatin1("[DAYS]"), timeCountStr);

    timeCountStr = hoursFormat.arg(interval.hours, 2, 10, QLatin1Char('0'));
    timeString.replace(QString::fromLatin1("[HOURS]"), timeCountStr);

    timeCountStr = minutesFormat.arg(interval.minutes, 2, 10, QLatin1Char('0'));
    timeString.replace(QString::fromLatin1("[MINUTES]"), timeCountStr);

    timeCountStr = secondsFormat.arg(interval.seconds, 2, 10, QLatin1Char('0'));
    timeString.replace(QString::fromLatin1("[SECONDS]"), timeCountStr);

    QString colorString = (color ? QLatin1String("color:#777777;") : QString());
    QString styleStartTag = QString::fromUtf8("<span style=\"%1 text-decoration:none;\">").arg(colorString);
    QString styleEndTag = QString::fromLatin1("</span>");

    timeString.replace(QString::fromLatin1("[A]"),  styleStartTag);
    timeString.replace(QString::fromLatin1("[/A]"), styleEndTag);

    return cleanedTimeString(timeString);
}

QString Utilities::cleanedTimeString(const QString &timeString)
{
    QString cleanedStr = timeString;
    if(!cleanedStr.isEmpty() && cleanedStr.at(0) == QLatin1Char('0'))
    {
        cleanedStr.replace(0, 1, QLatin1Char(' '));
    }
    return cleanedStr;
}

unsigned long long Utilities::getNearestUnit(long long bytes)
{
    unsigned long long unsignedBytes = static_cast<unsigned long long>(bytes);
    if (unsignedBytes >= TB)
    {
        return TB;
    }
    else if (unsignedBytes >= GB)
    {
        return GB;
    }
    else if (unsignedBytes >= MB)
    {
        return MB;
    }
    else if (unsignedBytes >= KB)
    {
        return KB;
    }
    return 1;
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
        dstDir.mkpath(QString::fromLatin1("."));
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

QString Utilities::getTimeString(long long secs, bool secondPrecision, bool color)
{
    TimeInterval interval(secs, secondPrecision);
    return filledTimeString(getTimeFormat(interval), interval, color);
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

QString Utilities::getAddedTimeString(long long secs)
{
    const int SECS_IN_1_MINUTE = 60;
    const int SECS_IN_1_HOUR = 3600;
    const int SECS_IN_1_DAY = 86400;
    const int SECS_IN_1_MONTH = 2592000;
    const int SECS_IN_1_YEAR = 31536000;

    if (secs < 2)
    {
        return QCoreApplication::translate("Utilities", "Added just now");
    }
    else if (secs < SECS_IN_1_MINUTE)
    {
        const int secsAsInt = static_cast<int>(secs);
        return QCoreApplication::translate("Utilities", "Added %n second ago", "", secsAsInt);
    }
    else if (secs < SECS_IN_1_HOUR)
    {
        const int minutes = static_cast<int>(secs/SECS_IN_1_MINUTE);
        return QCoreApplication::translate("Utilities", "Added %n minute ago", "", minutes);
    }
    else if (secs < SECS_IN_1_DAY)
    {
        const int hours = static_cast<int>(secs/SECS_IN_1_HOUR);
        return QCoreApplication::translate("Utilities", "Added %n hour ago", "", hours);
    }
    else if (secs < SECS_IN_1_MONTH)
    {
        const int days = static_cast<int>(secs/SECS_IN_1_DAY);
        return QCoreApplication::translate("Utilities", "Added %n day ago", "", days);
    }
    else if (secs < SECS_IN_1_YEAR)
    {
        const int months = static_cast<int>(secs/SECS_IN_1_MONTH);
        return QCoreApplication::translate("Utilities", "Added %n month ago", "", months);
    }
    // We might not need century precision... give years.
    else
    {
        const int years = static_cast<int>(secs/SECS_IN_1_YEAR);
        return QCoreApplication::translate("Utilities", "Added %n year ago", "", years);
    }
}

QString Utilities::getSizeString(unsigned long long bytes)
{
    QString language = ((MegaApplication*)qApp)->getCurrentLanguageCode();
    QLocale locale(language);
    
    if (bytes >= TB)
    {
        return locale.toString(toDoubleInUnit(bytes, TB)) + QString::fromLatin1(" ")
                + QCoreApplication::translate("Utilities", "TB");
    }

    if (bytes >= GB)
    {
        return locale.toString(toDoubleInUnit(bytes, GB)) + QString::fromLatin1(" ")
                + QCoreApplication::translate("Utilities", "GB");
    }

    if (bytes >= MB)
    {
        return locale.toString(toDoubleInUnit(bytes, MB)) + QString::fromLatin1(" ")
                + QCoreApplication::translate("Utilities", "MB");
    }

    if (bytes >= KB)
    {
        return locale.toString(toDoubleInUnit(bytes, KB)) + QString::fromLatin1(" ")
                + QCoreApplication::translate("Utilities", "KB");
    }

    return locale.toString(toDoubleInUnit(bytes, 1)) + QString::fromLatin1(" ")
                    + QCoreApplication::translate("Utilities", "Bytes");
}

QString Utilities::getSizeStringLocalized(quint64 bytes)
{
    auto baseUnitSize = Platform::getInstance()->getBaseUnitsSize();

    static const QVector<QPair<QString, quint64>> unitsSize{
        {QLatin1String("TB"), static_cast<quint64>(std::pow(baseUnitSize, 4))},
        {QLatin1String("GB"), static_cast<quint64>(std::pow(baseUnitSize, 3))},
        {QLatin1String("MB"), static_cast<quint64>(std::pow(baseUnitSize, 2))},
        {QLatin1String("KB"), baseUnitSize},
        {QLatin1String("Bytes"), 1}
    };

    auto foundIt = std::find_if(unitsSize.constBegin(), unitsSize.constEnd(), [&bytes](const QPair<QString, quint64>& pair){
        return bytes >= pair.second;
    });

    QString language = ((MegaApplication*)qApp)->getCurrentLanguageCode();
    QLocale locale(language);

    if (foundIt != unitsSize.constEnd())
    {
        return locale.toString(toDoubleInUnit(bytes, foundIt->second)) + QString::fromLatin1(" ")
                + QCoreApplication::translate("Utilities", foundIt->first.toStdString().c_str());
    }
    else
    {
        return locale.toString(toDoubleInUnit(bytes, 1)) + QString::fromLatin1(" ")
                + QCoreApplication::translate("Utilities", "Bytes");
    }
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

int Utilities::toNearestUnit(long long bytes)
{
    unsigned long long nearestUnit = getNearestUnit(bytes);
    double inNearestUnit = toDoubleInUnit(bytes, nearestUnit);
    return static_cast<int>(inNearestUnit);
}

Utilities::ProgressSize Utilities::getProgressSizes(unsigned long long transferredBytes, unsigned long long totalBytes)
{
    ProgressSize sizes;
    Q_ASSERT(totalBytes >= transferredBytes);

    if (transferredBytes >= 0 && totalBytes >= 0)
    {    
        QString language = ((MegaApplication*)qApp)->getCurrentLanguageCode();
        QLocale locale(language);

        //Init on 1 for Bytes
        unsigned long long sizeToCompare(0);
        QString Units;

        if (totalBytes >= TB)
        {
            sizeToCompare = TB;
            Units = QCoreApplication::translate("Utilities", "TB");
        }
        else if (totalBytes >= GB)
        {
            sizeToCompare = GB;
            Units = QCoreApplication::translate("Utilities", "GB");
        }
        else if (totalBytes >= MB)
        {
            sizeToCompare = MB;
            Units = QCoreApplication::translate("Utilities", "MB");
        }
        else if (totalBytes >= KB)
        {
            sizeToCompare = KB;
            Units = QCoreApplication::translate("Utilities", "KB");
        }
        else
        {
            sizeToCompare = 1;
            Units = QCoreApplication::translate("Utilities", "Bytes");
        }

        sizes.transferredBytes = locale.toString( ((int)((10 * transferredBytes) / sizeToCompare))/10.0);
        sizes.totalBytes = locale.toString( ((int)((10 * totalBytes) / sizeToCompare))/10.0);
        sizes.units = QStringLiteral(" ") + Units;
    }

    return sizes;
}

QString Utilities::createSimpleUsedString(long long usedData)
{
    return createSimpleUsedStringWithoutReplacement(usedData).arg(getSizeString(usedData));
}

QString Utilities::createSimpleUsedStringWithoutReplacement(long long usedData)
{
    return QCoreApplication::translate("Utilities", "%1 used", "", toNearestUnit(usedData));
}

QString Utilities::createCompleteUsedString(long long usedData, long long totalData, int percentage)
{
    return QCoreApplication::translate("Utilities", "%1 (%2%) of %3 used", "", toNearestUnit(usedData)).arg(
            getSizeString(usedData),
            QString::number(percentage),
            getSizeString(totalData)
            );
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
        QStringList defaultPaths;
#ifdef WIN32
        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }
#else
        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }

        defaultPaths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        if (defaultPaths.size())
        {
            return defaultPaths.at(0);
        }
#endif
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
    auto preferences = Preferences::instance();
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
                                                    .arg(QDateTime::currentDateTimeUtc().toString(QString::fromLatin1("yyMMdd_hhmmss")))
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
    const auto screen = QGuiApplication::screenAt(position);
    auto screenGeometry = screen->availableGeometry();
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
        QDateTime date = QDateTime::fromMSecsSinceEpoch(ts * 1000l);
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
    int64_t tDiff = secsTimestamps - QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;

    // Compute in hours, then in days
    remainHours = tDiff / 3600;
    remainDays  = remainHours / 24;
}

QString Utilities::getNonDuplicatedNodeName(MegaNode *node, MegaNode *parentNode, const QString &currentName, bool unescapeName, const QStringList& itemsBeingRenamed)
{
    QString newName;
    QString nodeName;
    QString suffix;

    if(node->isFile())
    {
        QFileInfo fileInfo(currentName);

        auto nameSplitted = Utilities::getFilenameBasenameAndSuffix(fileInfo.fileName());
        if(nameSplitted != QPair<QString, QString>())
        {
            nodeName = nameSplitted.first;
            suffix = nameSplitted.second;
        }
        else
        {
            nodeName = fileInfo.fileName();
        }
    }
    else
    {
        nodeName = currentName;
    }

    if(unescapeName)
    {
        nodeName = QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(nodeName.toUtf8().constData()));
    }

    int counter(1);
    while(newName.isEmpty())
    {
        bool nameFound = false;
        QString suggestedName = nodeName + QString(QLatin1Literal("(%1)")).arg(QString::number(counter));
        if(node)
        {
            if(node->isFile() && !suffix.isEmpty())
            {
                suggestedName.append(suffix);
            }
        }

        if(!itemsBeingRenamed.contains(suggestedName, Qt::CaseInsensitive))
        {
            std::unique_ptr<MegaNodeList>nodes(MegaSyncApp->getMegaApi()->getChildren(parentNode));
            for(int index = 0; index < nodes->size(); ++index)
            {
                QString nodeName(QString::fromUtf8(nodes->get(index)->getName()));
                if(suggestedName.compare(nodeName, Qt::CaseInsensitive) == 0)
                {
                    nameFound = true;
                }
            }


            if(!nameFound)
            {
                newName = suggestedName;
            }
        }

        counter++;
    }

    return newName;
}

QString Utilities::getNonDuplicatedLocalName(const QFileInfo &currentFile, bool unescapeName, const QStringList& itemsBeingRenamed)
{
    QString repeatedName;
    QString suffix = currentFile.completeSuffix();

    QString fileName;
    if(unescapeName)
    {
        fileName = QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(currentFile.baseName().toUtf8().constData()));
    }
    else
    {
        fileName = currentFile.baseName();
    }

    QDirIterator filesIt(currentFile.path(), QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::NoIteratorFlags);

    int counter(0);
    bool notFound(true);
    do
    {
        notFound = true;
        counter++;

        QString suggestedName = fileName + QString(QLatin1Literal("(%1)")).arg(QString::number(counter));
        if(!suffix.isEmpty())
        {
            suggestedName += QString(QLatin1Literal(".%1")).arg(suffix);
        }

        if(!itemsBeingRenamed.contains(suggestedName))
        {
            while (filesIt.hasNext())
            {
                QString checkFileName;

                if(unescapeName)
                {
                    checkFileName = QString::fromUtf8(MegaSyncApp->getMegaApi()->unescapeFsIncompatible(filesIt.fileName().toUtf8().constData()));
                }
                else
                {
                    checkFileName = filesIt.fileName();
                }

                if (checkFileName.compare(suggestedName, Qt::CaseInsensitive) == 0)
                {
                    notFound = false;
                    break;
                }

                filesIt.next();
            }
        }

    } while(!notFound);

    repeatedName = currentFile.baseName() + QString(QLatin1Literal("(%1)")).arg(QString::number(counter));
    if(!suffix.isEmpty())
    {
        repeatedName += QString(QLatin1Literal(".%1")).arg(suffix);
    }

    return repeatedName;
}

QPair<QString, QString> Utilities::getFilenameBasenameAndSuffix(const QString& fileName)
{
    QMimeDatabase db;
    int length = fileName.length();
    QList <QPair <int, QMimeType> > list;
    for (int index = length; index > -1; index--)
    {
        QList<QMimeType> mimes = db.mimeTypesForFileName(fileName.section(QLatin1String(""), index, length));
        QMimeType mime = mimes.isEmpty() ? QMimeType() : mimes.last();
        if (mime.isValid() && (list.isEmpty() || list.last().second != mime))
        {
            list.push_back(qMakePair(index, mime));
        }
    }

    QPair<QString, QString> result;

    if (!list.isEmpty())
    {
#ifdef Q_OS_LINUX
        result = qMakePair<QString, QString>(fileName.left(list.last().first), fileName.mid(list.last().first));
#else
        result = qMakePair<QString, QString>(fileName.left(list.last().first - 1), fileName.mid(list.last().first - 1));
#endif
    }

    return result;
}

void Utilities::upgradeClicked()
{
    QString url = QString::fromUtf8("mega://#pro");
    getPROurlWithParameters(url);
    openUrl(QUrl(url));
}

QString Utilities::getNodePath(MegaTransfer* transfer)
{
    if (transfer->getPath() != nullptr)
    {
        return QString::fromUtf8(transfer->getPath());
    }
    return QString::fromUtf8(transfer->getParentPath()) + QString::fromUtf8(transfer->getFileName());
}

bool Utilities::isBusinessAccount()
{
    int accountType = Preferences::instance()->accountType();
    return accountType == Preferences::ACCOUNT_TYPE_BUSINESS
            || accountType == Preferences::ACCOUNT_TYPE_PRO_FLEXI;
}

QFuture<bool> Utilities::openUrl(QUrl url)
{
    return QtConcurrent::run(QDesktopServices::openUrl, url);
}

void Utilities::openInMega(MegaHandle handle)
{
    auto api (MegaSyncApp->getMegaApi());
    if (api)
    {
        std::unique_ptr<MegaNode> node (api->getNodeByHandle(handle));
        if (node)
        {
            std::unique_ptr<char[]> h (node->getBase64Handle());
            if(h)
            {
                openUrl(QUrl(QLatin1String("mega://#fm/") + QString::fromLatin1(h.get())));
            }
        }
    }
}

void Utilities::openBackupCenter()
{
    openUrl(QUrl(Utilities::BACKUP_CENTER_URL));
}

QString Utilities::getCommonPath(const QString &path1, const QString &path2, bool cloudPaths)
{
    QString ret;
    QString firstPath;
    QString secondPath;

    if(path1 < path2)
    {
        firstPath = cloudPaths ? path1 : QDir::toNativeSeparators(path1);
        secondPath = cloudPaths ? path2 : QDir::toNativeSeparators(path2);
    }
    else
    {
        firstPath = cloudPaths ? path2 : QDir::toNativeSeparators(path2);
        secondPath = cloudPaths ? path1 : QDir::toNativeSeparators(path1);
    }

    QString separator = cloudPaths ? QLatin1String("/") : QString(QDir::separator());
    if(!firstPath.endsWith(separator))
    {
        firstPath.append(separator);
    }

    if(!secondPath.endsWith(separator))
    {
        secondPath.append(separator);
    }

    int index = 1;
    while (firstPath.mid(0, index) == secondPath.mid(0, index))
    {
        ret = firstPath.mid(0, index);
        index++;
    }

    //Just in case the folder path contains common characters but they are not the same
    //EXAMPLE and EXAMPLE_1 for example
    //If ret does not end with a separator it is because folders are not the same
    if(!ret.endsWith(separator))
    {
        auto splittedPath = ret.split(separator);
        if(!splittedPath.isEmpty())
        {
            ret.remove(ret.lastIndexOf(separator) + 1,splittedPath.last().length());
        }
    }

    if(ret.endsWith(separator))
    {
        ret = ret.remove(ret.length() - 1, 1);
    }

    return ret;
}

bool Utilities::isIncommingShare(MegaNode *node)
{
    if(node && MegaSyncApp->getMegaApi()->checkAccess(node, MegaShare::ACCESS_OWNER).getErrorCode() != MegaError::API_OK)
    {
        return true;
    }

    return false;
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

bool Utilities::isNodeNameValid(const QString& name)
{
    QString trimmedName (name.trimmed());
    return !trimmedName.isEmpty() && !trimmedName.contains(FORBIDDEN_CHARS_RX);
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

WrappedNode::WrappedNode(TransferOrigin from, MegaNode *node, bool undelete)
    : mTransfersFrom(from), mNode(node), mUndelete(undelete)
{
    qRegisterMetaType<QQueue<WrappedNode*>>("QQueue<WrappedNode*>");
}

TimeInterval::TimeInterval(long long secs, bool secondPrecision)
    : useSecondPrecision(secondPrecision)
{
    days = static_cast<int>(secs / (60 * 60 * 24));
    hours   = static_cast<int>(secs / (60 * 60)) % 24;
    minutes = static_cast<int>((secs / 60) % 60);
    seconds = static_cast<int>(secs % 60);
}

//Create Folders
std::shared_ptr<MegaNode> PathCreator::createFolder(MegaNode* parentNode, const QString &folderName)
{
    std::shared_ptr<MegaNode> node(MegaSyncApp->getMegaApi()->getChildNode(parentNode, folderName.toUtf8().constData()));
    if(!node)
    {
        MegaSyncApp->getMegaApi()->createFolder(folderName.toUtf8().constData(), parentNode,
                                                new OnFinishOneShot(MegaSyncApp->getMegaApi(), this,[this, &node]
                                                                    (bool, const MegaRequest& request,const MegaError& e)
                                                                    {
                                                                        if(e.getErrorCode() == MegaError::API_OK)
                                                                        {
                                                                            node.reset(MegaSyncApp->getMegaApi()->getNodeByHandle(request.getNodeHandle()));
                                                                        }
                                                                        mEventLoop.quit();
                                                                    }));
        //In order to execute in synchronously
        mEventLoop.exec();
    }

    return node;
}


std::shared_ptr<MegaNode> PathCreator::mkDir(const QString& root, const QString &path)
{
    std::shared_ptr<MegaNode> nodeCreated(root.isEmpty() ? MegaSyncApp->getMegaApi()->getRootNode() :
                                           MegaSyncApp->getMegaApi()->getNodeByPath(root.toUtf8().constData()));
    auto auxPath(path);

    mPathCreated = auxPath.split(QLatin1String("/"));

    while(!mPathCreated.isEmpty())
    {
        auto followingPath(mPathCreated.takeFirst());
        if(followingPath.isEmpty())
        {
            continue;
        }

        nodeCreated = createFolder(nodeCreated.get(), followingPath);
        if(!nodeCreated)
        {
            mPathCreated.clear();
        }
    }

    return nodeCreated;
}


//Move to BIN
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief MoveToCloudBinUtilities::moveToBin
/// \param handles
/// \param binFolderName
/// \param addDateFolder
/// This method is run synchronously. So, it waits for the SDK to create the folders, and only then we move the files
/// As the SDK createFolder is async, we need to use QEventLoop to stop the thread (which is not the main thread, so UI will not be frozen)
/// When the createFolder returns we continue or stop the eventloop.
/// When all files are moved, we stop the eventloop and we continue
bool MoveToCloudBinUtilities::moveToBin(const QList<MegaHandle>& handles, const QString& binFolderName, bool addDateFolder)
{
    mHandles = handles;

    auto moveLambda = [this](std::unique_ptr<MegaNode> rubbishNode){
        foreach(auto handle, mHandles)
        {
            std::unique_ptr<MegaNode> nodeToMove(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
            if(nodeToMove)
            {
                QEventLoop moveEventLoop;
                MegaSyncApp->getMegaApi()->moveNode(nodeToMove.get(),rubbishNode.get(), new OnFinishOneShot(MegaSyncApp->getMegaApi(),
                                                                                                            [&moveEventLoop]
                                                                                                            (bool,
                                                                                                            const MegaRequest&,
                                                                                                            const MegaError&)
                {
                    moveEventLoop.quit();
                }));
                moveEventLoop.exec();
            }
        }

        mResult = true;
        mEventLoop.quit();
    };

    auto moveToDateFolder = [this, moveLambda](std::unique_ptr<MegaNode> parentNode, const QString& duplicatedSyncFolderPath)
    {
        QString dateFolder(QDate::currentDate().toString(Qt::DateFormat::ISODate));
        QString dateFolderPath(QString::fromLatin1("%1/%2").arg(duplicatedSyncFolderPath, dateFolder));
        std::unique_ptr<MegaNode> duplicatedRubbishDateNode(MegaSyncApp->getMegaApi()->getNodeByPath(dateFolderPath.toUtf8().constData()));
        if(!duplicatedRubbishDateNode)
        {
            MegaSyncApp->getMegaApi()->createFolder(dateFolder.toUtf8().constData(), parentNode.get(), new OnFinishOneShot(MegaSyncApp->getMegaApi(), this,
                                                      [this, moveLambda]
                                                      (bool isContextValid, const MegaRequest& request,const MegaError& e)
            {
                if(!isContextValid)
                {
                    return;
                }

                if (e.getErrorCode() == MegaError::API_OK)
                {
                    std::unique_ptr<MegaNode> newFolder(MegaSyncApp->getMegaApi()->getNodeByHandle(request.getNodeHandle()));
                    if(newFolder)
                    {
                        moveLambda(std::move(newFolder));
                        return;
                    }
                }

                mEventLoop.quit();
            }));

            if(!mEventLoop.isRunning())
            {
                //In order to execute in synchronously
                mEventLoop.exec();
            }
        }
        else
        {
            moveLambda(std::move(duplicatedRubbishDateNode));
        }
    };

    auto binNode = MegaSyncApp->getRubbishNode();

    QString fullDuplicatedFolderPath(QString::fromLatin1("//bin/%1").arg(binFolderName));

    std::unique_ptr<MegaNode> duplicatedRubbishNode(MegaSyncApp->getMegaApi()->getNodeByPath(fullDuplicatedFolderPath.toUtf8().constData()));
    if(!duplicatedRubbishNode)
    {
        MegaSyncApp->getMegaApi()->createFolder(binFolderName.toUtf8().constData(), binNode.get(), new OnFinishOneShot(MegaSyncApp->getMegaApi(), this,
                                                                                                                                [this, fullDuplicatedFolderPath, moveLambda, moveToDateFolder, addDateFolder]
                                                                                                                                (bool isContextValid, const MegaRequest& request,const MegaError& e)
        {
            if(!isContextValid)
            {
                return;
            }

            if (e.getErrorCode() == MegaError::API_OK)
            {
                std::unique_ptr<MegaNode> newFolder(MegaSyncApp->getMegaApi()->getNodeByHandle(request.getNodeHandle()));
                if(newFolder)
                {
                    if(addDateFolder)
                    {
                        moveToDateFolder(std::move(newFolder),fullDuplicatedFolderPath);
                    }
                    else
                    {
                        moveLambda(std::move(newFolder));
                    }
                    return;
                }
            }

            mEventLoop.quit();
        }));

        //In order to execute in synchronously
        mEventLoop.exec();
    }
    else
    {
        if(addDateFolder)
        {
            moveToDateFolder(std::move(duplicatedRubbishNode),fullDuplicatedFolderPath);
        }
        else
        {
            moveLambda(std::move(duplicatedRubbishNode));
        }
    }

    return mResult;
}

//FOLDER MERGE LOGIC
/*
   1. Detect if there are more than 1 folder with the same name in the name conflict received from the SDK
   2. Get the folder with more files (we can call it the "main" folder) as it is going to be faster moving other folders to this one
   3. Start iterating the rest of folders one by one (we can call them the "secondary" folders:
        a. If the secondary and the main folder have a file with the same name
            i.  If it is identical (same fingerpint), we skip it
            ii. It if is not identical (different fingerprint), I move the secondary folder file with a different name (%1 suffix)
        b. If the secondary folder has a file which is not in the main folder, we move it directly.
        c. If the secondary and the main folder have a folder with the same name:
            i.  We run this algorithm recursively but updating the main and the secondary folders pointer.
        d. If the secondary folder has a folder which is not in the main folder, we move it directly
*/
void CloudFoldersMerge::merge(ActionForDuplicates action)
{

    QStringList itemsBeingRenamed;

    std::unique_ptr<MegaNodeList> folderTargetNodes(MegaSyncApp->getMegaApi()->getChildren(mFolderTarget));
    QMap<QString, int> nodesIndexesByName;
    for(int index = 0; index < folderTargetNodes->size(); ++index)
    {
        auto node(folderTargetNodes->get(index));
        nodesIndexesByName.insertMulti(QString::fromUtf8(node->getName()), index);
    }

    QEventLoop eventLoop;

    std::unique_ptr<MegaNodeList> folderToMergeNodes(MegaSyncApp->getMegaApi()->getChildren(mFolderToMerge));
    for(int index = 0; index < folderToMergeNodes->size(); ++index)
    {
        auto node(folderToMergeNodes->get(index));
        QString nodeName(QString::fromUtf8(node->getName()));
        auto targetIndexes(nodesIndexesByName.values(nodeName));
        if(!targetIndexes.isEmpty())
        {
            bool duplicateFound(false);
            foreach(auto targetIndex, targetIndexes)
            {
                auto targetNode(folderTargetNodes->get(targetIndex));
                if(node->isFile() && targetNode->isFile())
                {
                    auto nodeFp(QString::fromUtf8(node->getFingerprint()));
                    auto targetNodeFp(QString::fromUtf8(targetNode->getFingerprint()));
                    if(nodeFp == targetNodeFp)
                    {
                        duplicateFound = true;
                        break;
                    }
                }
                else if(node->isFolder() && targetNode->isFolder())
                {
                    mDepth++;
                    CloudFoldersMerge folderMerge(targetNode, node);
                    folderMerge.merge(action);
                    mDepth--;
                    std::unique_ptr<MegaNodeList>folderChild(MegaSyncApp->getMegaApi()->getChildren(node));
                    if(action == ActionForDuplicates::IgnoreAndRemove || folderChild->size() == 0)
                    {
                        MegaSyncApp->getMegaApi()->remove(node, new OnFinishOneShot(MegaSyncApp->getMegaApi(),
                                                                                    [&eventLoop]
                                                                                    (bool, const MegaRequest&,const MegaError&)
                        {
                            eventLoop.quit();
                        }));
                        eventLoop.exec();
                    }
                    else
                    {
                        duplicateFound = true;
                    }
                }
            }

            if(!duplicateFound || action == ActionForDuplicates::Rename)
            {
                auto newName = Utilities::getNonDuplicatedNodeName(node, mFolderTarget, nodeName, true, itemsBeingRenamed);
                MegaSyncApp->getMegaApi()->moveNode(node, mFolderTarget,newName.toUtf8().constData(), new OnFinishOneShot(MegaSyncApp->getMegaApi(),
                                                                                                                                [&eventLoop]
                                                                                                                                (bool, const MegaRequest&,const MegaError&)
                {
                    eventLoop.quit();
                }));
                eventLoop.exec();
            }
        }
        else
        {
            MegaSyncApp->getMegaApi()->moveNode(node, mFolderTarget, new OnFinishOneShot(MegaSyncApp->getMegaApi(),
                                                                                                        [&eventLoop]
                                                                                                        (bool, const MegaRequest&,const MegaError&)
            {
                eventLoop.quit();
            }));
            eventLoop.exec();
        }
    }

    //Only done when the recursion finishes
    if(mDepth == 0)
    {
        std::unique_ptr<MegaNodeList>folderChild(MegaSyncApp->getMegaApi()->getChildren(mFolderToMerge));
        if(folderChild->size() != 0)
        {
            MoveToCloudBinUtilities toBin;
            toBin.moveToBin(QList<MegaHandle>() << mFolderToMerge->getHandle(), QLatin1String("FoldersMerge"), true);
        }
        else
        {
            MegaSyncApp->getMegaApi()->remove(mFolderToMerge);
        }
    }
}
