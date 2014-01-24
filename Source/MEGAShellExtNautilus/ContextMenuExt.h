#ifndef CONTEXTMENUEXT_H
#define CONTEXTMENUEXT_H

#include "MEGAShellExt.h"

void on_upload_to_mega_selected (NautilusMenuItem *item, gpointer user_data);
GList * mega_extension_get_file_items (NautilusMenuProvider *provider, GtkWidget *window, GList *files);

#endif // CONTEXTMENUEXT_H
