/**
 * @file gfx.cpp
 * @brief Platform-independent bitmap graphics transformation functionality
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
#include "mega/gfx.h"

namespace mega {
const int GfxProc::dimensions[][2] = {
    { 120, 0 },     // THUMBNAIL120X120: square thumbnail, cropped from near center
    { 1000, 1000 }  // PREVIEW1000x1000: scaled version inside 1000x1000 bounding square
};

void GfxProc::transform(int& w, int& h, int& rw, int& rh, int& px, int& py)
{
    if (rh)
    {
        // rectangular rw*rh bounding box
        if (h*rw > w*rh)
        {
            w = w * rh / h;
            h = rh;
        }
        else
        {
            h = h * rw / w;
            w = rw;
        }

        px = 0;
        py = 0;

        rw = w;
        rh = h;
    }
    else
    {
        // square rw*rw crop thumbnail
        if (w < h)
        {
            h = h * rw / w;
            w = rw;
        }
        else
        {
            w = w * rw / h;
            h = rw;
        }

        px = (w - rw) / 2;
        py = (h - rw) / 3;
        
        rh = rw;
    }
}

// load bitmap image, generate all designated sizes, attach to specified upload/node handle
// FIXME: move to a worker thread to keep the engine nonblocking
void GfxProc::gendimensionsputfa(FileAccess* fa, string* localfilename, handle th, SymmCipher* key, int missing)
{
    // (this assumes that the width of the largest dimension is max)
    if (readbitmap(fa, localfilename, dimensions[sizeof dimensions/sizeof dimensions[0]-1][0]))
    {
        string* jpeg = NULL;

        // successively downscale the original image
        for (int i = sizeof dimensions/sizeof dimensions[0]; i--; )
        {
            if (!jpeg)
            {
                jpeg = new string;
            }

            if (missing & (1 << i) && resizebitmap(dimensions[i][0], dimensions[i][1], jpeg))
            {
                // store the file attribute data - it will be attached to the file
                // immediately if the upload has already completed; otherwise, once
                // the upload completes
                client->putfa(th, (meta_t)i, key, jpeg);
                
                jpeg = NULL;
            }
        }

        if (jpeg)
        {
            free(jpeg);
        }

        freebitmap();
    }
}
} // namespace
