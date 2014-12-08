#include "MEGAService.h"
#include "MacXSystemServiceTask.h"
#include "MacXFunctions.h"
#include <QStringList>

@implementation MEGAService

-(id)initWithDelegate:(MacXSystemServiceTask*)delegate_instance {

    self = [super init];
    if (self) {
       delegate = delegate_instance;
     }

    [NSApp setServicesProvider:self];
    NSUpdateDynamicServices();

    return self;
}

- (void)handleItems:(NSPasteboard *)pboard userData:(NSString *)userData error:(NSString **)error {

    if([[pboard types] containsObject:NSFilenamesPboardType]){

        QStringList itemList;
        NSArray* fileArray=[pboard propertyListForType:NSFilenamesPboardType];

        itemList = qt_mac_NSArrayToQStringList(fileArray);
        delegate->processItems(itemList);
    }
}

@end
