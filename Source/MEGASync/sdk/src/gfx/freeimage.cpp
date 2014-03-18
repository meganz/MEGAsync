/**
 * @file freeimage.cpp
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
#include "mega/gfx/freeimage.h"

#ifdef _WIN32
#define FreeImage_GetFileTypeX FreeImage_GetFileTypeU
#define FreeImage_LoadX FreeImage_LoadU
typedef const wchar_t freeimage_filename_char_t;
#else
#define FreeImage_GetFileTypeX FreeImage_GetFileType
#define FreeImage_LoadX FreeImage_Load
typedef const char freeimage_filename_char_t;
#endif

namespace mega {
bool GfxProcFreeImage::isgfx(string* name)
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

bool GfxProcFreeImage::readbitmap(FileAccess* fa, string* localname, int size)
{
#ifdef _WIN32
    localname->append("", 1);
#endif

    // FIXME: race condition, need to use open file instead of filename
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeX((freeimage_filename_char_t*)localname->data());

    if (fif == FIF_UNKNOWN)
    {
#ifdef _WIN32
        localname->resize(localname->size()-1);
#endif
        return false;
    }

    if (fif == FIF_JPEG)
    {
        // load JPEG (scale & EXIF-rotate)
        FITAG *tag;

        if (!(dib = FreeImage_LoadX(fif, (freeimage_filename_char_t*) localname->data(),
                                    JPEG_EXIFROTATE | JPEG_FAST | (size << 16))))
        {
#ifdef _WIN32
            localname->resize(localname->size()-1);
#endif
            return false;
        }

        if (FreeImage_GetMetadata(FIMD_COMMENTS, dib, "OriginalJPEGWidth", &tag))
        {
            w = atoi((char*)FreeImage_GetTagValue(tag));
        }
        else
        {
            w = FreeImage_GetWidth(dib);
        }

        if (FreeImage_GetMetadata(FIMD_COMMENTS, dib, "OriginalJPEGHeight", &tag))
        {
            h = atoi((char*)FreeImage_GetTagValue(tag));
        }
        else
        {
            h = FreeImage_GetHeight(dib);
        }
    }
    else
    {
        // load all other image types - for RAW formats, rely on embedded preview
        if (!(dib = FreeImage_LoadX(fif, (freeimage_filename_char_t*)localname->data(),
                                    (fif == FIF_RAW) ? RAW_PREVIEW : 0)))
        {
#ifdef _WIN32
            localname->resize(localname->size()-1);
#endif
            return false;
        }

        w = FreeImage_GetWidth(dib);
        h = FreeImage_GetHeight(dib);
    }
    
    return w && h;
}

bool GfxProcFreeImage::resizebitmap(int rw, int rh, string* jpegout)
{
    FIBITMAP* tdib;
    FIMEMORY* hmem;
    int px, py;

    jpegout->clear();

    transform(w, h, rw, rh, px, py);

    if ((tdib = FreeImage_Rescale(dib, w, h, FILTER_BILINEAR)))
    {
        FreeImage_Unload(dib);

        dib = tdib;

        if ((tdib = FreeImage_Copy(dib, px, py, px + rw, py + rh)))
        {
            FreeImage_Unload(dib);

            dib = tdib;

            if ((hmem = FreeImage_OpenMemory()))
            {
                if (FreeImage_SaveToMemory(FIF_JPEG, dib, hmem, JPEG_BASELINE | JPEG_OPTIMIZE | 85))
                {
                    BYTE* tdata;
                    DWORD tlen;

                    FreeImage_AcquireMemory(hmem, &tdata, &tlen);
                    jpegout->assign((char*)tdata, tlen);
                }

                FreeImage_CloseMemory(hmem);
            }
        }
    }

    return !!jpegout->size();
}

void GfxProcFreeImage::freebitmap()
{
    FreeImage_Unload(dib);
}
} // namespace
