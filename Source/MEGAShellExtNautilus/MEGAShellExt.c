#include <libnautilus-extension/nautilus-extension-types.h>
#include <libnautilus-extension/nautilus-file-info.h>
#include <libnautilus-extension/nautilus-menu-provider.h>
#include <libnautilus-extension/nautilus-info-provider.h>
#include "MEGAShellExt.h"
#include "mega_ext_client.h"

static GObjectClass *parent_class;

static void mega_ext_class_init(MEGAExtClass *class)
{
	parent_class = g_type_class_peek_parent(class);
}

static void mega_ext_instance_init(MEGAExt *mega_ext)
{
    mega_ext->srv_sock = -1;
    mega_ext->chan = NULL;
    mega_ext->num_retries = 2;
}

static const gchar *file_state_to_str(FileState state)
{
    switch(state) {
        case FILE_SYNCED:
            return "synced";
        case FILE_PENDING:
            return "pending";
        case FILE_SYNCING:
            return "syncing";
        case FILE_NOTFOUND:
        default:
            return "notfound";
    }
}

// user clicked on "Upload to MEGA" menu item
static void mega_ext_on_upload_selected(NautilusMenuItem *item, gpointer user_data)
{
    MEGAExt *mega_ext = MEGA_EXT(user_data);
    GList *l;
    GList *files;
    gboolean flag = FALSE;

    files = g_object_get_data(G_OBJECT(item), "MEGAExtension::files");
    for (l = files; l != NULL; l = l->next) {
        NautilusFileInfo *file = NAUTILUS_FILE_INFO(l->data);
        FileState state;
        gchar *path;
        GFile *fp;

        fp = nautilus_file_info_get_location(file);
        if (!fp)
            continue;

        path = g_file_get_path(fp);
        if (!path)
            continue;

        state = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(file), "MEGAExtension::state"));

        if (state != FILE_SYNCED && state != FILE_PENDING && state != FILE_SYNCING) {
            if (mega_ext_client_upload(mega_ext, path))
                flag = TRUE;
        }
    }

    if (flag)
        mega_ext_client_end_request(mega_ext);
}

// user clicked on "Get MEGA link" menu item
static void mega_ext_on_get_link_selected(NautilusMenuItem *item, gpointer user_data)
{
    MEGAExt *mega_ext = MEGA_EXT(user_data);
    GList *l;
    GList *files;
    gboolean flag = FALSE;

    files = g_object_get_data(G_OBJECT(item), "MEGAExtension::files");
    for (l = files; l != NULL; l = l->next) {
        NautilusFileInfo *file = NAUTILUS_FILE_INFO(l->data);
        FileState state;
        gchar *path;
        GFile *fp;

        fp = nautilus_file_info_get_location(file);
        if (!fp)
            continue;

        path = g_file_get_path(fp);
        if (!path)
            continue;

        state = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(file), "MEGAExtension::state"));

        if (state == FILE_SYNCED) {
            if (mega_ext_client_paste_link(mega_ext, path))
                flag = TRUE;
        }
    }

    if (flag)
        mega_ext_client_end_request(mega_ext);
}

// Executed on context menu for selected object(-s)
// Check the state of the selected files
// to show the menu item "Upload to MEGA", "Get MEGA link" or both
// Return: list of NautilusMenuItem
static GList *mega_ext_get_file_items(NautilusMenuProvider *provider, G_GNUC_UNUSED GtkWidget *window, GList *files)
{
    MEGAExt *mega_ext = MEGA_EXT(provider);
    GList *l, *l_out = NULL;
    int syncedFiles, syncedFolders, unsyncedFiles, unsyncedFolders;
    const gchar *out = NULL;

    g_debug("mega_ext_get_file_items");

    syncedFiles = syncedFolders = unsyncedFiles = unsyncedFolders = 0;

    // get list of selected objects
    for(l = files; l != NULL; l = l->next) {
        NautilusFileInfo *file = NAUTILUS_FILE_INFO (l->data);
        gchar *path;
        GFile *fp;
        FileState state;

        fp = nautilus_file_info_get_location(file);
        if (!fp)
            continue;

        path = g_file_get_path(fp);
        if (!path)
            continue;

        state = mega_ext_client_get_path_state(mega_ext, path);
        g_free(path);

        if (state == FILE_ERROR)
            continue;

        g_debug("State: %s", file_state_to_str(state));

        g_object_set_data_full((GObject*)file, "MEGAExtension::state", GINT_TO_POINTER(state), NULL);

        // count the number of synced / unsynced files and folders
        if (state == FILE_SYNCED) {
            if (nautilus_file_info_get_file_type(file) == G_FILE_TYPE_DIRECTORY) {
                syncedFolders++;
            } else {
                syncedFiles++;
            }
        } else {
            if (nautilus_file_info_get_file_type(file) == G_FILE_TYPE_DIRECTORY) {
                unsyncedFolders++;
            } else {
                unsyncedFiles++;
            }
        }
    }

    // if there any unsynced files / folders selected
    if (unsyncedFiles || unsyncedFolders) {
        NautilusMenuItem *item;
        out = mega_ext_client_get_string(mega_ext, STRING_UPLOAD, unsyncedFiles, unsyncedFolders);
        item = nautilus_menu_item_new("MEGAExtension::upload_to_mega", out, "Upload files to you MEGA account", "mega");
        g_signal_connect(item, "activate", G_CALLBACK(mega_ext_on_upload_selected), provider);
        g_object_set_data_full((GObject*)item, "MEGAExtension::files", nautilus_file_info_list_copy(files), (GDestroyNotify)nautilus_file_info_list_free);
        l_out = g_list_append(l_out, item);
    }

    // if there any synced files / folders selected
    if (syncedFiles || syncedFolders) {
        NautilusMenuItem *item;
        out = mega_ext_client_get_string(mega_ext, STRING_GETLINK, syncedFiles, syncedFolders);
        item = nautilus_menu_item_new("MEGAExtension::get_mega_link", out, "Get MEGA link", "mega");
        g_signal_connect(item, "activate", G_CALLBACK(mega_ext_on_get_link_selected), provider);
        g_object_set_data_full((GObject*)item, "MEGAExtension::files", nautilus_file_info_list_copy(files), (GDestroyNotify)nautilus_file_info_list_free);
        l_out = g_list_append(l_out, item);
    }

    return l_out;
}

static NautilusOperationResult mega_ext_update_file_info(NautilusInfoProvider *provider,
    NautilusFileInfo *file, G_GNUC_UNUSED GClosure *update_complete, G_GNUC_UNUSED NautilusOperationHandle **handle)
{
    MEGAExt *mega_ext = MEGA_EXT(provider);
    gchar *path;
    GFile *fp;
    FileState state;

    g_debug("mega_ext_update_file_info");

    fp = nautilus_file_info_get_location(file);
    if (!fp)
        return NAUTILUS_OPERATION_COMPLETE;

    path = g_file_get_path(fp);
    if (!path)
        return NAUTILUS_OPERATION_COMPLETE;

    state = mega_ext_client_get_path_state(mega_ext, path);
    g_free(path);

    if (state == FILE_ERROR || state == FILE_NOTFOUND)
        return NAUTILUS_OPERATION_COMPLETE;

    switch (state) {
        case FILE_SYNCED:
            nautilus_file_info_add_emblem(file, "mega-synced");
            break;
        case FILE_PENDING:
            nautilus_file_info_add_emblem(file, "mega-pending");
            break;
        case FILE_SYNCING:
            nautilus_file_info_add_emblem(file, "mega-syncing");
            break;
        default:
            break;
    }

    return NAUTILUS_OPERATION_COMPLETE;
}

static void mega_ext_menu_provider_iface_init(NautilusMenuProviderIface *iface)
{
    iface->get_file_items = mega_ext_get_file_items;
}

static void mega_ext_info_provider_iface_init(NautilusInfoProviderIface *iface)
{
    iface->update_file_info = mega_ext_update_file_info;
}

static GType mega_ext_type = 0;

GType mega_ext_get_type (void)
{
	return mega_ext_type;
}

void mega_ext_register_type(GTypeModule *module)
{
	static const GTypeInfo mega_type_info = {
		sizeof(MEGAExtClass),
		NULL,
		NULL,
		(GClassInitFunc)mega_ext_class_init,
		NULL,
		NULL,
		sizeof (MEGAExt),
		0,
		(GInstanceInitFunc)mega_ext_instance_init,
        NULL
	};

	static const GInterfaceInfo menu_provider_iface_info = {
		(GInterfaceInitFunc) mega_ext_menu_provider_iface_init,
		NULL,
		NULL
	};

    static const GInterfaceInfo info_provider_iface_info = {
        (GInterfaceInitFunc) mega_ext_info_provider_iface_init,
        NULL,
        NULL
    };

	mega_ext_type = g_type_module_register_type(module,
        G_TYPE_OBJECT, "MEGAExtension", &mega_type_info, 0);

	g_type_module_add_interface(module,
        mega_ext_type,
        NAUTILUS_TYPE_MENU_PROVIDER,
        &menu_provider_iface_info);

    g_type_module_add_interface(module,
        mega_ext_type,
        NAUTILUS_TYPE_INFO_PROVIDER,
        &info_provider_iface_info);
}
