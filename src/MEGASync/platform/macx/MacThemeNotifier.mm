#import <AppKit/AppKit.h>

typedef void (*ThemeCallback)(bool isDark);

static inline bool readAppearance() {
    NSAppearance *appearance = NSApp.effectiveAppearance;
    NSAppearanceName best = [appearance bestMatchFromAppearancesWithNames:@[
        NSAppearanceNameAqua, NSAppearanceNameDarkAqua
    ]];
    return [best isEqualToString:NSAppearanceNameDarkAqua];
}

@interface ThemeObserver : NSObject
@property (nonatomic, assign) ThemeCallback cb;
@end

@implementation ThemeObserver

- (instancetype)initWithCallback:(ThemeCallback)cb {
    if ((self = [super init])) {
        _cb = cb;

        [[NSDistributedNotificationCenter defaultCenter] addObserver:self
                                                            selector:@selector(onThemeChanged:)
                                                                name:@"AppleInterfaceThemeChangedNotification"
                                                              object:nil
                                                  suspensionBehavior:NSNotificationSuspensionBehaviorDeliverImmediately];
    }
    return self;
}

- (void)dealloc {
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (void)onThemeChanged:(NSNotification *) __unused note {
    [self notifyNow];
}

- (void)notifyNow {
    if (!_cb) return;

    bool isDark = readAppearance();
    _cb(isDark);
}

@end

extern "C" {
    void* MacTheme_Create(ThemeCallback cb) {
        ThemeObserver *obs = [[ThemeObserver alloc] initWithCallback:cb];
        return (void*)obs;
    }
    void  MacTheme_Destroy(void* handle) {
        ThemeObserver *obs = (ThemeObserver*)handle;
        [obs release];
    }

    void MacTheme_GetCurrentAppearance(bool* outDark) {
        if (outDark)
        {
            *outDark = readAppearance();
        }
    }
}
