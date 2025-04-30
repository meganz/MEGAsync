#ifndef NOTIFICATIONCATEGORYMANAGER_H
#define NOTIFICATIONCATEGORYMANAGER_H

// NotificationCategoryManager.h
#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h>

NS_ASSUME_NONNULL_BEGIN

@interface NotificationCategoryManager : NSObject

@property (nonatomic, strong) NSMutableSet<UNNotificationCategory *> *allCategories;

+ (instancetype)sharedManager;
- (void)addCategory:(UNNotificationCategory *)category;
- (void)removeCategoryByIdentifier:(NSString *)categoryIdentifier;
- (void)registerCategories;

@end

NS_ASSUME_NONNULL_END

#endif // NOTIFICATIONCATEGORYMANAGER_H
