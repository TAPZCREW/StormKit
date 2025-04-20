#ifndef STORMKIT_CORE_THREAD_UTILS_H
#define STORMKIT_CORE_THREAD_UTILS_H

#ifdef __OBJC__
@class NSThread;
#else
typedef void NSThread;
#endif

void setNSThreadName(NSThread* id, const char* name);
void setCurrentNSThreadName(const char* name);

#endif
