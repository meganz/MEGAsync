#include "MacUtils.h"
#include <Cocoa/Cocoa.h>

bool downloadFileSynchronously(string url, string path, string userAgent)
{
    @autoreleasepool
    {
        NSString *stringURL = [NSString stringWithCString:url.c_str() encoding:NSUTF8StringEncoding];
        NSURL *myURL = [NSURL URLWithString:stringURL];
        if (myURL == nil)
        {
            return false;
        }

        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:myURL];
        if (!userAgent.empty())
        {
            [request setValue:[NSString stringWithCString:userAgent.c_str()
                                                encoding:NSUTF8StringEncoding]
                   forHTTPHeaderField:@"User-Agent"];
        }
        [request setCachePolicy:NSURLRequestReloadIgnoringLocalCacheData];

        NSString *destination = [NSString stringWithCString:path.c_str()
                                                  encoding:NSUTF8StringEncoding];

        // Only a scalar crosses the block boundary: the file is written inside
        // the completion handler, while the data is still owned by the session.
        // This file is built without ARC, so keeping the NSData to use it after
        // the wait below would need an explicit retain.
        __block bool success = false;
        dispatch_semaphore_t finished = dispatch_semaphore_create(0);

        NSURLSessionDataTask *task =
            [[NSURLSession sharedSession] dataTaskWithRequest:request
                                           completionHandler:^(NSData *data,
                                                               NSURLResponse *response,
                                                               NSError *error)
        {
            // Unlike dataWithContentsOfURL, which returned nil for an HTTP error
            // status, NSURLSession reports one as a successful task with the
            // error body as its data. Without this check a version cut-off rule
            // firing would write "404 not found" over the file and only surface
            // later as a bad signature.
            const NSInteger statusCode =
                [response isKindOfClass:[NSHTTPURLResponse class]] ?
                    [(NSHTTPURLResponse *)response statusCode] : 200;

            if (error == nil && data != nil && statusCode >= 200 && statusCode < 300)
            {
                success = [data writeToFile:destination atomically:NO] == YES;
            }

            dispatch_semaphore_signal(finished);
        }];

        [task resume];

        // The completion handler runs on the session's own delegate queue, not on
        // the one this was called from, so waiting here cannot deadlock.
        dispatch_semaphore_wait(finished, DISPATCH_TIME_FOREVER);
        dispatch_release(finished);

        return success;
    }
}
