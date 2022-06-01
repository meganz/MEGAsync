#include "Utilities.h"

#include <windows.h>
#include <shellapi.h>

bool Utilities::moveFileToTrash(const QString &filePath)
{
    const QString sourcePath = filePath;

    // double null termination needed, so can't use QString::utf16
    QVarLengthArray<wchar_t, MAX_PATH + 1> winFile(sourcePath.length() + 2);
    sourcePath.toWCharArray(winFile.data());
    winFile[sourcePath.length()] = wchar_t{};
    winFile[sourcePath.length() + 1] = wchar_t{};

    SHFILEOPSTRUCTW operation;
    operation.hwnd = nullptr;
    operation.wFunc = FO_DELETE;
    operation.pFrom = winFile.constData();
    operation.pTo = nullptr;
    operation.fFlags = FOF_ALLOWUNDO | FOF_NO_UI;
    operation.fAnyOperationsAborted = FALSE;
    operation.hNameMappings = nullptr;
    operation.lpszProgressTitle = nullptr;

    return SHFileOperation(&operation) == 0 ? true: false;
}

