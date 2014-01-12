#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef WIN32
    #include "platform/win/WindowsPlatform.h"
    typedef WindowsPlatform Platform;
#endif

#endif // PLATFORM_H
