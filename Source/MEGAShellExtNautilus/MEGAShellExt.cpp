#include "MEGAShellExt.h"
#include "ContextMenuExt.h"

GType extensions[1];
GType context_extension_type;

void mega_extension_instance_init(MEGAExtension *object)
{
    g_print ("mega_extension_instance_init\n");
    (void)object;
}

void mega_extension_class_init(MEGAExtensionClass *megaClass)
{
    g_print ("mega_extension_class_init\n");
    (void)megaClass;
}

void mega_extension_menu_provider_iface_init(
        NautilusMenuProviderIface *iface)
{
    //Put the callback that will create menu items
    iface->get_file_items = mega_extension_get_file_items;
}


void nautilus_module_initialize(GTypeModule *module)
{
    g_print ("nautilus_module_initialize\n");

    static const GTypeInfo info = {
            sizeof (MEGAExtensionClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) mega_extension_class_init,
            NULL,
            NULL,
            sizeof (MEGAExtension),
            0,
            (GInstanceInitFunc) mega_extension_instance_init,
            NULL
    };

    static const GInterfaceInfo menu_provider_iface_info = {
        (GInterfaceInitFunc) mega_extension_menu_provider_iface_init,
        NULL,
        NULL
    };

    //Register the extension
    context_extension_type = g_type_module_register_type (module,
                         G_TYPE_OBJECT,
                         "MEGAExtension",
                         &info, (GTypeFlags)0);

    //Add interfaces
    g_type_module_add_interface (module,
                     context_extension_type,
                     NAUTILUS_TYPE_MENU_PROVIDER,
                     &menu_provider_iface_info);

    //Put interfaces in this array
    //You should change the size in the declaration
    extensions[0] = context_extension_type;
}


void nautilus_module_shutdown()
{
    g_print ("nautilus_module_shutdown\n");
}


void nautilus_module_list_types(const GType **types, int *num_types)
{
    //Return the implemented interfaces
    *types = extensions;
    *num_types = G_N_ELEMENTS(extensions);
}
