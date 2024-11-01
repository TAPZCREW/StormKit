#import "ThreadUtilsImpl.h"

#import <Foundation/NSString.h>
#import <Foundation/NSThread.h>

////////////////////////////////////////
////////////////////////////////////////
void setThreadName(NSThread *id, const char *name) {
    [id setName:[NSString stringWithCString:name encoding:NSASCIIStringEncoding]];
}

////////////////////////////////////////
////////////////////////////////////////
void setCurrentThreadName(const char *name) {
    NSThread *id = [NSThread currentThread];
    setThreadName(id, name);
}
