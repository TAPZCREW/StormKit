// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

export namespace std {
  using std::clock_t;
  using std::size_t;
  using std::time_t;

  using std::timespec;
  using std::tm;

  using std::asctime;
  using std::clock;

#ifdef _UCRT
#ifndef _In_ 
#define _In_ 
#endif
#ifndef _Out_ 
#define _Out_
#endif
#ifndef _Inout_
#define _Inout_ 
#endif
#ifndef _Out_opt_
#define _Out_opt_ 
#endif

  inline char* __CRTDECL ctime(_In_ time_t const* const _Time) {
    return _ctime64(_Time);
  }

  inline double __CRTDECL difftime(_In_ time_t const _Time1, _In_ time_t const _Time2) {
    return _difftime64(_Time1, _Time2);
  }

  inline struct tm* __CRTDECL gmtime(_In_ time_t const* const _Time) {
    return _gmtime64(_Time);
  }

  inline struct tm* __CRTDECL localtime(_In_ time_t const* const _Time) {
    return _localtime64(_Time);
  }

  inline time_t __CRTDECL mktime(_Inout_ struct tm* const _Tm) {
    return _mktime64(_Tm);
  }
#else
  using std::ctime;
  using std::difftime;
  using std::gmtime;
  using std::localtime;
  using std::mktime;
#endif

  using std::strftime;
#ifdef _UCRT
  inline time_t __CRTDECL time(_Out_opt_ time_t* const _Time) {
    return _time64(_Time);
  }

  inline int __CRTDECL timespec_get(_Out_ struct timespec* const _Ts, _In_ int const _Base) {
    return _timespec64_get((struct _timespec64*)_Ts, _Base);
  }
#else
  using std::time;
  using std::timespec_get;
#endif
} // namespace std