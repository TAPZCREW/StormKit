#import "ObjCBridge.h"

#if __cplusplus
extern "C"
#endif
  const void* toCPtr(const CALayer* layer) {
    return (__bridge const void*)(layer);
}
