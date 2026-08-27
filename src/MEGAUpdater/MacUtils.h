#ifndef MACUTILS_H
#define MACUTILS_H
#include <iostream>

using namespace std;

// An empty userAgent leaves the request with CFNetwork's default one.
bool downloadFileSynchronously(string url, string path, string userAgent);

#endif // MACUTILS_H
