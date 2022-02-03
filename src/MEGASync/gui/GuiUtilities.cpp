#include "gui/GuiUtilities.h"

namespace GuiUtilities{

using namespace mega;

void updateDataRequestProgressBar(QProgressBar* progressBar, MegaRequest *request)
{
    if (request->getType() == MegaRequest::TYPE_FETCH_NODES)
    {
        if (request->getTotalBytes() > 0)
        {
            double total = static_cast<double>(request->getTotalBytes());
            double part = static_cast<double>(request->getTransferredBytes());
            progressBar->setMaximum(100);
            progressBar->setValue(static_cast<int>(100.0 * part / total));
        }
    }
}

}
