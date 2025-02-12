#ifndef ALLMEGAINCLUDES_H
#define ALLMEGAINCLUDES_H

/******************************
 *
 * The include order has been copied from MEGASync main.cpp
 *
 * It is important for Windows : if including only MegaApplication.h
 * in the automated tests main, there are errors related to winsock.
 *
 *****************************/

#ifndef DEBUG
#include "CrashHandler.h"
#endif

#include "MegaApplication.h"
#include "MegaProxyStyle.h"
#include "Platform.h"
#include "PowerOptions.h"
#include "ProxyStatsEventHandler.h"
#include "qtlockedfile/qtlockedfile.h"

#include <QFontDatabase>

#include <cassert>
#include <iostream>

#ifdef Q_OS_LINUX
#include <condition_variable>
#include <signal.h>
#endif

#ifndef WIN32
#include <unistd.h>
#else
#include <Psapi.h>
#include <Shellapi.h>
#include <Strsafe.h>
#include <Windows.h>
#endif

#if defined(WIN32) || defined(Q_OS_LINUX)
#include "ScaleFactorManager.h"

#include <QScreen>
#endif

#endif // ALLMEGAINCLUDES_H
