#import "View.h"

#import <UIKit/UIScreen.h>

@implementation View

+ (id)layerClass {
    return [CAMetalLayer class];
}

- (id)initWithFrame:(CGRect)frame withScaleFactor:(CGf32)factor {
    self = [super initWithFrame:frame];

    if (self) {
        self.contentScaleFactor     = factor;
        self.userInteractionEnabled = YES;
        self.multipleTouchEnabled   = YES;
    }

    return self;
}

- (BOOL)canBecomeFirstResponder {
    return YES;
}

@end
