#include "WinShellDispatcherTask.h"
#include "megaapi.h"

PIPEINST Pipe[INSTANCES];
HANDLE hEvents[INSTANCES+1];

VOID DisconnectAndReconnect(DWORD i);
BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo);

using namespace std;
using namespace mega;

typedef enum {
       STRING_UPLOAD = 0,
       STRING_GETLINK = 1,
       STRING_SHARE = 2,
       STRING_SEND = 3,
       STRING_REMOVE_FROM_LEFT_PANE = 4
} StringID;

WinShellDispatcherTask::WinShellDispatcherTask(MegaApplication *receiver) : QThread()
{
    this->receiver = receiver;
    moveToThread(this);
}

WinShellDispatcherTask::~WinShellDispatcherTask()
{
}

void WinShellDispatcherTask::run()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Shell dispatcher starting...");
    connect(this, SIGNAL(newUploadQueue(QQueue<QString>)), receiver, SLOT(shellUpload(QQueue<QString>)));
    connect(this, SIGNAL(newExportQueue(QQueue<QString>)), receiver, SLOT(shellExport(QQueue<QString>)));
    dispatchPipe();
}

int WinShellDispatcherTask::dispatchPipe()
{
    DWORD i, dwWait, cbRet, dwErr;
    BOOL fSuccess;
    PCWSTR lpszPipename = TEXT("\\\\.\\pipe\\MEGAprivacyMEGAsync");

    // The initial loop creates several instances of a named pipe
    // along with an event object for each instance.  An
    // overlapped ConnectNamedPipe operation is started for
    // each instance.

    for (i = 0; i < INSTANCES; i++)
    {
        // Create an event object for this instance.
        hEvents[i] = CreateEvent(
            NULL,    // default security attribute
            TRUE,    // manual-reset event
            TRUE,    // initial state = signaled
            NULL);   // unnamed event object

        if (hEvents[i] == NULL)
        {
            printf("CreateEvent failed with %d.\n", GetLastError());
            return 0;
        }

        Pipe[i].oOverlap.hEvent = hEvents[i];

        Pipe[i].hPipeInst = CreateNamedPipe(
            lpszPipename,            // pipe name
            PIPE_ACCESS_DUPLEX |     // read/write access
            FILE_FLAG_OVERLAPPED,    // overlapped mode
            PIPE_TYPE_MESSAGE |      // message-type pipe
            PIPE_READMODE_MESSAGE |  // message-read mode
            PIPE_WAIT,               // blocking mode
            INSTANCES,               // number of instances
            BUFSIZE*sizeof(TCHAR),   // output buffer size
            BUFSIZE*sizeof(TCHAR),   // input buffer size
            PIPE_TIMEOUT,            // client time-out
            NULL);                   // default security attributes

        if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE)
        {
         printf("CreateNamedPipe failed with %d.\n", GetLastError());
         return 0;
        }

        // Call the subroutine to connect to the new client

        Pipe[i].fPendingIO = ConnectToNewClient(
                    Pipe[i].hPipeInst,
                    &Pipe[i].oOverlap);

        Pipe[i].dwState = Pipe[i].fPendingIO ?
                  CONNECTING_STATE : // still connecting
                  READING_STATE;     // ready to read
    }

    hEvents[INSTANCES] = CreateEvent(
        NULL,    // default security attribute
        FALSE,    // auro-reset event
        FALSE,    // initial state = unsignaled
        NULL);   // unnamed event object

    while (1)
    {
        // Wait for the event object to be signaled, indicating
        // completion of an overlapped read, write, or
        // connect operation.

        dwWait = WaitForMultipleObjects(
                    INSTANCES+1,    // number of event objects
                    hEvents,      // array of event objects
                    FALSE,        // does not wait for all
                    INFINITE);    // waits indefinitely

        // dwWait shows which pipe completed the operation.

        i = dwWait - WAIT_OBJECT_0;  // determines which pipe
        if (i < 0 || i > (INSTANCES - 1))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Shell dispatcher closing...");
            for (int j = 0; j < INSTANCES; j++)
            {
                CloseHandle(Pipe[j].hPipeInst);
            }

            for (int j = 0; j < (INSTANCES + 1); j++)
            {
                CloseHandle(hEvents[j]);
            }

            return 0;
        }

        // Get the result if the operation was pending.
        if (Pipe[i].fPendingIO)
        {
            fSuccess = GetOverlappedResult(
                        Pipe[i].hPipeInst, // handle to pipe
                        &Pipe[i].oOverlap, // OVERLAPPED structure
                        &cbRet,            // bytes transferred
                        FALSE);            // do not wait

            switch (Pipe[i].dwState)
            {
            // Pending connect operation
            case CONNECTING_STATE:
                if (!fSuccess)
                {
                    printf("Error %d.\n", GetLastError());
                    return 0;
                }
                Pipe[i].dwState = READING_STATE;
                break;

            // Pending read operation
            case READING_STATE:
                if (!fSuccess || cbRet == 0)
                {
                    DisconnectAndReconnect(i);
                    continue;
                }
                Pipe[i].cbRead = cbRet;
                Pipe[i].dwState = WRITING_STATE;
                break;

            // Pending write operation
            case WRITING_STATE:
                if (!fSuccess || cbRet != Pipe[i].cbToWrite)
                {
                    DisconnectAndReconnect(i);
                    continue;
                }
                Pipe[i].dwState = READING_STATE;
                break;

            default:
            {
                printf("Invalid pipe state.\n");
                return 0;
            }
            }
        }

        // The pipe state determines which operation to do next.
        switch (Pipe[i].dwState)
        {
            // READING_STATE:
            // The pipe instance is connected to the client
            // and is ready to read a request from the client.

        case READING_STATE:
            fSuccess = ReadFile(
                        Pipe[i].hPipeInst,
                        Pipe[i].chRequest,
                        BUFSIZE*sizeof(TCHAR),
                        &Pipe[i].cbRead,
                        &Pipe[i].oOverlap);

            // The read operation completed successfully.
            if (fSuccess && Pipe[i].cbRead != 0)
            {
                Pipe[i].fPendingIO = FALSE;
                Pipe[i].dwState = WRITING_STATE;
                continue;
            }

            // The read operation is still pending.
            dwErr = GetLastError();
            if (! fSuccess && (dwErr == ERROR_IO_PENDING))
            {
               Pipe[i].fPendingIO = TRUE;
               continue;
            }

            // An error occurred; disconnect from the client.
            DisconnectAndReconnect(i);
            break;

            // WRITING_STATE:
            // The request was successfully read from the client.
            // Get the reply data and write it to the client.
        case WRITING_STATE:
            GetAnswerToRequest(&Pipe[i]);

            fSuccess = WriteFile(
                        Pipe[i].hPipeInst,
                        Pipe[i].chReply,
                        Pipe[i].cbToWrite,
                        &cbRet,
                        &Pipe[i].oOverlap);

            // The write operation completed successfully.
            if (fSuccess && cbRet == Pipe[i].cbToWrite)
            {
                Pipe[i].fPendingIO = FALSE;
                Pipe[i].dwState = READING_STATE;
                continue;
            }

            // The write operation is still pending.
            dwErr = GetLastError();
            if (! fSuccess && (dwErr == ERROR_IO_PENDING))
            {
                Pipe[i].fPendingIO = TRUE;
                continue;
            }

            // An error occurred; disconnect from the client.
            DisconnectAndReconnect(i);
            break;

        default:
        {
            printf("Invalid pipe state.\n");
            return 0;
        }
        }
    }

   return 0;
}

void WinShellDispatcherTask::exitTask()
{
    SetEvent(hEvents[INSTANCES]);
    this->exit();
}


// DisconnectAndReconnect(DWORD)
// This function is called when an error occurs or when the client
// closes its handle to the pipe. Disconnect from this client, then
// call ConnectNamedPipe to wait for another client to connect.

VOID DisconnectAndReconnect(DWORD i)
{
    // Disconnect the pipe instance.
    if (! DisconnectNamedPipe(Pipe[i].hPipeInst) )
    {
        printf("DisconnectNamedPipe failed with %d.\n", GetLastError());
    }

    // Call a subroutine to connect to the new client.
    Pipe[i].fPendingIO = ConnectToNewClient(
                Pipe[i].hPipeInst,
                &Pipe[i].oOverlap);

    Pipe[i].dwState = Pipe[i].fPendingIO ?
                CONNECTING_STATE : // still connecting
                READING_STATE;     // ready to read
}

// ConnectToNewClient(HANDLE, LPOVERLAPPED)
// This function is called to start an overlapped connect operation.
// It returns TRUE if an operation is pending or FALSE if the
// connection has been completed.

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo)
{
    BOOL fConnected, fPendingIO = FALSE;

    // Start an overlapped connection for this pipe instance.
    fConnected = ConnectNamedPipe(hPipe, lpo);

    // Overlapped ConnectNamedPipe should return zero.
    if (fConnected)
    {
        printf("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }

    switch (GetLastError())
    {
    // The overlapped connection in progress.
    case ERROR_IO_PENDING:
        fPendingIO = TRUE;
        break;

    // Client is already connected, so signal an event.
    case ERROR_PIPE_CONNECTED:
        if (SetEvent(lpo->hEvent))
            break;

    // If an error occurs during the connect operation...
    default:
    {
        printf("ConnectNamedPipe failed with %d.\n", GetLastError());
        return 0;
    }
    }

   return fPendingIO;
}

#define RESPONSE_DEFAULT    L"9"
#define RESPONSE_SYNCED     L"0"
#define RESPONSE_PENDING    L"1"
#define RESPONSE_SYNCING    L"2"

VOID WinShellDispatcherTask::GetAnswerToRequest(LPPIPEINST pipe)
{
    //wprintf( TEXT("[%d] %s\n"), pipe->hPipeInst, pipe->chRequest);
    wcscpy_s(pipe->chReply, BUFSIZE, RESPONSE_DEFAULT);
    wchar_t c = pipe->chRequest[0];
    wchar_t *content =  pipe->chRequest+2;
    switch(c)
    {
        case L'T':
        {
            if (lstrlen(pipe->chRequest) < 3)
            {
                break;
            }

            bool ok;
            QStringList parameters = QString::fromWCharArray(content).split(QChar::fromAscii(':'));
            if (parameters.size() < 1)
            {
                break;
            }

            int stringId = parameters[0].toInt(&ok);
            if (!ok)
            {
                break;
            }

            QString actionString;
            switch(stringId)
            {
                case STRING_UPLOAD:
                    actionString = QCoreApplication::translate("ShellExtension", "Upload to MEGA");
                    break;
                case STRING_GETLINK:
                    actionString = QCoreApplication::translate("ShellExtension", "Get MEGA link");
                    break;
                case STRING_SHARE:
                    actionString = QCoreApplication::translate("ShellExtension", "Share with a MEGA user");
                    break;
                case STRING_SEND:
                    actionString = QCoreApplication::translate("ShellExtension", "Send to a MEGA user");
                    break;
                case STRING_REMOVE_FROM_LEFT_PANE:
                    actionString = QCoreApplication::translate("ShellExtension", "Remove from left pane");
                    break;
            }

            wcscpy_s( pipe->chReply, BUFSIZE, (const wchar_t *)actionString.utf16());
            break;
        }
        case L'F':
        {
            if (lstrlen(pipe->chRequest) < 3)
            {
                break;
            }
            QString filePath = QString::fromWCharArray(content);
            if (filePath.startsWith(QString::fromAscii("\\\\?\\")))
            {
                filePath = filePath.mid(4);
            }

            QFileInfo file(filePath);
            if (file.exists())
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Adding file to upload queue: %1").arg(filePath).toUtf8().constData());
                uploadQueue.enqueue(QDir::toNativeSeparators(file.absoluteFilePath()));
            }
            break;
        }
        case L'L':
        {
            if (lstrlen(pipe->chRequest) < 3)
            {
                break;
            }

            QString filePath = QString::fromWCharArray(content);
            if (filePath.startsWith(QString::fromAscii("\\\\?\\")))
            {
                filePath = filePath.mid(4);
            }

            QFileInfo file(filePath);
            if (file.exists())
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Adding file to export queue: %1")
                             .arg(filePath).toUtf8().constData());
                exportQueue.enqueue(QDir::toNativeSeparators(file.absoluteFilePath()));
            }
            break;
        }
        case L'P':
        {
            if (lstrlen(pipe->chRequest) < 3)
            {
                break;
            }

            bool overlayIcons = true;
            QStringList parameters = QString::fromWCharArray(content).split(QChar::fromAscii('|'));
            if (parameters.size() > 1)
            {
                bool ok;
                overlayIcons = parameters[1].toInt(&ok);
                if (!ok)
                {
                    overlayIcons = true;
                }
            }

            if ((parameters[0].size() < 3) || (overlayIcons && Preferences::instance()->overlayIconsDisabled()))
            {
                break;
            }

            int state;
            QString temp = parameters[0];
            if (temp.startsWith(QString::fromAscii("\\\\?\\")))
            {
                temp = temp.mid(4);
            }

            if ((temp == lastPath) && (numHits < 3))
            {
                state = lastState;
                numHits++;
            }
            else
            {
                MegaApplication *app = (MegaApplication *)qApp;
                MegaApi *megaApi = app->getMegaApi();
                string tmpPath((const char*)temp.utf16(), temp.size()*sizeof(wchar_t));
                state = megaApi->syncPathState(&tmpPath);
                lastState = state;
                lastPath = temp;
                numHits = 1;
            }

            switch(state)
            {
                case MegaApi::STATE_SYNCED:
                    wcscpy_s( pipe->chReply, BUFSIZE, RESPONSE_SYNCED );
                    break;
                case MegaApi::STATE_SYNCING:
                     wcscpy_s( pipe->chReply, BUFSIZE, RESPONSE_SYNCING );
                     break;
                case MegaApi::STATE_PENDING:
                     wcscpy_s( pipe->chReply, BUFSIZE, RESPONSE_PENDING );
                     break;
                case MegaApi::STATE_NONE:
                case MegaApi::STATE_IGNORED:
                default:
                     wcscpy_s( pipe->chReply, BUFSIZE, RESPONSE_DEFAULT );
            }
            break;
        }
        case L'E':
        {
            if (!uploadQueue.isEmpty())
            {
                emit newUploadQueue(uploadQueue);
                uploadQueue.clear();
            }

            if (!exportQueue.isEmpty())
            {
                emit newExportQueue(exportQueue);
                exportQueue.clear();
            }
        }
        case L'I':
        default:;
    }
    pipe->cbToWrite = (lstrlen(pipe->chReply)+1)*sizeof(WCHAR);
}
