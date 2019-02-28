#include "MEGAShellExt.h"
#include <glib/gprintf.h>

#define G_LOG_DOMAIN ((gchar*) 0)

void nemo_module_initialize(GTypeModule* module)
{
    mega_ext_register_type(module);
}

void nemo_module_shutdown(void)
{
}

void nemo_module_list_types(const GType **types, int *num_types)
{
    static GType type_list[1];

    type_list[0] = MEGA_EXT_TYPE;
    *types = type_list;
    *num_types = 1;
}
