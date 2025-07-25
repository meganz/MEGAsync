#ifndef WINSHELLDISPATCHERTASK_H
#define WINSHELLDISPATCHERTASK_H

#include "MegaApplication.h"
#include <Shlobj.h>
#include <Shobjidl.h>

#include <QFileInfo>
#include <QString>
#include <QThread>

#include <commctrl.h>
#include <objbase.h>
#include <stdlib.h>
#include <winbase.h>
#include <windows.h>

#define CONNECTING_STATE 0
#define READING_STATE 1
#define WRITING_STATE 2
#define INSTANCES 21
#define PIPE_TIMEOUT 5000
#define BUFSIZE 512

typedef struct
{
   OVERLAPPED oOverlap;
   HANDLE hPipeInst;
   TCHAR chRequest[BUFSIZE];
   DWORD cbRead;
   TCHAR chReply[BUFSIZE];
   DWORD cbToWrite;
   DWORD dwState;
   BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;

class WinShellDispatcherTask : public QThread
{
    Q_OBJECT

 public:
    WinShellDispatcherTask(MegaApplication *receiver);
    virtual ~WinShellDispatcherTask();
    void exitTask();

 protected:
    int dispatchPipe();
    VOID GetAnswerToRequest(LPPIPEINST pipe);
    QQueue<QString> uploadQueue;
    QQueue<QString> exportQueue;
    MegaApplication *receiver;
    QString lastPath;
    int lastState;
    int numHits;

 signals:
    void newUploadQueue(QQueue<QString> uploadQueue);
    void newExportQueue(QQueue<QString> exportQueue);
    void viewOnMega(const QString& path, bool versions);
    void syncFolderFromShellExt(const QString& path);
    void backupFolderFromShellExt(const QString& path);

protected:
    virtual void run();

private:
    QStringList extractParameters(wchar_t* content, const QChar& separator = QChar());
    QStringList extractParametersWChar(wchar_t* content, const QChar& separator = QChar());
    int toIntFromWChar(const QString& str, bool& ok);

    void sendViewOnMegaSignal(const QStringList& parameters, bool versions, LPPIPEINST pipe);

    void sendErrorToPipe(LPPIPEINST pipe);
};

#endif

