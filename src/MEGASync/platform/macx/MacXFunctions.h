#ifndef MACXFUNCTIONS_H
#define MACXFUNCTIONS_H

#include <QMacToolBar>
#include <QString>
#include <QStringList>
#include <QWidget>

#import <objc/runtime.h>

Q_FORWARD_DECLARE_OBJC_CLASS(NSView);
Q_FORWARD_DECLARE_OBJC_CLASS(NSPopover);
Q_FORWARD_DECLARE_OBJC_CLASS(NSOpenPanel);

void setMacXActivationPolicy();
QStringList qt_mac_NSArrayToQStringList(void *nsarray);
void selectMultipleFiles(QString uploadTitle, QString defaultDir, bool showFiles, bool showFolders, bool modal, std::function<void (QStringList)> func);
void selectSingleFiles(QString uploadTitle, QString defaultDir, bool showFiles, bool showFolders, bool modal, std::function<void (QString)> func);
void raiseFileSelectionPanels();
bool startAtLogin(bool opt);
bool isStartAtLoginActive();
void addLoginItem();
void removeLoginItem();
void addPathToPlaces(QString path, QString pathName);
void removePathFromPlaces(QString path);
void setFolderIcon(QString path);
void unSetFolderIcon(QString path);
QString defaultOpenApp(QString extension);
void enableBlurForWindow(QWidget *window);
bool registerUpdateDaemon();
bool runHttpServer();
bool runHttpsServer();
bool userActive();
double uptime();
QString appBundlePath();
QString fromNSString(const NSString *str);


//You take the ownership of the returned value
NSPopover* allocatePopOverWithView(NSView* view, QSize size);
void showPopOverRelativeToRect(WId view, id popOver, QPointF rect);
void releaseIdObject(id obj);

static NSOpenPanel *multipleSelectionPanel = NULL;
static NSOpenPanel *singleSelectionPanel = NULL;

#endif // MACXFUNCTIONS_H
