/**
 * @file gfxqt.cpp
 * @brief Graphics layer using FreeImage
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

#include "mega.h"
#include "gfxqt.h"

namespace mega {
bool GfxProcQT::isgfx(string* name)
{
    QString imagePath = QString::fromUtf8(name->c_str());
#ifdef WIN32
    if(imagePath.startsWith(QString::fromAscii("\\\\?\\")))
        imagePath = imagePath.mid(4);
#endif

    return !QImageReader::imageFormat(imagePath).isEmpty();
}

bool GfxProcQT::readbitmap(FileAccess* fa, string* localname, int size)
{
#ifdef _WIN32
    localname->append("");
    QString imagePath = QString::fromWCharArray((wchar_t *)localname->c_str());
    if(imagePath.startsWith(QString::fromAscii("\\\\?\\")))
        imagePath = imagePath.mid(4);
#else
    QString imagePath = QString::fromUtf8(localname->c_str());
#endif

    image = new QImageReader(imagePath);
    QSize s = image->size();
    w = s.width();
    h = s.height();
    return w && h;
}

bool GfxProcQT::resizebitmap(int rw, int rh, string* jpegout)
{
    int px, py;
    jpegout->clear();
    transform(w, h, rw, rh, px, py);

    image->setScaledSize(QSize(w, h));
    image->setScaledClipRect(QRect(px, py, rw, rh));
    QImage result = image->read();
    if(result.isNull())
        return false;
    image->device()->seek(0);

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
} // namespace
