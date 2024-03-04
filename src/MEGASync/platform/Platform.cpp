#include "Platform.h"

#ifdef WIN32
    #include "platform/win/PlatformImplementation.h"
#elif __APPLE__
    #include "platform/macx/PlatformImplementation.h"
#else
    #include "platform/linux/PlatformImplementation.h"
#endif

AbstractPlatform* Platform::mInstance = nullptr;

void Platform::create()
{
    mInstance = new PlatformImplementation();
}

void Platform::destroy()
{
    delete mInstance;
    mInstance = nullptr;
}

AbstractPlatform *Platform::getInstance()
{
    assert(mInstance != nullptr);
    return mInstance;
}

