#include "CommonUtils.h"
#include "Utils.h"

#include <QImageReader>

//This force the compiler to instantiate the necessary methods
//and allows having the implementation of this class in this separate .cpp file
//This function doesn't need to be called, it's only to make the linker happy
void Instantiate_Template_Methods()
{
    Utils::getSizeString(0);
    Utils::verifySyncedFolderLimits(QString::fromAscii(""));
    Utils::getExtensionPixmapSmall(QString::fromAscii(""));
    Utils::getExtensionPixmapMedium(QString::fromAscii(""));
    Utils::createThumbnail(QString::fromAscii(""),0);
}

template <class T>
void CommonUtils<T>::initializeExtensions()
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
                            extensionIcons[QString::fromAscii("pdb")] = QString::fromAscii("database.png");

    extensionIcons[QString::fromAscii("dwt")] = QString::fromAscii("dreamweaver.png");

    extensionIcons[QString::fromAscii("xls")] = extensionIcons[QString::fromAscii("xlsx")] = extensionIcons[QString::fromAscii("xlt")]  =
                            extensionIcons[QString::fromAscii("xltm")]  = QString::fromAscii("excel.png");

    extensionIcons[QString::fromAscii("exe")] = extensionIcons[QString::fromAscii("com")] = extensionIcons[QString::fromAscii("bin")]  =
                            extensionIcons[QString::fromAscii("apk")]  = extensionIcons[QString::fromAscii("app")]  =
                             extensionIcons[QString::fromAscii("msi")]  = extensionIcons[QString::fromAscii("cmd")]  =
                            extensionIcons[QString::fromAscii("gadget")] = QString::fromAscii("executable.png");

    extensionIcons[QString::fromAscii("as")] = extensionIcons[QString::fromAscii("ascs")] =
               extensionIcons[QString::fromAscii("asc")]  = QString::fromAscii("fla_lang.png");
    extensionIcons[QString::fromAscii("fla")] = QString::fromAscii("flash.png");
    extensionIcons[QString::fromAscii("flv")] = QString::fromAscii("flash_video.png");

    extensionIcons[QString::fromAscii("fnt")] = extensionIcons[QString::fromAscii("otf")] = extensionIcons[QString::fromAscii("ttf")]  =
                            extensionIcons[QString::fromAscii("fon")]  = QString::fromAscii("font.png");

    extensionIcons[QString::fromAscii("gpx")] = extensionIcons[QString::fromAscii("kml")] = extensionIcons[QString::fromAscii("kmz")]  = QString::fromAscii("gis.png");


    extensionIcons[QString::fromAscii("gif")] = extensionIcons[QString::fromAscii("tiff")]  = extensionIcons[QString::fromAscii("tif")]  =
                            extensionIcons[QString::fromAscii("bmp")]  = extensionIcons[QString::fromAscii("png")] =
                            extensionIcons[QString::fromAscii("tga")]  = QString::fromAscii("graphic.png");

    extensionIcons[QString::fromAscii("html")] = extensionIcons[QString::fromAscii("htm")]  = extensionIcons[QString::fromAscii("dhtml")]  =
                            extensionIcons[QString::fromAscii("xhtml")]  = QString::fromAscii("html.png");

    extensionIcons[QString::fromAscii("ai")] = extensionIcons[QString::fromAscii("ait")] = QString::fromAscii("illustrator.png");
    extensionIcons[QString::fromAscii("jpg")] = extensionIcons[QString::fromAscii("jpeg")] = QString::fromAscii("image.png");
    extensionIcons[QString::fromAscii("indd")] = QString::fromAscii("indesign.png");

    extensionIcons[QString::fromAscii("jar")] = extensionIcons[QString::fromAscii("java")]  = extensionIcons[QString::fromAscii("class")]  = QString::fromAscii("java.png");

    extensionIcons[QString::fromAscii("mid")] = extensionIcons[QString::fromAscii("midi")] = QString::fromAscii("midi.png");
    extensionIcons[QString::fromAscii("pdf")] = QString::fromAscii("pdf.png");
    extensionIcons[QString::fromAscii("abr")] = extensionIcons[QString::fromAscii("psb")]  = extensionIcons[QString::fromAscii("psd")]  =
                            QString::fromAscii("photoshop.png");

    extensionIcons[QString::fromAscii("pls")] = extensionIcons[QString::fromAscii("m3u")]  = extensionIcons[QString::fromAscii("asx")]  =
                            QString::fromAscii("playlist.png");
    extensionIcons[QString::fromAscii("pcast")] = QString::fromAscii("podcast.png");

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

    extensionIcons[QString::fromAscii("rm")] = extensionIcons[QString::fromAscii("ra")]  = extensionIcons[QString::fromAscii("ram")]  =
                            QString::fromAscii("real_audio.png");

    extensionIcons[QString::fromAscii("sh")] = extensionIcons[QString::fromAscii("c")]  = extensionIcons[QString::fromAscii("cc")]  =
                            extensionIcons[QString::fromAscii("cpp")]  = extensionIcons[QString::fromAscii("cxx")] =
                            extensionIcons[QString::fromAscii("h")] = extensionIcons[QString::fromAscii("hpp")]  =
                            extensionIcons[QString::fromAscii("dll")] = QString::fromAscii("source_code.png");

    extensionIcons[QString::fromAscii("ods")]  = extensionIcons[QString::fromAscii("ots")]  =
                            extensionIcons[QString::fromAscii("gsheet")]  = extensionIcons[QString::fromAscii("nb")] =
                            extensionIcons[QString::fromAscii("xlr")] = extensionIcons[QString::fromAscii("numbers")]  =
                            QString::fromAscii("spreadsheet.png");

    extensionIcons[QString::fromAscii("swf")] = QString::fromAscii("swf.png");
    extensionIcons[QString::fromAscii("torrent")] = QString::fromAscii("torrent.png");

    extensionIcons[QString::fromAscii("txt")] = extensionIcons[QString::fromAscii("rtf")]  = extensionIcons[QString::fromAscii("ans")]  =
                            extensionIcons[QString::fromAscii("ascii")]  = extensionIcons[QString::fromAscii("log")] =
                            extensionIcons[QString::fromAscii("odt")] = extensionIcons[QString::fromAscii("wpd")]  =
                            QString::fromAscii("text.png");

    extensionIcons[QString::fromAscii("vcf")] = QString::fromAscii("vcard.png");
    extensionIcons[QString::fromAscii("svgz")]  = extensionIcons[QString::fromAscii("svg")]  =
                            extensionIcons[QString::fromAscii("cdr")]  = extensionIcons[QString::fromAscii("eps")] =
                            QString::fromAscii("vector.png");


     extensionIcons[QString::fromAscii("mkv")]  = extensionIcons[QString::fromAscii("webm")]  =
                            extensionIcons[QString::fromAscii("avi")]  = extensionIcons[QString::fromAscii("mp4")] =
                            extensionIcons[QString::fromAscii("m4v")] = extensionIcons[QString::fromAscii("mpg")]  =
                            extensionIcons[QString::fromAscii("mpeg")] = extensionIcons[QString::fromAscii("mov")]  =
                            extensionIcons[QString::fromAscii("3g2")] = extensionIcons[QString::fromAscii("3gp")]  =
                            extensionIcons[QString::fromAscii("asf")] = extensionIcons[QString::fromAscii("wmv")]  =
                            QString::fromAscii("video.png");


     extensionIcons[QString::fromAscii("srt")] = QString::fromAscii("video_subtitles.png");
     extensionIcons[QString::fromAscii("vob")] = QString::fromAscii("video_vob.png");

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
}

template <class T>
void CommonUtils<T>::countFilesAndFolders(QString path, long *numFiles, long *numFolders, long fileLimit, long folderLimit)
{
    QFileInfo baseDir(path);
    if(!baseDir.exists() || !baseDir.isDir()) return;
    if((((*numFolders) > folderLimit)) ||
       (((*numFiles) > fileLimit)))
        return;

    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for(int i=0; i<entries.size(); i++)
    {
        QFileInfo info = entries[i];
        if(info.isFile()) (*numFiles)++;
        else if(info.isDir())
        {
            countFilesAndFolders(info.absoluteFilePath(), numFiles, numFolders, fileLimit, folderLimit);
            (*numFolders)++;
        }
    }
}

template <class T>
QPixmap CommonUtils<T>::getExtensionPixmap(QString fileName, QString prefix)
{
    if(extensionIcons.isEmpty()) initializeExtensions();

    QFileInfo f(fileName);
    if(extensionIcons.contains(f.suffix().toLower()))
        return QPixmap(QString(extensionIcons[f.suffix().toLower()]).insert(0, prefix));
    else
        return QPixmap(prefix + QString::fromAscii("generic.png"));
}

template <class T>
QPixmap CommonUtils<T>::getExtensionPixmapSmall(QString fileName)
{
    return getExtensionPixmap(fileName, QString::fromAscii("://images/small_"));
}

template <class T>
QPixmap CommonUtils<T>::getExtensionPixmapMedium(QString fileName)
{
    return getExtensionPixmap(fileName, QString::fromAscii("://images/drag_"));
}

template <class T>
QImage CommonUtils<T>::createThumbnail(QString imagePath, int size)
{
    if(QImageReader::imageFormat(imagePath).isEmpty()) return QImage();

    QImage image(imagePath);
    int w = image.width();
    int h = image.height();
    if(!(w >= 20 && w >= 20)) return QImage();

    if (w < h)
    {
        h = h*size/w;
        w = size;
    }
    else
    {
        w = w*size/h;
        h = size;
    }

    return image.scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)
             .copy((w-size)/2,(h-size)/3,size,size);
}

template <class T>
bool CommonUtils<T>::verifySyncedFolderLimits(QString path)
{
    long numFiles = 0;
    long numFolders = 0;
    QFileInfo info(path);
    if(!info.isDir()) return true;
    countFilesAndFolders(path, &numFiles, &numFolders,
                         Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER,
                         Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER);

    if(((numFolders > Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER)) ||
       ((numFiles > Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER)))
        return false;
    return true;
}

template <class T>
QString CommonUtils<T>::getSizeString(unsigned long long bytes)
{
    unsigned long long KB = 1024;
    unsigned long long MB = 1024 * KB;
    unsigned long long GB = 1024 * MB;
    unsigned long long TB = 1024 * GB;

    if(bytes > TB)
        return QString::number( ((int)((100 * bytes) / TB))/100.0) + QString::fromAscii(" TB");

    if(bytes > GB)
        return QString::number( ((int)((100 * bytes) / GB))/100.0) + QString::fromAscii(" GB");

    if(bytes > MB)
        return QString::number( ((int)((100 * bytes) / MB))/100.0) + QString::fromAscii(" MB");

    if(bytes > KB)
        return QString::number( ((int)((100 * bytes) / KB))/100.0) + QString::fromAscii(" KB");

    return QString::number(bytes) + QString::fromAscii(" bytes");
}


