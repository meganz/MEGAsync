/**
 * @file gfxqt.cpp
 * @brief Graphics layer using QT
 *
 * (c) 2014 by Mega Limited, Wellsford, New Zealand
 * EXIF related functions are based on http://www.sentex.net/~mwandel/jhead/
 * Rewritten and published in public domain like
 * the original code by http://imonad.com
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#include "mega.h"
#include "mega/gfx/qt.h"

namespace mega {

/************* EXIF STUFF **************/
#define M_SOI   0xD8          // Start Of Image (beginning of datastream)
#define M_SOS   0xDA          // Start Of Scan (begins compressed data)
#define M_EOI   0xD9          // End Of Image (end of datastream)
#define M_EXIF  0xE1          // Exif marker.  Also used for XMP data!

#define NUM_FORMATS   12
#define FMT_BYTE       1
#define FMT_STRING     2
#define FMT_USHORT     3
#define FMT_ULONG      4
#define FMT_URATIONAL  5
#define FMT_SBYTE      6
#define FMT_UNDEFINED  7
#define FMT_SSHORT     8
#define FMT_SLONG      9
#define FMT_SRATIONAL 10
#define FMT_SINGLE    11
#define FMT_DOUBLE    12

#define TAG_ORIENTATION        0x0112
#define TAG_INTEROP_OFFSET     0xA005
#define TAG_EXIF_OFFSET        0x8769

#define DIR_ENTRY_ADDR(Start, Entry) (Start+2+12*(Entry))

const int BytesPerFormat[] = {0,1,1,2,4,8,1,1,2,4,8,4,8};

//--------------------------------------------------------------------------
// Parse the marker stream until SOS or EOI is seen;
//--------------------------------------------------------------------------
int GfxProcQT::getExifOrientation(QString &filePath)
{
    QByteArray *data;
    uint8_t c;
    bool ok;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return -1;
    ok = file.getChar((char *)&c);
    if((c != 0xFF) || !ok) return -1;

    ok = file.getChar((char *)&c);
    if((c != M_SOI) || !ok) return -1;

    for(;;)
    {
        int itemlen;
        int prev = 0;
        uint8_t marker = 0;
        uint8_t ll, lh;

        for(int i=0;;i++)
        {
            ok = file.getChar((char *)&marker);
            if(!ok) return -1;

            if ((marker != 0xFF) && (prev == 0xFF))
                break;

            prev = marker;
        }

        // Read the length of the section.
        ok = file.getChar((char *)&lh);
        if(!ok) return -1;

        ok = file.getChar((char *)&ll);
        if(!ok) return -1;

        itemlen = (lh << 8) | ll;
        if (itemlen < 2) return -1;

        data = new QByteArray(file.read(itemlen-2)); // Read the whole section.
        if(data->size() != (itemlen-2))
        {
            delete data;
            return -1;
        }

        switch(marker)
        {
            case M_SOS:   // stop before hitting compressed data
            case M_EOI:   // in case it's a tables-only JPEG stream
                delete data;
                return -1;
            case M_EXIF:
                if(data->left(4) == "Exif")
                {
                    int orientation = processEXIF(data, itemlen);
                    if((orientation >= 0) && (orientation <= 8))
                    {
                        delete data;
                        return orientation;
                    }
                }
            default:
                // Skip any other sections.
                delete data;
                break;
        }
    }

    return -1;
}

// Convert a 16 bit unsigned value from file's native byte order
int Get16u(const void * Short, int MotorolaOrder){
    if (MotorolaOrder){
        return (((uchar *)Short)[0] << 8) | ((uchar *)Short)[1];
    }else{
        return (((uchar *)Short)[1] << 8) | ((uchar *)Short)[0];
    }
}

// Convert a 32 bit signed value from file's native byte order
int Get32s(const void * Long, int MotorolaOrder){
    if (MotorolaOrder){
        return  ((( char *)Long)[0] << 24) | (((uchar *)Long)[1] << 16)
              | (((uchar *)Long)[2] << 8 ) | (((uchar *)Long)[3] << 0 );
    }else{
        return  ((( char *)Long)[3] << 24) | (((uchar *)Long)[2] << 16)
              | (((uchar *)Long)[1] << 8 ) | (((uchar *)Long)[0] << 0 );
    }
}

// Convert a 32 bit unsigned value from file's native byte order
unsigned Get32u(const void * Long, int MotorolaOrder){
    return (unsigned)Get32s(Long, MotorolaOrder) & 0xffffffff;
}

// Evaluate number, be it int, rational, or float from directory.
double ConvertAnyFormat(const void * ValuePtr, int Format, int MotorolaOrder){
     double Value;
     Value = 0;

     switch(Format){
         case FMT_SBYTE:     Value = *(signed char *)ValuePtr;  break;
         case FMT_BYTE:      Value = *(uchar *)ValuePtr;        break;

         case FMT_USHORT:    Value = Get16u(ValuePtr, MotorolaOrder);          break;
         case FMT_ULONG:     Value = Get32u(ValuePtr, MotorolaOrder);          break;

         case FMT_URATIONAL:
         case FMT_SRATIONAL:
             {
                 int Num, Den;
                 Num = Get32s(ValuePtr, MotorolaOrder);
                 Den = Get32s(4+(char *)ValuePtr, MotorolaOrder);
                 if (Den == 0){
                     Value = 0;
                 }else{
                     Value = (double)Num/Den;
                 }
                 break;
             }

         case FMT_SSHORT:    Value = (signed short)Get16u(ValuePtr, MotorolaOrder);  break;
         case FMT_SLONG:     Value = Get32s(ValuePtr, MotorolaOrder);                break;

         // Not sure if this is correct (never seen float used in Exif format)
         case FMT_SINGLE:    Value = (double)*(float *)ValuePtr;      break;
         case FMT_DOUBLE:    Value = *(double *)ValuePtr;             break;

         default: Value = 100;// Illegal format code


     }
     return Value;
}

// Process one of the nested EXIF directories.
int GfxProcQT::processEXIFDir(const char *DirStart, const char *OffsetBase, uint32_t exifSize, uint32_t nesting, int MotorolaOrder){
    int numDirEntries;
    int orientation;

    if(nesting>4) return -1; // Maximum Exif directory nesting exceeded (corrupt Exif header)

    numDirEntries = Get16u(DirStart, MotorolaOrder);
    for (int de=0; de<numDirEntries; de++)
    {
        int Tag, Format, Components;
        const char * DirEntry;
        const char * ValuePtr;
        int ByteCount;

        DirEntry = DIR_ENTRY_ADDR(DirStart, de);
        Tag        = Get16u(DirEntry,   MotorolaOrder);
        Format     = Get16u(DirEntry+2, MotorolaOrder);
        Components = Get32u(DirEntry+4, MotorolaOrder);

        if(Format-1 >= NUM_FORMATS) continue; // (-1) catches illegal zero case as unsigned underflows to positive large.
        if((unsigned)Components > 0x10000) continue; // Too many components

        ByteCount = Components * BytesPerFormat[Format];

        if (ByteCount > 4){ // If its bigger than 4 bytes, the dir entry contains an offset.
            unsigned OffsetVal = Get32u(DirEntry+8, MotorolaOrder);
            if (OffsetVal+ByteCount > exifSize) continue; // Bogus pointer offset and / or bytecount value
            ValuePtr = OffsetBase+OffsetVal;
        }else{ // 4 bytes or less and value is in the dir entry itself
            ValuePtr = DirEntry+8;
        }

        // Extract useful components of tag
        switch(Tag){
            case TAG_ORIENTATION:
                orientation = (int)ConvertAnyFormat(ValuePtr, Format, MotorolaOrder);
                if (orientation >= 0 || orientation <= 8)
                    return orientation;
                break;

            case TAG_EXIF_OFFSET:
            case TAG_INTEROP_OFFSET:
                const char * SubdirStart;
                SubdirStart = OffsetBase + Get32u(ValuePtr, MotorolaOrder);
                if(!(SubdirStart < OffsetBase || SubdirStart > OffsetBase+ exifSize))
                {
                    orientation = processEXIFDir(SubdirStart, OffsetBase, exifSize, nesting+1, MotorolaOrder);
                    if (orientation >= 0 || orientation <= 8)
                        return orientation;
                }
                continue;
                break;

            default:
                // Skip any other sections.
                break;
        }
    }

    return -1;
}

// Process a EXIF marker
// Describes all the drivel that most digital cameras include...
int GfxProcQT::processEXIF(QByteArray *data, int itemlen){
    int MotorolaOrder = 0;
    if(data->mid(6,2) == "II") MotorolaOrder = 0;
    else if(data->mid(6,2) == "MM") MotorolaOrder = 1;
    else return -1;

    // get first offset
    QByteArray ttt(data->mid(10,4));
    const char *ttt2 = ttt.constData();
    uint32_t FirstOffset;
    FirstOffset = Get32u(ttt2, MotorolaOrder);

    if (FirstOffset < 8 || FirstOffset > 16){
            if (FirstOffset < 16 || int(FirstOffset) > itemlen-16)  return -1;  // invalid offset for first Exif IFD value ;
    }

    const char *dirStart = data->constData();
    const char *offsetBase = data->constData();

    dirStart   += 6 + FirstOffset;
    offsetBase += 6;

    // First directory starts 16 bytes in.  All offset are relative to 8 bytes in.
    return processEXIFDir(dirStart, offsetBase, itemlen-8, 0, MotorolaOrder);
}
/************* END OF EXIF STUFF **************/


/*GfxProc implementation*/

bool GfxProcQT::isgfx(string* name)
{
    // FreeImage sometimes crashes if fed with something that appears to be an image, but isn't,
    // so we pre-screen by filename to reduce the odds
    size_t p = name->find_last_of('.');

    if (!(p + 1))
    {
        return false;
    }

    string ext(*name,p);

    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    char* ptr =
            strstr((char*) ".jpg.png.bmp.tif.tiff.jpeg.cut.dds.exr.g3.gif.hdr.ico.iff.ilbm"
            ".jbig.jng.jif.koala.pcd.mng.pcx.pbm.pgm.ppm.pfm.pict.pic.pct.pds.raw.3fr.ari"
            ".arw.bay.crw.cr2.cap.dcs.dcr.dng.drf.eip.erf.fff.iiq.k25.kdc.mdc.mef.mos.mrw"
            ".nef.nrw.obm.orf.pef.ptx.pxn.r3d.raf.raw.rwl.rw2.rwz.sr2.srf.srw.x3f.ras.tga"
            ".xbm.xpm.jp2.j2k.jpf.jpx.", ext.c_str());

    return ptr && ptr[ext.size()] == '.';
}

bool GfxProcQT::readbitmap(FileAccess*, string* localname, int)
{
#ifdef _WIN32
    localname->append("", 1);
    QString imagePath = QString::fromWCharArray((wchar_t *)localname->c_str());
    if(imagePath.startsWith(QString::fromAscii("\\\\?\\")))
        imagePath = imagePath.mid(4);
#else
    QString imagePath = QString::fromUtf8(localname->c_str());
#endif

    image = readbitmapQT(w, h, orientation, imagePath);

#ifdef _WIN32
    localname->resize(localname->size()-1);
#endif

    return (image!=NULL);
}

bool GfxProcQT::resizebitmap(int rw, int rh, string* jpegout)
{
    jpegout->clear();
    QImage result = resizebitmapQT(image, orientation, w, h, rw, rh);
    if(result.isNull()) return false;

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    result.save(&buffer, "JPG", 85);
    jpegout->assign(ba.constData(), ba.size());
    return !!jpegout->size();
}

void GfxProcQT::freebitmap()
{
    delete image;
}

QImage GfxProcQT::createThumbnail(QString imagePath)
{
    int w, h, orientation;
    QImageReader *image = readbitmapQT(w, h, orientation, imagePath);
    if(!image) return QImage();

    QImage result = GfxProcQT::resizebitmapQT(image, orientation, w, h, 120, 0);
    delete image;
    return result;
}

QImageReader *GfxProcQT::readbitmapQT(int &w, int &h, int &orientation, QString imagePath)
{
    QImageReader* image = new QImageReader(imagePath);
    QSize s = image->size();
    if(!s.isValid())
    {
        delete image;
        return NULL;
    }

    orientation = getExifOrientation(imagePath);
    if(orientation < ROTATION_LEFT_MIRRORED)
    {
        //No rotation or 180º rotation
        w = s.width();
        h = s.height();
    }
    else
    {
        //90º or 270º rotation
        w = s.height();
        h = s.width();
    }

    if(w && h) return image;
    else
    {
        delete image;
        return NULL;
    }
}

QImage GfxProcQT::resizebitmapQT(QImageReader *image, int orientation, int w, int h, int rw, int rh)
{
    int px, py;
    transform(w, h, rw, rh, px, py);

    //Assuming that the thumbnail is centered horizontally.
    //That is the case of MEGA thumbnails and makes the extraction easier and more efficient.
    if((orientation == ROTATION_DOWN) || (orientation == ROTATION_DOWN_MIRRORED) || (orientation == ROTATION_RIGHT_MIRRORED) || (orientation == ROTATION_RIGHT))
        py = (h-rh)-py;

    if(orientation < ROTATION_LEFT_MIRRORED)
    {
         //No rotation or 180º rotation
        image->setScaledSize(QSize(w, h));
        image->setScaledClipRect(QRect(px, py, rw, rh));
    }
    else
    {
        //90º or 270º rotation
        image->setScaledSize(QSize(h, w));
        image->setScaledClipRect(QRect(py, px, rh, rw));
    }

    QImage result = image->read();
    if(result.isNull()) return result;
    image->device()->seek(0);

    QTransform transform;

    //Manage rotation
    switch(orientation)
    {
        case ROTATION_DOWN:
        case ROTATION_DOWN_MIRRORED:
            transform.rotate(180);
            break;
        case ROTATION_LEFT:
        case ROTATION_LEFT_MIRRORED:
            transform.rotate(90);
            break;
        case ROTATION_RIGHT:
        case ROTATION_RIGHT_MIRRORED:
            transform.rotate(270);
            break;
        default:
            break;
    }

    //Manage mirroring
    if((orientation == ROTATION_UP_MIRRORED) || (orientation == ROTATION_DOWN_MIRRORED))
        transform.scale(-1, 1);
    else if((orientation == ROTATION_LEFT_MIRRORED) || (orientation == ROTATION_RIGHT_MIRRORED))
        transform.scale(1, -1);

    if(!transform.isIdentity())
        result = result.transformed(transform);

    return result;
}

} // namespace
