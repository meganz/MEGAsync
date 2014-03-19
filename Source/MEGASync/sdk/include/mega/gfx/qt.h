/**
 * @file qtgfx.h
 * @brief Graphics layer implementation using QT
 *
 * (c) 2014 by Mega Limited, Wellsford, New Zealand
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

#ifdef USE_QT
#ifndef GFX_CLASS
#define GFX_CLASS GfxProcQT

#include <QImageReader>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QIODevice>
#include <QSize>
#include <QFile>

namespace mega {

// bitmap graphics processor
class MEGA_API GfxProcQT : public GfxProc
{
    enum {
        ROTATION_UP = 1,
        ROTATION_UP_MIRRORED = 2,
        ROTATION_DOWN = 3,
        ROTATION_DOWN_MIRRORED = 4,
        ROTATION_LEFT_MIRRORED = 5,
        ROTATION_LEFT = 6,
        ROTATION_RIGHT_MIRRORED = 7,
        ROTATION_RIGHT = 8
    };

    QImageReader *image;
    int orientation;
    int w, h;

    bool readbitmap(FileAccess*, string*, int);
    bool resizebitmap(int, int, string*);
    void freebitmap();

protected:
    static int processEXIF(QByteArray *barr, int itemlen);
    static int processEXIFDir(const char *dirStart, const char *offsetBase, uint32_t size, uint32_t nesting, int MotorolaOrder);
    static QImageReader *readbitmapQT(int &w, int &h, int &orientation, QString imagePath);
    static QImage resizebitmapQT(QImageReader *image, int orientation, int w, int h, int rw, int rh);

public:
    bool isgfx(string*);
    static int getExifOrientation(QString &filePath);
    static QImage createThumbnail(QString imagePath);

};
} // namespace

#endif
#endif
