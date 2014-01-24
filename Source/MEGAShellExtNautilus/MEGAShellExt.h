#ifndef MEGASHELLEXT_H
#define MEGASHELLEXT_H

/* Nautilus extension headers */
#include <libnautilus-extension/nautilus-extension-types.h>
#include <libnautilus-extension/nautilus-file-info.h>
#include <libnautilus-extension/nautilus-info-provider.h>
#include <libnautilus-extension/nautilus-menu-provider.h>
#include <libnautilus-extension/nautilus-property-page-provider.h>
#include <libnautilus-extension/nautilus-column-provider.h>

typedef struct {
    GObject parent_slot;
} MEGAExtension;

typedef struct {
    GObjectClass parent_slot;
} MEGAExtensionClass;


//Nautilus interface
void nautilus_module_initialize (GTypeModule*module);
void nautilus_module_shutdown (void);
void nautilus_module_list_types (const GType **types, int *num_types);

#endif // MEGASHELLEXT_H
