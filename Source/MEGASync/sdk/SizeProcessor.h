#ifndef SIZEPROCESSOR_H
#define SIZEPROCESSOR_H

#include "megaapi.h"

class SizeProcessor : public TreeProcessor
{
    long long totalBytes;
public:
    SizeProcessor();
    virtual int processNode(Node* node);
    long long getTotalBytes();
};

#endif // SIZEPROCESSOR_H
