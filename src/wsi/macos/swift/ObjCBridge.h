#ifndef CPP_BINDINGS_HPP
#define CPP_BINDINGS_HPP

#import <QuartzCore/CALayer.h>

#if __cplusplus
extern "C"
#endif
  void* toCPtr(const CALayer*);

#endif
