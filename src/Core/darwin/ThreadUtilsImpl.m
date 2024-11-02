#import "ThreadUtilsImpl.h"

#import <Foundation/NSString.h>
#import <Foundation/NSThread.h>

////////////////////////////////////////
////////////////////////////////////////
void setNSThreadName(NSThread *id, const char *name) {
    [id setName:[NSString stringWithCString:name encoding:NSASCIIStringEncoding]];
}

////////////////////////////////////////
////////////////////////////////////////
void setCurrentNSThreadName(const char *name) {
    NSThread *id = [NSThread currentThread];
    setNSThreadName(id, name);
}
