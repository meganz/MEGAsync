#include "CommonUtils.h"
#include "Utils.h"

#include <QImageReader>

//This force the compiler to instantiate the necessary methods
//and allows having the implementation of this class in this separate .cpp file
//This function doesn't need to be called, it's only to make the linker happy
void Instantiate_Template_Methods()
{
    Utils::getSizeString(0);
    Utils::verifySyncedFolderLimits("");
    Utils::getExtensionPixmapSmall("");
    Utils::getExtensionPixmapMedium("");
    Utils::createThumbnail("",0);
}

template <class T>
void CommonUtils<T>::initializeExtensions()
{
    extensionIcons["3ds"] = extensionIcons["3dm"]  = extensionIcons["max"] =
                            extensionIcons["obj"]  = "3D.png";

    extensionIcons["aep"] = extensionIcons["aet"]  = "aftereffects.png";

    extensionIcons["mp3"] = extensionIcons["wav"]  = extensionIcons["3ga"]  =
                            extensionIcons["aif"]  = extensionIcons["aiff"] =
                            extensionIcons["flac"] = extensionIcons["iff"]  =
                            extensionIcons["m4a"]  = extensionIcons["wma"]  =  "audio.png";

    extensionIcons["dxf"] = extensionIcons["dwg"] =  "cad.png";


    extensionIcons["zip"] = extensionIcons["rar"] = extensionIcons["tgz"]  =
                            extensionIcons["gz"]  = extensionIcons["bz2"]  =
                            extensionIcons["tbz"] = extensionIcons["tar"]  =
                            extensionIcons["7z"]  = extensionIcons["sitx"] =  "compressed.png";

    extensionIcons["sql"] = extensionIcons["accdb"] = extensionIcons["db"]  =
                            extensionIcons["dbf"]  = extensionIcons["mdb"]  =
                            extensionIcons["pdb"] = "database.png";

    extensionIcons["dwt"] = "dreamweaver.png";

    extensionIcons["xls"] = extensionIcons["xlsx"] = extensionIcons["xlt"]  =
                            extensionIcons["xltm"]  = "excel.png";

    extensionIcons["exe"] = extensionIcons["com"] = extensionIcons["bin"]  =
                            extensionIcons["apk"]  = extensionIcons["app"]  =
                             extensionIcons["msi"]  = extensionIcons["cmd"]  =
                            extensionIcons["gadget"] = "executable.png";

    extensionIcons["as"] = extensionIcons["ascs"] =
               extensionIcons["asc"]  = "fla_lang.png";
    extensionIcons["fla"] = "flash.png";
    extensionIcons["flv"] = "flash_video.png";

    extensionIcons["fnt"] = extensionIcons["otf"] = extensionIcons["ttf"]  =
                            extensionIcons["fon"]  = "font.png";

    extensionIcons["gpx"] = extensionIcons["kml"] = extensionIcons["kmz"]  = "gis.png";


    extensionIcons["gif"] = extensionIcons["tiff"]  = extensionIcons["tif"]  =
                            extensionIcons["bmp"]  = extensionIcons["png"] =
                            extensionIcons["tga"]  = "graphic.png";

    extensionIcons["html"] = extensionIcons["htm"]  = extensionIcons["dhtml"]  =
                            extensionIcons["xhtml"]  = "html.png";

    extensionIcons["ai"] = extensionIcons["ait"] = "illustrator.png";
    extensionIcons["jpg"] = extensionIcons["jpeg"] = "image.png";
    extensionIcons["indd"] = "indesign.png";

    extensionIcons["jar"] = extensionIcons["java"]  = extensionIcons["class"]  = "java.png";

    extensionIcons["mid"] = extensionIcons["midi"] = "midi.png";
    extensionIcons["pdf"] = "pdf.png";
    extensionIcons["abr"] = extensionIcons["psb"]  = extensionIcons["psd"]  =
                            "photoshop.png";

    extensionIcons["pls"] = extensionIcons["m3u"]  = extensionIcons["asx"]  =
                            "playlist.png";
    extensionIcons["pcast"] = "podcast.png";

    extensionIcons["pps"] = extensionIcons["ppt"]  = extensionIcons["pptx"] = "powerpoint.png";

    extensionIcons["prproj"] = extensionIcons["ppj"]  = "premiere.png";

    extensionIcons["3fr"] = extensionIcons["arw"]  = extensionIcons["bay"]  =
                            extensionIcons["cr2"]  = extensionIcons["dcr"] =
                            extensionIcons["dng"] = extensionIcons["fff"]  =
                            extensionIcons["mef"] = extensionIcons["mrw"]  =
                            extensionIcons["nef"] = extensionIcons["pef"]  =
                            extensionIcons["rw2"] = extensionIcons["srf"]  =
                            extensionIcons["orf"] = extensionIcons["rwl"]  =
                            "raw.png";

    extensionIcons["rm"] = extensionIcons["ra"]  = extensionIcons["ram"]  =
                            "real_audio.png";

    extensionIcons["sh"] = extensionIcons["c"]  = extensionIcons["cc"]  =
                            extensionIcons["cpp"]  = extensionIcons["cxx"] =
                            extensionIcons["h"] = extensionIcons["hpp"]  =
                            extensionIcons["dll"] = "source_code.png";

    extensionIcons["ods"]  = extensionIcons["ots"]  =
                            extensionIcons["gsheet"]  = extensionIcons["nb"] =
                            extensionIcons["xlr"] = extensionIcons["numbers"]  =
                            "spreadsheet.png";

    extensionIcons["swf"] = "swf.png";
    extensionIcons["torrent"] = "torrent.png";

    extensionIcons["txt"] = extensionIcons["rtf"]  = extensionIcons["ans"]  =
                            extensionIcons["ascii"]  = extensionIcons["log"] =
                            extensionIcons["odt"] = extensionIcons["wpd"]  =
                            "text.png";

    extensionIcons["vcf"] = "vcard.png";
    extensionIcons["svgz"]  = extensionIcons["svg"]  =
                            extensionIcons["cdr"]  = extensionIcons["eps"] =
                            "vector.png";


     extensionIcons["mkv"]  = extensionIcons["webm"]  =
                            extensionIcons["avi"]  = extensionIcons["mp4"] =
                            extensionIcons["m4v"] = extensionIcons["mpg"]  =
                            extensionIcons["mpeg"] = extensionIcons["mov"]  =
                            extensionIcons["3g2"] = extensionIcons["3gp"]  =
                            extensionIcons["asf"] = extensionIcons["wmv"]  =
                            "video.png";


     extensionIcons["srt"] = "video_subtitles.png";
     extensionIcons["vob"] = "video_vob.png";

     extensionIcons["html"]  = extensionIcons["xml"] = extensionIcons["shtml"]  =
                            extensionIcons["dhtml"] = extensionIcons["js"] =
                            extensionIcons["css"]  = "web_data.png";

     extensionIcons["php"]  = extensionIcons["php3"]  =
                            extensionIcons["php4"]  = extensionIcons["php5"] =
                            extensionIcons["phtml"] = extensionIcons["inc"]  =
                            extensionIcons["asp"] = extensionIcons["pl"]  =
                            extensionIcons["cgi"] = extensionIcons["py"]  =
                            "web_lang.png";


     extensionIcons["doc"]  = extensionIcons["docx"] = extensionIcons["dotx"]  =
                            extensionIcons["wps"] = "word.png";
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
        return QPixmap(prefix + "generic.png");
}

template <class T>
QPixmap CommonUtils<T>::getExtensionPixmapSmall(QString fileName)
{
    return getExtensionPixmap(fileName, "://images/small_");
}

template <class T>
QPixmap CommonUtils<T>::getExtensionPixmapMedium(QString fileName)
{
    return getExtensionPixmap(fileName, "://images/drag_");
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
        return QString::number( ((int)((100 * bytes) / TB))/100.0) + " TB";

    if(bytes > GB)
        return QString::number( ((int)((100 * bytes) / GB))/100.0) + " GB";

    if(bytes > MB)
        return QString::number( ((int)((100 * bytes) / MB))/100.0) + " MB";

    if(bytes > KB)
        return QString::number( ((int)((100 * bytes) / KB))/100.0) + " KB";

    return QString::number(bytes) + " bytes";
}


