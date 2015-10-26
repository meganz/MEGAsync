#ifndef MEGASHELLEXT_H
#define MEGASHELLEXT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MEGA_EXT_TYPE  (mega_ext_get_type())
#define MEGA_EXT(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), MEGA_EXT_TYPE, MEGAExt))
#define MEGA_IS_EXT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), MEGA_EXT_TYPE))

typedef struct _MEGAExt MEGAExt;
typedef struct _MEGAExtClass MEGAExtClass;

struct _MEGAExt {
    GObject __parent;
    GIOChannel *chan;
    GIOChannel *notify_chan;
    int srv_sock;
    int notify_sock;
    gint num_retries; // reconnection retries
    gboolean syncs_received; // TRUE if the list with sync folders is received

    GHashTable *h_syncs; // table of paths of shared folders
    gchar *string_upload; // cached string
    gchar *string_getlink; // cached string
};

struct _MEGAExtClass {
    GObjectClass __parent;
};

typedef enum {
    FILE_ERROR = 0,
    FILE_SYNCED = 1,
    FILE_PENDING = 2,
    FILE_SYNCING = 3,
    FILE_NOTFOUND = 9,
} FileState;

typedef enum {
    STRING_UPLOAD = 0,
    STRING_GETLINK = 1,
    STRING_SHARE = 2,
    STRING_SEND = 3
} StringID;

GType mega_ext_get_type(void);
void  mega_ext_register_type(GTypeModule *module);

G_END_DECLS

void mega_ext_on_item_changed(MEGAExt *mega_ext, const gchar *path);
void mega_ext_on_sync_add(MEGAExt *mega_ext, const gchar *path);
void mega_ext_on_sync_del(MEGAExt *mega_ext, const gchar *path);

#endif
