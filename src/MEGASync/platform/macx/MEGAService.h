#ifndef MEGASERVICE_H
#define MEGASERVICE_H

#import <Cocoa/Cocoa.h>

class MacXSystemServiceTask;

@interface MEGAService : NSObject  {
    MacXSystemServiceTask *delegate;
}

-(id)initWithDelegate:(MacXSystemServiceTask*)delegate;
@end

#endif // MEGASERVICE_H
