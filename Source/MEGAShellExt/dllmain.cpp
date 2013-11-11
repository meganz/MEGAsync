#include <qt_windows.h>
#include <QAxFactory>
#include "MEGAShellExt.h"

QT_USE_NAMESPACE

QAXFACTORY_BEGIN(
   "{65833484-7A21-43D2-B98A-CC8597612028}", /* Type Library ID (TLB) */
   "{69846F54-F66C-4551-96F0-91BAC6F544B9}"  /* Application ID (AppID) */
)

QAXCLASS(ShellOverlayBinder1)
QAXCLASS(ShellOverlayBinder2)
QAXFACTORY_END()

//Si se cambia el icono, hay que borrar el contenido de esta carpeta:
//C:\Users\Javi\AppData\Local\Microsoft\Windows\Explorer
