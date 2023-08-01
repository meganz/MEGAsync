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
void selectorsImpl(QString uploadTitle, QString defaultDir, bool multiSelection, bool showFiles, bool showFolders, QWidget* parent, std::function<void (QStringList)> func);
void raiseFileSelectionPanels();
void closeFileSelectionPanels(QWidget* parent);

static NSOpenPanel *panel = nullptr;

#endif // MACXFUNCTIONS_H
