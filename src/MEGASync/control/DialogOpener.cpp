#include "DialogOpener.h"

#include <QApplication>
#include <QOperatingSystemVersion>

QList<std::shared_ptr<DialogOpener::DialogInfoBase>> DialogOpener::mOpenedDialogs = QList<std::shared_ptr<DialogOpener::DialogInfoBase>>();
QMap<QString, DialogOpener::GeometryInfo> DialogOpener::mSavedGeometries = QMap<QString, DialogOpener::GeometryInfo>();


#ifdef _WIN32
ExternalDialogOpener::ExternalDialogOpener()
    : QWidget(nullptr, Qt::SubWindow)
{
#ifdef _WIN32
    if(QOperatingSystemVersion::current() <= QOperatingSystemVersion::Windows10)
    {
        setAttribute(Qt::WA_DeleteOnClose, true);
        setWindowFlag(Qt::WindowStaysOnBottomHint, true);
        setWindowFlag(Qt::FramelessWindowHint, true);
        setFixedSize(0,0);
        show();
        raise();
        activateWindow();
    }
#endif
}

ExternalDialogOpener::~ExternalDialogOpener()
{
    close();
}
#endif

DialogBlocker::DialogBlocker(QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setGeometry(QRect(1,1,1,1));
    setModal(true);
    open();
    qApp->processEvents();
}

DialogBlocker::~DialogBlocker()
{
    close();
}
