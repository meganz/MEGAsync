#include "Utilities.h"
#include "platform/Platform.h"

#include <Foundation/NSFileManager.h>

bool Utilities::moveFileToTrash(const QString &filePath)
{
    QFileInfo info(filePath);
    NSString *filepath = info.filePath().toNSString();
    NSURL *fileurl = [NSURL fileURLWithPath:filepath isDirectory:info.isDir()];
    NSURL *resultingUrl = nil;
    NSError *nserror = nil;
    NSFileManager *fm = [NSFileManager defaultManager];

    return [fm trashItemAtURL:fileurl resultingItemURL:&resultingUrl error:&nserror] == YES;
}

