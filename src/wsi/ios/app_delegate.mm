#import "app_delegate.hpp"

#include "ios_main.hpp"

@interface AppDelegate ()

@end

@implementation AppDelegate

@synthesize window;

- (void)runUserMain {
    iosMain(0, nullptr);
}

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    UNUSED(application)
    UNUSED(launchOptions)

    [[NSFileManager defaultManager]
        changeCurrentDirectoryPath:[[NSBundle mainBundle] resourcePath]];

    [self performSelector:@selector(runUserMain) withObject:nil afterDelay:0.0];

    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    UNUSED(application)
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    UNUSED(application)
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    UNUSED(application)
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    UNUSED(application)
}

- (void)applicationWillTerminate:(UIApplication *)application {
    UNUSED(application)
}

@end
