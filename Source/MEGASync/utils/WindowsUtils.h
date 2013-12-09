#ifndef WINDOWSUTILS_H
#define WINDOWSUTILS_H

#include "utils/win32/PipeDispatcher.h"

#include <QApplication>
#include <QString>
#include <QFile>
#include <QHash>
#include <QPixmap>
#include <QThread>

#define     WIN32_LEAN_AND_MEAN
#define     _WIN32_WINNT    0x0501

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
using namespace std;


typedef interface ITrayNotify ITrayNotify;
typedef interface ITrayNotifyNew ITrayNotifyNew;
typedef interface INotificationCB INotificationCB;

typedef struct tagNOTIFYITEM
{
    PWSTR pszExeName;
    PWSTR pszTip;
    HICON hIcon;
    HWND hWnd;
    DWORD dwPreference;
    UINT uID;
    GUID guidItem;
} NOTIFYITEM;

//INotificationCB GUID
[ uuid ("D782CCBA-AFB0-43F1-94DB-FDA3779EACCB") ]
interface INotificationCB : public IUnknown
{
    virtual HRESULT __stdcall Notify (ULONG, NOTIFYITEM *) = 0;
};

//ItrayNotifyNew GUID (Windows 8)
[ uuid ("D133CE13-3537-48BA-93A7-AFCD5D2053B4") ]

//Virtual Functions for Windows 8
interface ITrayNotifyNew : public IUnknown
{
    virtual HRESULT __stdcall RegisterCallback (INotificationCB *,unsigned long *) = 0;
    virtual HRESULT __stdcall UnregisterCallback (unsigned long ) = 0;
    virtual HRESULT __stdcall SetPreference (NOTIFYITEM const *) = 0;
    virtual HRESULT __stdcall EnableAutoTray (BOOL) = 0;
    virtual HRESULT __stdcall DoAction (BOOL) = 0;
};

//ItrayNotify GUID (XP - Windows 7)
[ uuid ("FB852B2C-6BAD-4605-9551-F15F87830935") ]
interface ITrayNotify : public IUnknown
{
    virtual HRESULT __stdcall RegisterCallback (INotificationCB *) = 0;
    virtual HRESULT __stdcall SetPreference (NOTIFYITEM const *) = 0;
    virtual HRESULT __stdcall EnableAutoTray (BOOL) = 0;
};

//TrayNotifyClass GUID
[ uuid ("25DEAD04-1EAC-4911-9E3A-AD0A4AB560FD") ]
class TrayNotify : public ITrayNotify {};

//TrayNotifyClassNew GUID
[ uuid ("25DEAD04-1EAC-4911-9E3A-AD0A4AB560FD") ]
class TrayNotifyNew : public ITrayNotifyNew {};
/*  ************************************************************************  */

class TrayNotificationReceiver : public INotificationCB
{
protected:
    ITrayNotify *m_ITrayNotify;
    ITrayNotifyNew *m_ITrayNotifyNew;
    LONG m_cRef;
    QString executable;
    unsigned long id;

public:
    TrayNotificationReceiver(ITrayNotify *m_ITrayNotify, QString &executable);
    TrayNotificationReceiver(ITrayNotifyNew *m_ITrayNotifyNew, QString &executable);
    boolean start();
    void stop();
    ~TrayNotificationReceiver();

    /*	INotificationCB methods  */
    HRESULT __stdcall QueryInterface(REFIID riid, PVOID *ppv);
    ULONG __stdcall AddRef(VOID);
    ULONG __stdcall Release(VOID);
    HRESULT __stdcall Notify(ULONG Event, NOTIFYITEM *NotifyItem);
};

class WindowsUtils
{


private:
    WindowsUtils();
    static HRESULT CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc);

public:
	static QString getSizeString(unsigned long long bytes);
	static QHash<QString, QString> extensionIcons;
    static void initialize();
    static boolean enableIcon(QString &executable);
    static void notifyNewFolder(QString &path);
    static void notifyFolderContentsChange(QString &path);
    static void notifyFolderDeleted(QString &path);
    static void notifyNewFile(QString &path);
    static void notifyFileDeleted(QString &path);
    static void notifyItemChange(QString &path);
    static void notifyAttributeChange(QString &path);
    static bool startOnStartup(bool value);
	static void showInFolder(QString pathIn);
};


#endif // WINDOWSUTILS_H
