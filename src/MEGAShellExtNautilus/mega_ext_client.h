#ifndef MEGA_EXT_CLIENT_H
#define MEGA_EXT_CLIENT_H

#include "MEGAShellExt.h"

gchar *mega_ext_client_get_string(MEGAExt *mega_ext, int stringID, int numFiles, int numFolders);
FileState mega_ext_client_get_path_state(MEGAExt *mega_ext, const gchar *path);
gboolean mega_ext_client_paste_link(MEGAExt *mega_ext, const gchar *path);
gboolean mega_ext_client_upload(MEGAExt *mega_ext, const gchar *path);
gboolean mega_ext_client_end_request(MEGAExt *mega_ext);

#endif
