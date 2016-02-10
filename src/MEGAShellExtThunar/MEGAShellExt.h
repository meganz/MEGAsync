
#ifndef _MEGA_SYNC_EXT_PLUGIN_H_
#define _MEGA_SYNC_EXT_PLUGIN_H_

#include <thunarx/thunarx.h>

G_BEGIN_DECLS;

typedef struct _MEGAExtClass MEGAExtClass;
typedef struct _MEGAExt MEGAExt;

#define MEGA_TYPE_EXT            (mega_ext_get_type())
#define MEGA_EXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), MEGA_TYPE_EXT, MEGAExt))
#define MEGA_EXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), MEGA_TYPE_EXT, MEGAExtClass))
#define MEGA_IS_EXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), MEGA_TYPE_EXT))
#define MEGA_EXT_IS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MEGA_TYPE_EXT))
#define MEGA_EXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), MEGA_TYPE_EXT, MEGAExtClass))

struct _MEGAExtClass
{
    GObjectClass __parent__;
};

struct _MEGAExt
{
    GObject __parent__;

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


GType mega_ext_get_type(void) G_GNUC_CONST;
void  mega_ext_register_type(ThunarxProviderPlugin *plugin);

G_END_DECLS;

#endif
