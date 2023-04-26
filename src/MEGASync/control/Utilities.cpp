#include "Utilities.h"
#include "control/Preferences.h"

#include <QApplication>
#include <QImageReader>
#include <QDirIterator>
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

const QString Utilities::SUPPORT_URL = QString::fromUtf8("https://mega.nz/contact");
const QString Utilities::BACKUP_CENTER_URL = QString::fromLatin1("mega://#fm/devices");

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
    extensionIcons[QString::fromAscii("3ds")] = extensionIcons[QString::fromAscii("3dm")]  = extensionIcons[QString::fromAscii("max")] =
                            extensionIcons[QString::fromAscii("obj")]  = QString::fromAscii("3D.png");

    extensionIcons[QString::fromAscii("aep")] = extensionIcons[QString::fromAscii("aet")]  = QString::fromAscii("aftereffects.png");

    extensionIcons[QString::fromAscii("mp3")] = extensionIcons[QString::fromAscii("wav")]  = extensionIcons[QString::fromAscii("3ga")]  =
                            extensionIcons[QString::fromAscii("aif")]  = extensionIcons[QString::fromAscii("aiff")] =
                            extensionIcons[QString::fromAscii("flac")] = extensionIcons[QString::fromAscii("iff")]  = extensionIcons[QString::fromAscii("ogg")] =
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
    extensionIcons[QString::fromAscii("jpg")] = extensionIcons[QString::fromAscii("jpeg")] = extensionIcons[QString::fromAscii("heic")] = QString::fromAscii("image.png");
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
                            extensionIcons[QString::fromAscii("ari")] = extensionIcons[QString::fromAscii("braw")]  =
                            extensionIcons[QString::fromAscii("crw")] = extensionIcons[QString::fromAscii("cr3")]  =
                            extensionIcons[QString::fromAscii("cap")] =
                            extensionIcons[QString::fromAscii("dcs")] = extensionIcons[QString::fromAscii("drf")]  =
                            extensionIcons[QString::fromAscii("eip")] = extensionIcons[QString::fromAscii("erf")]  =
                            extensionIcons[QString::fromAscii("gpr")] = extensionIcons[QString::fromAscii("iiq")]  =
                            extensionIcons[QString::fromAscii("k25")] = extensionIcons[QString::fromAscii("kdc")]  =
                            extensionIcons[QString::fromAscii("mdc")] = extensionIcons[QString::fromAscii("mos")]  =
                            extensionIcons[QString::fromAscii("nrw")] = extensionIcons[QString::fromAscii("obm")]  =
                            extensionIcons[QString::fromAscii("ptx")] = extensionIcons[QString::fromAscii("pxn")]  =
                            extensionIcons[QString::fromAscii("r3d")] = extensionIcons[QString::fromAscii("raf")]  =
                            extensionIcons[QString::fromAscii("raw")] = extensionIcons[QString::fromAscii("rwz")]  =
                            extensionIcons[QString::fromAscii("sr2")] = extensionIcons[QString::fromAscii("srw")]  =
                            extensionIcons[QString::fromAscii("tif")] = extensionIcons[QString::fromAscii("x3f")]  =
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

QString Utilities::getFinishedTimeString(long long secs)
{
    const int SECS_IN_1_MINUTE = 60;
    const int SECS_IN_1_HOUR = 3600;
    const int SECS_IN_1_DAY = 86400;
    const int SECS_IN_1_MONTH = 2592000;
    const int SECS_IN_1_YEAR = 31536000;

    if (secs < 2)
    {
        return QCoreApplication::translate("Utilities", "just now");
    }
    else if (secs < SECS_IN_1_MINUTE)
    {
        const int secsAsInt = static_cast<int>(secs);
        return QCoreApplication::translate("Utilities", "%n second ago", "", secsAsInt);
    }
    else if (secs < SECS_IN_1_HOUR)
    {
        const int minutes = static_cast<int>(secs/SECS_IN_1_MINUTE);
        return QCoreApplication::translate("Utilities", "%n minute ago", "", minutes);
    }
    else if (secs < SECS_IN_1_DAY)
    {
        const int hours = static_cast<int>(secs/SECS_IN_1_HOUR);
        return QCoreApplication::translate("Utilities", "%n hour ago", "", hours);
    }
    else if (secs < SECS_IN_1_MONTH)
    {
        const int days = static_cast<int>(secs/SECS_IN_1_DAY);
        return QCoreApplication::translate("Utilities", "%n day ago", "", days);
    }
    else if (secs < SECS_IN_1_YEAR)
    {
        const int months = static_cast<int>(secs/SECS_IN_1_MONTH);
        return QCoreApplication::translate("Utilities", "%n month ago", "", months);
    }
    // We might not need century precision... give years.
    else
    {
        const int years = static_cast<int>(secs/SECS_IN_1_YEAR);
        return QCoreApplication::translate("Utilities", "%n year ago", "", years);
    }
}

QString Utilities::getSizeString(unsigned long long bytes)
{
    QString language = ((MegaApplication*)qApp)->getCurrentLanguageCode();
    QLocale locale(language);
    
    if (bytes >= TB)
    {
        return locale.toString(toDoubleInUnit(bytes, TB)) + QString::fromAscii(" ")
                + QCoreApplication::translate("Utilities", "TB");
    }

    if (bytes >= GB)
    {
        return locale.toString(toDoubleInUnit(bytes, GB)) + QString::fromAscii(" ")
                + QCoreApplication::translate("Utilities", "GB");
    }

    if (bytes >= MB)
    {
        return locale.toString(toDoubleInUnit(bytes, MB)) + QString::fromAscii(" ")
                + QCoreApplication::translate("Utilities", "MB");
    }

    if (bytes >= KB)
    {
        return locale.toString(toDoubleInUnit(bytes, KB)) + QString::fromAscii(" ")
                + QCoreApplication::translate("Utilities", "KB");
    }

    return locale.toString(toDoubleInUnit(bytes, 1)) + QString::fromAscii(" ")
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
        firstPath = path1;
        secondPath = path2;
    }
    else
    {
        firstPath = path2;
        secondPath = path1;
    }

    QString separator = cloudPaths ? QLatin1String("/") : QString(QDir::separator());
    if(cloudPaths)
    {
        firstPath.append(separator);
        secondPath.append(separator);
    }

    int index = 1;
    while (firstPath.mid(0, index) == secondPath.mid(0, index))
    {
        ret = firstPath.mid(0, index);
        index++;
    }

    if(!ret.endsWith(separator))
    {
        auto splittedPath = ret.split(separator);
        if(!splittedPath.isEmpty())
        {
            ret.remove(ret.lastIndexOf(separator) + 1,splittedPath.last().length());
        }
    }
    else if(cloudPaths)
    {
        ret = ret.remove(ret.length() - 1, 1);
    }

    return ret;
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

WrappedNode::WrappedNode(TransferOrigin from, MegaNode *node)
    : mTransfersFrom(from), mNode(node)
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
