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
#include <string>
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
     // bestEffort: legacy pipe - a failed claim is expected and non-fatal (logged
     // softly, integration not disabled). pipeName: the name this instance serves.
     WinShellDispatcherTask(MegaApplication* receiver, std::wstring pipeName, bool bestEffort);
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
    // Emitted once when the legacy pipe first serves a request (SNC-6808 telemetry).
    void legacyPipeUsed();

protected:
    virtual void run();

private:
    QStringList extractParameters(wchar_t* content, const QChar& separator = QChar());
    int toIntFromWChar(const QString& str, bool& ok);

    void sendViewOnMegaSignal(const QStringList& parameters, bool versions, LPPIPEINST pipe);

    void sendErrorToPipe(LPPIPEINST pipe);

    // Closes the first `count` created pipe instances and their events, and
    // clears their slots.
    void closeInstances(DWORD count);

    // Closes the exit event (hEvents[INSTANCES]) and clears its slot.
    void closeExitEvent();

    // Disconnects pipe instance `i` and re-arms it for the next client.
    VOID DisconnectAndReconnect(DWORD i);

    // Per-dispatcher state (was globals) so scoped and legacy dispatchers can
    // run at once (SNC-6808). Zero-initialised on purpose: as globals these were
    // zeroed for free, as members they would hold garbage, and exitTask() reads
    // hEvents[INSTANCES] even when dispatchPipe() returned before creating it.
    PIPEINST Pipe[INSTANCES]{};
    HANDLE hEvents[INSTANCES + 1]{};

    std::wstring mPipeName;
    bool mBestEffort = false;
    bool mLegacyUsageReported = false;
};

#endif

