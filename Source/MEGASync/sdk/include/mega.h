/**
 * @file mega.h
 * @brief Main header file for inclusion by client software.
 *
 * (c) 2013 by Mega Limited, Wellsford, New Zealand
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

#ifndef MEGA_H
#define MEGA_H 1

#ifndef MEGA_SDK
#define MEGA_SDK
#endif

// project types
#include "mega/types.h"

// project includes
#include "mega/account.h"
#include "mega/http.h"
#include "mega/attrmap.h"
#include "mega/backofftimer.h"
#include "mega/base64.h"
#include "mega/command.h"
#include "mega/console.h"
#include "mega/fileattributefetch.h"
#include "mega/filefingerprint.h"
#include "mega/file.h"
#include "mega/filesystem.h"
#include "mega/db.h"
#include "mega/json.h"
#include "mega/pubkeyaction.h"
#include "mega/request.h"
#include "mega/serialize64.h"
#include "mega/share.h"
#include "mega/sharenodekeys.h"
#include "mega/treeproc.h"
#include "mega/user.h"
#include "mega/utils.h"
#include "mega/waiter.h"

#include "mega/node.h"
#include "mega/sync.h"
#include "mega/transfer.h"
#include "mega/transferslot.h"
#include "mega/megaapp.h"
#include "mega/megaclient.h"

// target-specific headers
#include "megawaiter.h"
#include "meganet.h"
#include "megafs.h"
#include "megaconsole.h"
#include "megaconsolewaiter.h"

#include "mega/db/sqlite.h"
#include "mega/db/bdb.h"

#include "mega/gfx/freeimage.h"

#endif
