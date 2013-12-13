#ifndef UTILS_H
#define UTILS_H

#include "utils/CommonUtils.h"

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #define _WIN32_WINNT    0x0501
    #include "utils/win32/WindowsUtils.h"
    typedef CommonUtils<WindowsUtils> Utils;
#endif

#endif // UTILS_H
