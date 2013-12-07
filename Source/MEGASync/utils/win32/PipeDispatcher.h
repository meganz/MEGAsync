#ifndef PIPEDISPATCHER_H
#define PIPEDISPATCHER_H

#include "sdk/megaapi.h"
#include "MegaApplication.h"

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>

#include <iostream>

#include <QString>
#include <QThread>

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

class ShellDispatcher : public QThread
 {
	 Q_OBJECT

 public:
	ShellDispatcher();
	void stopTask();

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

