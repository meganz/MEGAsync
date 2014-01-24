#include "ContextMenuExt.h"

//This is called when our menu item has been pressed
void on_upload_to_mega_selected(NautilusMenuItem *item, gpointer user_data)
{
    (void)user_data;

    GList *files;
    GList *l;

    files = (GList *)g_object_get_data ((GObject *) item, "mega_extension_files");

    for (l = files; l != NULL; l = l->next) {
           NautilusFileInfo *file = NAUTILUS_FILE_INFO (l->data);
           char *name;
           name = nautilus_file_info_get_name (file);
           g_print ("Upload to MEGA: %s\n", name);
           g_free (name);
    }
}

//Creates menu items
GList * mega_extension_get_file_items (NautilusMenuProvider *provider,
                GtkWidget *window,
                GList *files)
{
    (void)window;

    NautilusMenuItem *item;
    GList *l;
    GList *ret;

    //Here we should check if MEGAsync is started and the state of the selected files
    //to show the menu item "Upload to MEGA", "Get MEGA link" or both (See the Windows implementation)
    //We could add in each item the affected files
    //We could communicate the this extension and MEGAsync using a pipe.
    //The Windows implementation uses a Windows named pipe.

    for (l = files; l != NULL; l = l->next) {
            NautilusFileInfo *file = NAUTILUS_FILE_INFO (l->data);
            char *name;
            name = nautilus_file_info_get_name (file);
            g_print ("selected %s\n", name);
            g_free (name);
    }

    item = nautilus_menu_item_new ("MEGAExtension::upload_to_mega",
                                   "Upload to MEGA",
                                   "Upload files to you MEGA account",
                                   NULL /* icon name */);
    g_signal_connect (item, "activate", G_CALLBACK (on_upload_to_mega_selected), provider);
    g_object_set_data_full ((GObject*) item, "mega_extension_files",
                            nautilus_file_info_list_copy (files),
                            (GDestroyNotify)nautilus_file_info_list_free);
    ret = g_list_append (NULL, item);

    return ret;
}
