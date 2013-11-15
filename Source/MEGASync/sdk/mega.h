#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <memory.h>
#include <time.h>

#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>

#ifndef __MACH__
#include <endian.h>
#endif
#endif

#ifdef _WIN32
#define atoll _atoi64
#define snprintf _snprintf
#define mkdir _mkdir
#include <windows.h>
#include <winhttp.h>
#define _CRT_SECURE_NO_WARNINGS
#endif

typedef int64_t m_off_t;

#ifndef htobe64
#define htobe64(x) (((uint64_t) htonl((uint32_t) ((x) >> 32))) | (((uint64_t) htonl((uint32_t) x)) << 32))
#endif

#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <iterator>
#include <queue>
#include <list>

using namespace std;
