#include "Utilities.h"
#include "control/Preferences.h"

#include <QApplication>
#include <QImageReader>
#include <QDirIterator>
#include <QDesktopServices>
#include <QTextStream>
#include <QDateTime>
#include <iostream>
#include "MegaApplication.h"

#ifndef WIN32
#include "megaapi.h"
#include <utime.h>
#endif

using namespace std;
using namespace mega;

QHash<QString, QString> Utilities::extensionIcons;
QHash<QString, QString> Utilities::languageNames;

void Utilities::initializeExtensions()
{
    extensionIcons[QString::fromLatin1("3ds")] = extensionIcons[QString::fromLatin1("3dm")]  = extensionIcons[QString::fromLatin1("max")] =
                            extensionIcons[QString::fromLatin1("obj")]  = QString::fromLatin1("3D.png");

    extensionIcons[QString::fromLatin1("aep")] = extensionIcons[QString::fromLatin1("aet")]  = QString::fromLatin1("aftereffects.png");

    extensionIcons[QString::fromLatin1("mp3")] = extensionIcons[QString::fromLatin1("wav")]  = extensionIcons[QString::fromLatin1("3ga")]  =
                            extensionIcons[QString::fromLatin1("aif")]  = extensionIcons[QString::fromLatin1("aiff")] =
                            extensionIcons[QString::fromLatin1("flac")] = extensionIcons[QString::fromLatin1("iff")]  =
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
    extensionIcons[QString::fromLatin1("jpg")] = extensionIcons[QString::fromLatin1("jpeg")] = QString::fromLatin1("image.png");
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

QString Utilities::getExtensionPixmap(QString fileName, QString prefix)
{
    if (extensionIcons.isEmpty())
    {
        initializeExtensions();
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

QString Utilities::languageCodeToString(QString code)
{
    if (languageNames.isEmpty())
    {       
        languageNames[QString::fromLatin1("ar")] = QString::fromUtf8("العربية");
        languageNames[QString::fromLatin1("de")] = QString::fromUtf8("Deutsch");  
        languageNames[QString::fromLatin1("en")] = QString::fromUtf8("English");
        languageNames[QString::fromLatin1("es")] = QString::fromUtf8("Español");
        languageNames[QString::fromLatin1("fr")] = QString::fromUtf8("Français");
        languageNames[QString::fromLatin1("id")] = QString::fromUtf8("Bahasa Indonesia");
        languageNames[QString::fromLatin1("it")] = QString::fromUtf8("Italiano");
        languageNames[QString::fromLatin1("ja")] = QString::fromUtf8("日本語");
        languageNames[QString::fromLatin1("ko")] = QString::fromUtf8("한국어");
        languageNames[QString::fromLatin1("nl")] = QString::fromUtf8("Nederlands");
        languageNames[QString::fromLatin1("pl")] = QString::fromUtf8("Polski");
        languageNames[QString::fromLatin1("pt_BR")] = QString::fromUtf8("Português Brasil");
        languageNames[QString::fromLatin1("pt")] = QString::fromUtf8("Português");
        languageNames[QString::fromLatin1("ro")] = QString::fromUtf8("Română");
        languageNames[QString::fromLatin1("ru")] = QString::fromUtf8("Pусский");
        languageNames[QString::fromLatin1("th")] = QString::fromUtf8("ภาษาไทย");
        languageNames[QString::fromLatin1("tl")] = QString::fromUtf8("Tagalog");
        languageNames[QString::fromLatin1("uk")] = QString::fromUtf8("Українська");
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

    }
    return languageNames.value(code);
}

QString Utilities::getExtensionPixmapSmall(QString fileName)
{
    return getExtensionPixmap(fileName, QString::fromLatin1(":/images/small_"));
}

QString Utilities::getExtensionPixmapMedium(QString fileName)
{
    return getExtensionPixmap(fileName, QString::fromLatin1(":/images/drag_"));
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

bool Utilities::verifySyncedFolderLimits(QString path)
{
#ifdef WIN32
    if (path.startsWith(QString::fromLatin1("\\\\?\\")))
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
QString Utilities::getTimeString(long long secs, bool secondPrecision)
{
    int seconds = (int) secs % 60;
    int minutes = (int) ((secs / 60) % 60);
    int hours   = (int) (secs / (60 * 60)) % 24;
    int days = (int)(secs / (60 * 60 * 24));

    int items = 0;
    QString time;

    if (days)
    {
        items++;
        time.append(QString::fromUtf8(" %1 <span style=\"color:#777777; text-decoration:none;\">d</span>").arg(days));
    }

    if (items || hours)
    {
        items++;
        time.append(QString::fromUtf8(" %1 <span style=\"color:#777777; text-decoration:none;\">h</span>").arg(hours));
    }

    if (items == 2)
    {
        time = time.trimmed();
        return time;
    }

    if (items || minutes)
    {
        items++;
        time.append(QString::fromUtf8(" %1 <span style=\"color:#777777; text-decoration:none;\">m</span>").arg(minutes));
    }

    if (items == 2)
    {
        time = time.trimmed();
        return time;
    }

    if (secondPrecision)
    {
        time.append(QString::fromUtf8(" %1 <span style=\"color:#777777; text-decoration:none;\">s</span>").arg(seconds));
    }
    time = time.trimmed();
    return time;
}

QString Utilities::getFinishedTimeString(long long secs)
{
    if (secs < 2)
    {
        return QCoreApplication::translate("Utilities", "just now");
    }
    else if (secs < 60)
    {
        return QCoreApplication::translate("Utilities", "%1 seconds ago").arg(secs);
    }
    else if (secs < 3600)
    {
        int minutes = secs/60;
        if (minutes == 1)
        {
            return QCoreApplication::translate("Utilities", "1 minute ago");
        }
        else
        {
            return QCoreApplication::translate("Utilities", "%1 minutes ago").arg(minutes);
        }
    }
    else if (secs < 86400)
    {
        int hours = secs/3600;
        if (hours == 1)
        {
            return QCoreApplication::translate("Utilities", "1 hour ago");
        }
        else
        {
            return QCoreApplication::translate("Utilities", "%1 hours ago").arg(hours);
        }
    }
    else if (secs < 2592000)
    {
        int days = secs/86400;
        if (days == 1)
        {
            return QCoreApplication::translate("Utilities", "1 day ago");
        }
        else
        {
            return QCoreApplication::translate("Utilities", "%1 days ago").arg(days);
        }
    }
    else if (secs < 31536000)
    {
        int months = secs/2592000;
        if (months == 1)
        {
            return QCoreApplication::translate("Utilities", "1 month ago");
        }
        else
        {
            return QCoreApplication::translate("Utilities", "%1 months ago").arg(months);
        }
    }
    else
    {
        int years = secs/31536000;
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
    unsigned long long KB = 1024;
    unsigned long long MB = 1024 * KB;
    unsigned long long GB = 1024 * MB;
    unsigned long long TB = 1024 * GB;

    QString language = ((MegaApplication*)qApp)->getCurrentLanguageCode();
    QLocale locale(language);
    if (bytes >= TB)
    {
        return locale.toString( ((int)((100 * bytes) / TB))/100.0) + QString::fromLatin1(" ") + QCoreApplication::translate("Utilities", "TB");
    }

    if (bytes >= GB)
    {
        return locale.toString( ((int)((100 * bytes) / GB))/100.0) + QString::fromLatin1(" ") + QCoreApplication::translate("Utilities", "GB");
    }

    if (bytes >= MB)
    {
        return locale.toString( ((int)((100 * bytes) / MB))/100.0) + QString::fromLatin1(" ") + QCoreApplication::translate("Utilities", "MB");
    }

    if (bytes >= KB)
    {
        return locale.toString( ((int)((100 * bytes) / KB))/100.0) + QString::fromLatin1(" ") + QCoreApplication::translate("Utilities", "KB");
    }

    return locale.toString(bytes) + QString::fromLatin1(" bytes");
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

