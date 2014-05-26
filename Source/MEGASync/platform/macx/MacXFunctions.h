#ifndef MACXFUNCTIONS_H
#define MACXFUNCTIONS_H

#include <QString>

void setMacXActivationPolicy();
void SetProcessName(QString procname);
char *runWithRootPrivileges(char *command);
bool startAtLogin(bool opt);
bool isStartAtLoginActive();
void addPathToPlaces(QString path, QString pathName);
void removePathFromPlaces(QString path);
void setFolderIcon(QString path);
void unSetFolderIcon(QString path);

#endif // MACXFUNCTIONS_H
