#ifndef MEGASHELLEXT_H
#define MEGASHELLEXT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define MEGA_EXT_TYPE  (mega_ext_get_type ())
#define MEGA_EXT(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), MEGA_EXT_TYPE, MEGAExt))
#define MEGA_IS_EXT(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), MEGA_EXT_TYPE))

typedef struct _MEGAExt MEGAExt;
typedef struct _MEGAExtClass MEGAExtClass;

struct _MEGAExt {
	GObject __parent;
    GIOChannel *chan;
    int srv_sock;
    gint num_retries;
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

GType mega_ext_get_type (void);
void  mega_ext_register_type (GTypeModule *module);

G_END_DECLS

#endif
