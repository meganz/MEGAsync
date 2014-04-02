#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef WIN32
    #include "platform/win/WindowsPlatform.h"
    typedef WindowsPlatform Platform;
#elif __APPLE__
     #include "platform/macx/MacXPlatform.h"
    typedef MacXPlatform Platform;
#else
    #include "platform/linux/LinuxPlatform.h"
    typedef LinuxPlatform Platform;
#endif

#endif // PLATFORM_H
