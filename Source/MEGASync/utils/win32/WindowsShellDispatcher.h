#ifndef WINDOWSSHELLDISPATCHER_H
#define WINDOWSSHELLDISPATCHER_H

#include <QString>
#include <QThread>

#include "MegaApplication.h"
#include "sdk/megaapi.h"
#include "utils/Preferences.h"

#include <windows.h>
#include <winbase.h>
#include <Shlobj.h>
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <Shobjidl.h>
#include <wchar.h>

#define CONNECTING_STATE 0
#define READING_STATE 1
#define WRITING_STATE 2
#define INSTANCES 4
#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096

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

class WindowsShellDispatcher : public QThread
{
    Q_OBJECT

 public:
    WindowsShellDispatcher(MegaApplication *receiver);
    ~WindowsShellDispatcher();

 protected:
	void run ();
	int dispatchPipe();
	VOID GetAnswerToRequest(LPPIPEINST pipe);
	QQueue<QString> uploadQueue;
	boolean stop;

 signals:
	void newUploadQueue(QQueue<QString> uploadQueue);

};

#endif

