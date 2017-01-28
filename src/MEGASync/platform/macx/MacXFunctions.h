#ifndef MACXFUNCTIONS_H
#define MACXFUNCTIONS_H

#include <QString>
#include <QStringList>

void setMacXActivationPolicy();
QStringList qt_mac_NSArrayToQStringList(void *nsarray);
QStringList uploadMultipleFiles(QString uploadTitle);
void SetProcessName(QString procname);
char *runWithRootPrivileges(char *command);
bool startAtLogin(bool opt);
bool isStartAtLoginActive();
void addPathToPlaces(QString path, QString pathName);
void removePathFromPlaces(QString path);
void setFolderIcon(QString path);
void unSetFolderIcon(QString path);
QString defaultOpenApp(QString extension);
void enableBlurForWindow(QWidget *window);

#endif // MACXFUNCTIONS_H
