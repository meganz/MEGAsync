#include "SizeProcessor.h"

SizeProcessor::SizeProcessor()
{
    totalBytes=0;
}


int SizeProcessor::processNode(Node *node)
{
    totalBytes += node->size;
    return true;
}

long long SizeProcessor::getTotalBytes()
{
    return totalBytes;
}
