#ifndef MEGASHELLEXT_H
#define MEGASHELLEXT_H

#include <QtCore/qglobal.h>

#if defined(MEGASHELLEXT_LIBRARY)
#  define MEGASHELLEXTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MEGASHELLEXTSHARED_EXPORT Q_DECL_IMPORT
#endif

#include <qt_windows.h>
#include <shlobj.h>

#include <QObject>
#include <QAxBindable>
#include <QAxAggregated>
#include <QAxFactory>
#include <QUuid>
#include <QFileInfo>
#include <QLocalSocket>
#include <QMutex>

class ShellOverlayBinder1 : public QObject, public QAxBindable {
    Q_OBJECT
    Q_CLASSINFO("ClassID",     "{05B38830-F4E9-4329-978B-1DD28605D202}")
    Q_CLASSINFO("InterfaceID", "{041C3EDC-5B93-4281-B694-FA113141F377}")
    Q_CLASSINFO("EventsID",    "{2B32244E-899C-48A4-B82E-F420CACEAD00}")

public:
    ShellOverlayBinder1(QObject *parent = 0);
    QAxAggregated *createAggregate();
};

class ShellOverlayBinder2 : public QObject, public QAxBindable {
    Q_OBJECT
    Q_CLASSINFO("ClassID",     "{056D528D-CE28-4194-9BA3-BA2E9197FF8C}")
    Q_CLASSINFO("InterfaceID", "{DD778A0B-C5D6-4D91-9D70-DB6D7F821B0D}")
    Q_CLASSINFO("EventsID",    "{EE0A65B8-C5BD-4CC0-A84F-27F489101E60}")

public:
    ShellOverlayBinder2(QObject *parent = 0);
    QAxAggregated *createAggregate();
};


class MEGASHELLEXTSHARED_EXPORT MEGAShellExt
        : public QObject, public QAxAggregated, public IShellIconOverlayIdentifier
{
    Q_OBJECT

public:
    MEGAShellExt(int id, QObject *parent = 0);

    long queryInterface(const QUuid &iid, void**iface);

    // IUnknown
    QAXAGG_IUNKNOWN

    /*! Query information about the overlay icon
      \param pwszIconFile output parameter where to put the array of overlay icon (wchar_t **)
      \param cchMax size of the pwszIconFile buffer, in characters (not bytes)
      \param pIndex output parameter, index of the icon in the pwszIconFile file (e.g. if the file contains multiple icons), starting at 0
      \param pdwFlags output parameter, options for the overlay icon
      \return S_OK in case of success
      */
    STDMETHOD(GetOverlayInfo)(LPWSTR pwszIconFile, int cchMax, int *pIndex, DWORD* pdwFlags);

    STDMETHOD(GetPriority)(int* pPriority);

    /*! Query if the overlay is present for a particular file
      \param pwszPath path of the file to query (wchar_t*)
      \param dwAttrib attributes of the file
      \return S_OK if the icon has to be overlayed, S_FALSE else
      */
    STDMETHOD(IsMemberOf)(LPCWSTR pwszPath,DWORD dwAttrib);

protected:
    QLocalSocket *clientSocket;
    int id;
};

#endif // MEGASHELLEXT_H
