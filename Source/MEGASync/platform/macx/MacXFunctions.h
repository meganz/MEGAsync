#ifndef MACXFUNCTIONS_H
#define MACXFUNCTIONS_H

#include <QString>

void shutUpAppKit(void);
char *runWithRootPrivileges(char *command);
void setMacXActivationPolicy();
bool startAtLogin(bool opt);
bool isStartAtLoginActive();
void addPathToPlaces(QString path, QString pathName);
void removePathFromPlaces(QString path);
void setFolderIcon(QString path);
void unSetFolderIcon(QString path);

#endif // MACXFUNCTIONS_H
