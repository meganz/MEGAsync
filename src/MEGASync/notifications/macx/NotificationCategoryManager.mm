// NotificationCategoryManager.mm
#import "NotificationCategoryManager.h"

@implementation NotificationCategoryManager

+ (instancetype)sharedManager {
    static NotificationCategoryManager *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
        sharedInstance.allCategories = [NSMutableSet set];
    });
    return sharedInstance;
}

- (void)addCategory:(UNNotificationCategory *)category {
    [self.allCategories addObject:category];
    [self registerCategories];  // Register the updated set of categories
}

- (void)registerCategories {
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
    [center setNotificationCategories:self.allCategories];
}

- (void)removeCategoryByIdentifier:(NSString *)categoryIdentifier {

    // Remove the category with the given identifier from the mutable set
    for (UNNotificationCategory *category in self.allCategories) {
        if ([category.identifier isEqualToString:categoryIdentifier]) {
            [self.allCategories removeObject:category];
            break;
        }
    }
}



@end
