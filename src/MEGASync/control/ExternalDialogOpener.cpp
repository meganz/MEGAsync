#include "ExternalDialogOpener.h"

ExternalDialogOpener::ExternalDialogOpener()
{
#ifdef _WIN32
    if(QSysInfo::windowsVersion() <= QSysInfo::WV_WINDOWS10)
    {
        mActivationWidget = new QWidget(nullptr, Qt::SubWindow);
        mActivationWidget->setAttribute(Qt::WA_DeleteOnClose, true);
        mActivationWidget->setWindowFlag(Qt::WindowStaysOnBottomHint, true);
        mActivationWidget->setWindowFlag(Qt::FramelessWindowHint, true);
        mActivationWidget->setFixedSize(0,0);
        mActivationWidget->show();
        mActivationWidget->raise();
        mActivationWidget->activateWindow();
    }
#endif
}

ExternalDialogOpener::~ExternalDialogOpener()
{
    if(mActivationWidget)
    {
        mActivationWidget->close();
    }
}
